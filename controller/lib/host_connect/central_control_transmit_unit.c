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
		DEBUG_INFO( "Malloc Failed!" );
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
			DEBUG_INFO( "New Transmit  CCU (0x%016llx) Out Channel ( Index = %d)",
						endtity_id, stream_out_desc.descriptor_index );

			if( p_outch->tarker_index == CCU_CONTROL_TRANSMIT_UINT_OUTPUT )
			{
				muticast_connect_manger_chdefault_outmuticastor( &ptr_model->list, &p_outch->list );
			}
		}
	}
	
	return 0;
}

void central_control_transmit_unit_update( subject_data_elem reflesh_data )// 更新会议接收单元模块的连接状态
{
	uint16_t data_type = reflesh_data.ctrl_msg.data_type;
	uint16_t msg_type = reflesh_data.ctrl_msg.msg_type;
	int repson_status = reflesh_data.ctrl_msg.msg_resp_status;
}

void conference_recieve_uinit_init( void )
{
	// 初始化观察者
	init_observer( &gccu_transmit_observer, central_control_transmit_unit_update );
	// 加入观察者到被观察者
	attach_observer( &gconnector_subjector, &gccu_transmit_observer );
}

