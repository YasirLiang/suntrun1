/*
**central_control_transmit_unit.c
**9-3-2016
**
*/

// ****************************
// file built 2016-3-9
// 建立会议系统连接表管理机制
// 中心控制单元发送模块
// ****************************

#include "central_control_transmit_unit.h"
#include "connector_subject.h"
#include "log_machine.h"
#include "jdksavdecc_aem_descriptor.h"
#include "enum.h"
#include "host_controller_debug.h"
#include "muticast_connect_manager.h"
#include "time_handle.h"

#ifdef __DEBUG__
#define __CCU_TRANSMIT_UNIT_DEBUG__
#endif

#ifdef __CCU_TRANSMIT_UNIT_DEBUG__
#define ccu_transmit_unit_debug(fmt, args...) \
	fprintf( stdout,"\033[32m %s-%s-%d:\033[0m "fmt" \r\n", __FILE__, __func__, __LINE__, ##args);
#else
#define ccu_transmit_unit_debug(fmt, args...)
#endif

static observer_t gccu_transmit_observer;// 中央发送单元观察者
LIST_HEAD(gccu_trans_model_guard);// 创建并初始化中央传输单元头节点

int central_control_transmist_insert_model_list_trail( T_pccuTModel ptr_model, struct list_head *head )
{
	if( ptr_model == NULL ||(head == NULL) )
		return -1;
	
	list_add_tail( &ptr_model->list, head );

	return 0;
}

int central_control_transmist_model_init( T_pccuTModel ptr_model, uint64_t tarker_id )
{
	if( ptr_model == NULL )
		return -1;
	
	ptr_model->tarker_id = tarker_id;
	INIT_LIST_HEAD( &ptr_model->out_ch.list );// 初始化output 链表

	return 0;
}

int central_control_transmist_model_create( T_pccuTModel *ptr_model )
{
	if( ptr_model == NULL )
		return -1;

	*ptr_model = (T_pccuTModel)malloc( sizeof(TccuTModel) );
	if( *ptr_model == NULL )
	{
		ccu_transmit_unit_debug( "Malloc Failed!" );
		return -1;
	}

	return 0;
}

int central_control_transmit_unit_init( const uint8_t *frame, int pos, size_t frame_len, const desc_pdblist desc_node, const uint64_t endtity_id )
{
	struct jdksavdecc_string entity_name;// 终端avb名字
	struct jdksavdecc_descriptor_stream stream_out_desc; // Structure containing the stream_output_desc fields
	ssize_t ret = jdksavdecc_descriptor_stream_read( &stream_out_desc, frame, pos, frame_len );
        if (ret < 0)
        {
        	if( gp_log_imp != NULL )
        		gp_log_imp->log.post_log_msg( &gp_log_imp->log, 
									LOGGING_LEVEL_NOTICE, 
									"central_control_transmit_unit_init avdecc_read_descriptor_error: stream_out_desc_read error" );
		return -1;
        }

	if( stream_out_desc.descriptor_index >= CCU_CONTROL_TRANSMIT_UINT_MAX_OUTPUT_NUM )
	{
		if( gp_log_imp != NULL )
			gp_log_imp->log.post_log_msg( &gp_log_imp->log, 
									LOGGING_LEVEL_NOTICE, 
									"central_control_transmit_unit_init stream_out_desc.descriptor_index = %d out of range:  error" );
		return -1;
	}

	assert( desc_node != NULL );
	if( (endtity_id == 0) ||desc_node == NULL )
	{
		return -1;
	}

	// 检查是否是中央控制接收模块
	memcpy( &entity_name, &desc_node->endpoint_desc.entity_name, sizeof(struct jdksavdecc_string));
	if( strcmp( (char*)entity_name.value, CCU_CONTROL_TRANSMIT_UNIT_NAME) != 0 )
	{
		return -1;
	}

	/*
	  *1:查询是否存在endtity_id的output 输出通道
	  *若没有创建新节点
	  *
	  */
	T_pccuTModel ptr_model = NULL;
	struct list_head *p_list;
	bool found_model = false, found_output = false;
	list_for_each( p_list, &gccu_trans_model_guard )
	{
		T_pccuTModel ptr_tmp_model = NULL;
		ptr_tmp_model = list_entry( p_list, TccuTModel, list );
		if( ptr_tmp_model != NULL )
		{
			if( ptr_tmp_model->tarker_id == endtity_id )
			{// search model
				ptr_model = ptr_tmp_model;
				found_model = true;
				break;
			}
		}
	}

	T_pOutChannel p_outch = NULL;
	if( found_model )
	{// 找output
		list_for_each( p_list, &ptr_model->out_ch.list )
		{
			T_pOutChannel p_tmp_outch = NULL;
			p_tmp_outch = list_entry( p_list, TOutChannel, list );
			if( (p_tmp_outch != NULL) && (p_tmp_outch->tarker_index == stream_out_desc.descriptor_index) )
			{// found output
				found_output = true;
				p_outch = p_tmp_outch;
				break;
			}
		}
	}
	else
	{// create ccu trans model and output
		central_control_transmist_model_create( &ptr_model );
		if( ptr_model != NULL )
		{
			central_control_transmist_model_init( ptr_model, endtity_id );
			central_control_transmist_insert_model_list_trail( ptr_model, &gccu_trans_model_guard );
		}
	}

	if( ptr_model != NULL && !found_output )
	{
		// 创建output channel node
		p_outch = out_channel_node_create_can_init();
		if( p_outch != NULL )
		{
			output_channel_node_init_by_index( p_outch, stream_out_desc.descriptor_index );
			output_channel_insert_node_to_list( &ptr_model->out_ch.list, p_outch );
			ccu_transmit_unit_debug( "New Transmit  CCU (0x%016llx) Out Channel ( Index = %d)",
						endtity_id, stream_out_desc.descriptor_index );
			if( p_outch->tarker_index == CCU_CONTROL_TRANSMIT_UINT_OUTPUT )
			{
				muticast_connect_manger_chdefault_outmuticastor( &ptr_model->list, &p_outch->list );
			}
		}
	}
	
	return 0;
}

void central_control_transmit_unit_update_by_connect_rx_state( const uint64_t tarker_id,
															const uint16_t tarker_index,
 															const uint64_t listern_id, 
 															const uint16_t listern_id_index,
 															int resp_status )
{
	T_pccuTModel ptr_model = NULL;
	T_pOutChannel p_outch = NULL;
	struct list_head *p_list = NULL;
	bool found_model = false, found_output = false;

	if( (resp_status != 0) ||(tarker_id == 0) ||(listern_id == 0))// response not right?
		return;

	list_for_each( p_list, &gccu_trans_model_guard )
	{
		T_pccuTModel ptr_tmp_model = NULL;
		ptr_tmp_model = list_entry( p_list, TccuTModel, list );
		if( ptr_tmp_model != NULL )
		{
			if( ptr_tmp_model->tarker_id == tarker_id )
			{// search model
				ptr_model = ptr_tmp_model;
				found_model = true;
				break;
			}
		}
	}

	if( found_model )
	{// 找output
		p_list = NULL;
		list_for_each( p_list, &ptr_model->out_ch.list )
		{
			T_pOutChannel p_tmp_outch = NULL;
			p_tmp_outch = list_entry( p_list, TOutChannel, list );
			if( (p_tmp_outch != NULL) )
				if(p_tmp_outch->tarker_index == tarker_index)
				{// found output
					found_output = true;
					p_outch = p_tmp_outch;
					p_outch->operate_timetimp = get_current_time();
					break;
				}
		}
	}

	if( found_output  )// found ?
	{
		Input_pChannel p_intput = input_connect_node_create();
		if( p_intput != NULL )
		{// create success
			input_connect_node_init_by_index( p_intput, listern_id, listern_id_index );
			input_connect_node_insert_node_to_list( &p_outch->input_head.list, p_intput );
		}
	}
}

void central_control_transmit_unit_update_by_disconnect_rx_state( const uint64_t tarker_id,
															const uint16_t tarker_index,
 															const uint64_t listern_id, 
 															const uint16_t listern_id_index,
 															int resp_status )
{
	T_pccuTModel ptr_model = NULL;
	T_pOutChannel p_outch = NULL;
	struct list_head *p_list = NULL;
	bool found_model = false, found_output = false;

	if( (resp_status != 0) ||(tarker_id == 0) ||(listern_id == 0))// response not right?
		return;

	// 1、search output model
	// 2\search ouput model
	// 3\search output model's connect input channel
	list_for_each( p_list, &gccu_trans_model_guard )
	{
		T_pccuTModel ptr_tmp_model = NULL;
		ptr_tmp_model = list_entry( p_list, TccuTModel, list );
		if( ptr_tmp_model != NULL )
		{
			if( ptr_tmp_model->tarker_id == tarker_id )
			{// search model
				ptr_model = ptr_tmp_model;
				found_model = true;
				break;
			}
		}
	}

	if( found_model )
	{// 找output
		list_for_each( p_list, &ptr_model->out_ch.list )
		{
			T_pOutChannel p_tmp_outch = NULL;
			p_tmp_outch = list_entry( p_list, TOutChannel, list );
			if( (p_tmp_outch != NULL) && (p_tmp_outch->tarker_index == tarker_index) )
			{// found output
				found_output = true;
				p_outch = p_tmp_outch;
				break;
			}
		}
	}

	if (found_output )// found ?
	{
		p_list = NULL;
		list_for_each(p_list, &p_outch->input_head.list)// 原来是list_for_each( p_list, &p_outch->list )，这是错的
		{
			Input_pChannel p_tmp_inputch = NULL;
			p_tmp_inputch = list_entry( p_list, Input_Channel, list );
			if ((p_tmp_inputch != NULL) && (p_tmp_inputch->listener_id == listern_id)
				&& (p_tmp_inputch->listen_index == listern_id_index))// found input ?
			{// found output
				input_connect_node_delect_node_from_list( &p_outch->input_head.list, &p_tmp_inputch );
				break;
			}
		}

		p_outch->operate_timetimp = get_current_time();
	}
}


void central_control_transmit_unit_update( subject_data_elem reflesh_data )// 更新会议接收单元模块的连接状态
{
	uint16_t data_type = reflesh_data.ctrl_msg.data_type;
	uint16_t msg_type = reflesh_data.ctrl_msg.msg_type;
	int repson_status = reflesh_data.ctrl_msg.msg_resp_status;
	
	if( data_type != JDKSAVDECC_SUBTYPE_ACMP || 
		(msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_TX_STATE_RESPONSE) )
	{// not acmp msg or tramsit model do nothing with tx state response,in the meantime, transmit model do nothing with rx state response.
	 // Because stream Id is tarker id , transmit model also do nothing with TX state response.
		return;
	}

	switch( msg_type )
	{
		case JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_RX_STATE_RESPONSE:
		case JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_RX_STATE_COMMAND:
			// do nothing
			break;
		case JDKSAVDECC_ACMP_MESSAGE_TYPE_CONNECT_RX_RESPONSE:
		case JDKSAVDECC_ACMP_MESSAGE_TYPE_CONNECT_RX_COMMAND:
			central_control_transmit_unit_update_by_connect_rx_state( reflesh_data.tarker_id,
									reflesh_data.tarker_index,
									reflesh_data.listener_id,
									reflesh_data.listener_index,
									repson_status );
			break;
		case JDKSAVDECC_ACMP_MESSAGE_TYPE_DISCONNECT_RX_RESPONSE:
		case JDKSAVDECC_ACMP_MESSAGE_TYPE_DISCONNECT_RX_COMMAND:
			central_control_transmit_unit_update_by_disconnect_rx_state( reflesh_data.tarker_id,
									reflesh_data.tarker_index,
									reflesh_data.listener_id,
									reflesh_data.listener_index,
									repson_status );
			break;
		default:
			ccu_transmit_unit_debug( "update cfc recv model msg type(0x%02x) info err!", msg_type );
			break;
	}
}

void central_control_transmit_unit_model_pro_init( void )
{
	// 初始化观察者
	init_observer( &gccu_transmit_observer, central_control_transmit_unit_update );
	// 加入观察者到被观察者
	attach_observer( &gconnector_subjector, &gccu_transmit_observer );
}

void central_control_transmit_unit_model_destroy(void)
{
	T_pccuTModel pos = NULL, n = NULL;
	T_pOutChannel pos2 = NULL, n2 = NULL;
	Input_pChannel pos3 = NULL, n3 = NULL;
	
	list_for_each_entry_safe(pos, n, &gccu_trans_model_guard, list)
	{
		list_for_each_entry_safe(pos2, n2, &pos->out_ch.list, list)
		{
			list_for_each_entry_safe(pos3, n3, &pos2->input_head.list, list)
			{// release input connect
				__list_del_entry(&pos3->list);
				free(pos3);
			}

			__list_del_entry(&pos2->list);
			free(pos2);// release ouput channel
		}

		__list_del_entry(&pos->list);// delect node for ouput
		free(pos);
	}
}

bool central_control_transmit_unit_can_output_found( uint64_t tarker_id, uint16_t tarker_index, struct list_head** pp_model, struct list_head** pp_model_output )
{
	bool bret = true;
	
	if( pp_model == NULL || pp_model_output == NULL )
		return false;

	if( bret )
	{
		T_pccuTModel ptr_model = NULL;
		T_pOutChannel p_outch = NULL;
		struct list_head *p_list = NULL;
		bool found_model = false, found_output = false;

		list_for_each( p_list, &gccu_trans_model_guard )
		{
			T_pccuTModel ptr_tmp_model = NULL;
			ptr_tmp_model = list_entry( p_list, TccuTModel, list );
			if( ptr_tmp_model != NULL )
			{
				if( ptr_tmp_model->tarker_id == tarker_id )
				{// search model
					ptr_model = ptr_tmp_model;
					found_model = true;
					break;
				}
			}
		}

		if( found_model )
		{// 找output
			list_for_each( p_list, &ptr_model->out_ch.list )
			{
				T_pOutChannel p_tmp_outch = NULL;
				p_tmp_outch = list_entry( p_list, TOutChannel, list );
				if( (p_tmp_outch != NULL) && (p_tmp_outch->tarker_index == tarker_index) )
				{// found output
					found_output = true;
					p_outch = p_tmp_outch;
					break;
				}
			}
		}

		if( found_output && ptr_model != NULL && p_outch != NULL )
		{
			*pp_model = &ptr_model->list;
			*pp_model_output = &p_outch->list;
		}
		else
		{
			bret = false;
		}
	}

	return bret;
}

