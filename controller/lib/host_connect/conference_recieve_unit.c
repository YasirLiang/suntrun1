/*
**conferenc_recieve_unit.c
**注:此文件与muticastor_connector.c的作用一样，用于管理系统的
**		会议单元的连接,与muticastor_connector.c共同作用。
**9-3-2016
**
*/

// ****************************
// file built 2016-3-9
// 建立会议系统连接表管理机制
// 系统会议单元单元接收模块
// ****************************

#include "conference_recieve_unit.h"
#include "jdksavdecc_aem_descriptor.h"
#include "enum.h"
#include "host_controller_debug.h"
#include "linked_list_unit.h"
#include "muticast_connect_manager.h"
#include "acmp_controller_machine.h"

#ifdef __DEBUG__
//#define __CONFERENCE_RECV_UNIT_DEBUG__
#endif

#ifdef __CONFERENCE_RECV_UNIT_DEBUG__
#define conference_recv_unit_debug(fmt, args...) \
	fprintf( stdout,"\033[32m %s-%s-%d:\033[0m "fmt" \r\n", __FILE__, __func__, __LINE__, ##args);
#else
#define conference_recv_unit_debug(fmt, args...)
#endif


LIST_HEAD(gconferenc_recv_model_list);
observer_t gconference_recieve_observer;// 用于更新被广播单元模块的连接状态
static uint16_t gccu_recv_acmp_sequeue_id = 0;

extern solid_pdblist endpoint_list;
extern desc_pdblist descptor_guard;

bool conference_recieve_model_is_right( struct list_head * p_cur_model )
{
	if( p_cur_model == NULL )
		return false;
	
	return (p_cur_model == &gconferenc_recv_model_list)?false:true;
}

bool conference_recieve_model_found_next( struct list_head * p_cur_model, struct list_head **p_next_model )
{
	assert( p_next_model != NULL);
	if( p_next_model == NULL )
		return false;

	if( p_cur_model == NULL)
		*p_next_model = &gconferenc_recv_model_list;
	else
		*p_next_model = p_cur_model->next;	
	
	return true;
}

int conference_recieve_model_node_insert_to_list( T_Ptrconference_recieve_model p_recv_model, struct list_head *head )
{
	assert( p_recv_model && head );
	if( p_recv_model == NULL || head == NULL )
		return -1;

	list_add_tail( &p_recv_model->list, head );
	
	return 0;
}

int conference_recieve_model_node_init( T_Ptrconference_recieve_model p_recv_model, const uint64_t entity_id, const solid_pdblist solid_pnode, const desc_pdblist desc_pnode  )
{
	if( p_recv_model == NULL )
		return -1;

	p_recv_model->listener_id = entity_id;
	p_recv_model->solid_pnode = solid_pnode;
	p_recv_model->desc_pnode = desc_pnode;
	p_recv_model->channel_num = 0;
	p_recv_model->chanel_connect_num = 0;
	p_recv_model->p_ccu_muticast_channel = NULL;
	INIT_LIST_HEAD( &p_recv_model->channel_list );

	return 0;
}

int conference_recieve_model_node_create( T_Ptrconference_recieve_model *pp_recv_model )
{
	if( pp_recv_model == NULL )
		return -1;

	*pp_recv_model = (T_Ptrconference_recieve_model)malloc( sizeof(tconference_recieve_model) );
	if(*pp_recv_model == NULL )
		return -1;

	return 0;
}

int conference_recieve_model_connect_self( uint64_t listern_id )
{// listern_id是需重连的终端
	T_Ptrconference_recieve_model p_cfc_recv_model = NULL;
	struct list_head *p_loop_list = NULL;

	bool discut_self = muticast_muticast_connect_manger_get_discut_self_flag();
	if( !discut_self )
	{// discut self enable
		return 0;
	}
	
	list_for_each( p_loop_list, &gconferenc_recv_model_list )
	{
		T_Ptrconference_recieve_model p_tmp_model = NULL;
		p_tmp_model = list_entry( p_loop_list, tconference_recieve_model, list );
		if( p_tmp_model != NULL )
		{
			if( p_tmp_model->listener_id == listern_id )
			{
				p_cfc_recv_model = p_tmp_model;
				break;
			}
		}
	}

	if( p_cfc_recv_model != NULL )// found ?
	{// search input channel
		if( p_cfc_recv_model->discut_flag )
		{
			p_cfc_recv_model->discut_flag = false;
		}
		
		p_cfc_recv_model->tark_discut = false;// 重新连接
	}

	return 0;
}

int conference_recieve_model_discut_self( uint64_t listern_id )
{//listern_id 对应讲话的终端，
	T_Ptrconference_recieve_model p_cfc_recv_model = NULL;
	struct list_head *p_loop_list = NULL;

	bool discut_self = muticast_muticast_connect_manger_get_discut_self_flag();
	if( !discut_self )
	{// discut self enable
		return 0;
	}
	
	list_for_each( p_loop_list, &gconferenc_recv_model_list )
	{
		T_Ptrconference_recieve_model p_tmp_model = NULL;
		p_tmp_model = list_entry( p_loop_list, tconference_recieve_model, list );
		if( p_tmp_model != NULL )
		{
			if( p_tmp_model->listener_id == listern_id )
			{
				p_cfc_recv_model = p_tmp_model;
				break;
			}
		}
	}

	if( p_cfc_recv_model != NULL )// found ?
	{// search input channel
		T_pInChannel_universe p_input_node = NULL;
		
		if( NULL != p_cfc_recv_model->p_ccu_muticast_channel )
		{
			p_input_node = list_entry( p_cfc_recv_model->p_ccu_muticast_channel, TInChannel_universe, list );
		}
		
		if( p_input_node != NULL )
		{
			struct jdksavdecc_eui64 talker_entity_id;
			struct jdksavdecc_eui64 listener_entity_id;

			convert_uint64_to_eui64( talker_entity_id.value, p_input_node->tarker_id );
			convert_uint64_to_eui64( listener_entity_id.value, listern_id );
			acmp_disconnect_avail( talker_entity_id.value, 
								p_input_node->tarker_index, 
								listener_entity_id.value, 
								p_input_node->listener_index, 
								1, ++gccu_recv_acmp_sequeue_id );
			p_cfc_recv_model->discut_flag = true;
			p_input_node->pro_status = INCHANNEL_PRO_PRIMED;
		}
	}

	return 0;
}

int conference_recieve_model_init( const uint8_t *frame, int pos, size_t frame_len, const desc_pdblist desc_node, const uint64_t endtity_id )
{
 	solid_pdblist solid_node = NULL;
	struct jdksavdecc_string entity_name;// 终端avb名字
	struct jdksavdecc_descriptor_stream stream_input_desc; // Structure containing the stream_output_desc fields
	ssize_t ret = jdksavdecc_descriptor_stream_read( &stream_input_desc, frame, pos, frame_len );
        if (ret < 0)
        {
        	conference_recv_unit_debug( "conference_recieve_model_init: stream_input_desc_read error" );
		return -1;
        }

	if( stream_input_desc.descriptor_index >= CONFERENCE_RECIEVE_UNIT_IN_CHNNEL_MAX_NUM )
	{
		conference_recv_unit_debug( "conference_recieve_model_init : descriptor_index = %d out of range:  error",stream_input_desc.descriptor_index);
		return -1;
	}

	assert( endpoint_list && descptor_guard );
	if( (endpoint_list == NULL) || (descptor_guard == NULL) \
		|| (endtity_id == 0) ||desc_node == NULL )
	{
		return -1;
	}

	// 检查是否是会议接收模块
	memcpy( &entity_name, &desc_node->endpoint_desc.entity_name, sizeof(struct jdksavdecc_string));
	if( strcmp( (char*)entity_name.value, CONFERENCE_RECIEVE_UNIT_NAME) != 0 )
	{
		conference_recv_unit_debug( "not a right conference recieve model = %s:ring name is %s", \
			(char*)entity_name.value, CONFERENCE_RECIEVE_UNIT_NAME );
		return -1;
	}

	solid_node = search_endtity_node_endpoint_dblist( endpoint_list, endtity_id );
	if( solid_node == NULL )
	{
		conference_recv_unit_debug( "central control not found solid endtity( id = 0x%016llx)", endtity_id );
		return -1;
	}

	/**
	 *1、寻找gconferenc_recv_model_list是否存在endtity_id节点
	 *2、若endtity_id存在直接在寻找此endtity_id是否存在input通道信息节点，
	 *		若不存在则创造input通道信息节点
	 *3、若endtity_id没找到则创造，包括input通道信息节点，
	 */
	T_Ptrconference_recieve_model p_cfc_recv_model = NULL;
	T_pInChannel_universe p_input_node = NULL;
	struct list_head *p_loop_list = NULL;
	list_for_each( p_loop_list, &gconferenc_recv_model_list )
	{
		T_Ptrconference_recieve_model p_tmp_model = NULL;
		p_tmp_model = list_entry( p_loop_list, tconference_recieve_model, list );
		if( p_tmp_model != NULL )
		{
			if( p_tmp_model->listener_id == endtity_id )
			{
				p_cfc_recv_model = p_tmp_model;
				break;
			}
		}
	}

	if( p_cfc_recv_model != NULL )// found ?
	{// search input channel
		list_for_each( p_loop_list, &p_cfc_recv_model->channel_list )
		{
			T_pInChannel_universe p_tmp_input = NULL;
			p_tmp_input = list_entry( p_loop_list, TInChannel_universe, list );
			if( p_tmp_input != NULL )
			{
				if( p_tmp_input->listener_index == stream_input_desc.descriptor_index )
				{
					p_input_node = p_tmp_input;
					break;
				}
			}
		}
	}
	else 
	{// not found node,create new!
		conference_recieve_model_node_create( &p_cfc_recv_model );
		if( p_cfc_recv_model != NULL )
		{
			conference_recieve_model_node_init( p_cfc_recv_model, endtity_id, solid_node, desc_node );
			conference_recieve_model_node_insert_to_list( p_cfc_recv_model, &gconferenc_recv_model_list );
		}
	}

	if( NULL != p_cfc_recv_model && (NULL == p_input_node))// new input channel ?
	{// 
		universe_intput_channel_list_node_create( &p_input_node );
		if( p_input_node != NULL )
		{
			universe_input_channel_list_node_init( p_input_node, endtity_id, stream_input_desc.descriptor_index );
			universe_input_channel_list_add_trail( p_input_node, &p_cfc_recv_model->channel_list );

			// 初始化广播通道
			if( p_input_node->listener_index == CONFERENCE_MUTICASTED_INPUT_CHANNEL )
			{
				p_cfc_recv_model->p_ccu_muticast_channel =&p_input_node->list;
				p_cfc_recv_model->query_stop = false;
				p_cfc_recv_model->tark_discut = false;
				host_timer_start( 5*1000, &p_cfc_recv_model->muticast_query_timer );// 初始查询时间为5s
				host_timer_start( 10*1000, &p_cfc_recv_model->errlog_timer );
			}

			p_cfc_recv_model->channel_num++;
		}
	}

	return 0;
}

void conference_recieve_model_unit_update_by_get_rx_state( const uint64_t listern_stream_id,
															const uint16_t tarker_index,
 															const uint64_t listern_id, 
 															const uint16_t listern_id_index,
 															int resp_status)
{// listern_stream_id为tarker id or listen's stream id,一般设成tarker id
 	uint64_t tarker_id = 0;
	struct list_head *p_list = NULL;
	T_Ptrconference_recieve_model p_cfc_recv_model = NULL;
	T_pInChannel_universe p_Inchannel = NULL;

	tarker_id = convert_stream_id_to_tarker_id( listern_stream_id );
	if( tarker_id == listern_stream_id && listern_stream_id != 0 )
	{
		conference_recv_unit_debug( "listern_stream_id( 0x%016llx ) is Stream ID", listern_stream_id );
	}
	else
	{
		conference_recv_unit_debug( "listern_stream_id( 0x%016llx ) is  Stream's Tarker ID(Not zero)", listern_stream_id );
	}

	list_for_each( p_list, &gconferenc_recv_model_list )
	{
		if( p_list != NULL )
		{
			T_Ptrconference_recieve_model p_loop_model = NULL;
			p_loop_model = list_entry( p_list, tconference_recieve_model, list );
			if( p_loop_model != NULL && p_loop_model->listener_id == listern_id )
			{
				p_cfc_recv_model = p_loop_model;
				break;
			}
		}
	}

	if( p_cfc_recv_model != NULL )
	{
		list_for_each( p_list, &p_cfc_recv_model->channel_list )
		{
			if( p_list != NULL )
			{
				T_pInChannel_universe tmp = list_entry( p_list, TInChannel_universe, list );
				if( tmp != NULL && tmp->listener_index == listern_id_index )
				{
					p_Inchannel = tmp;
					break;
				}
			}
		}
	}

	if( p_Inchannel != NULL )
	{
		if(p_Inchannel->pro_status == INCHANNEL_PRO_PRIMED )
		{
			p_Inchannel->pro_status = INCHANNEL_PRO_HANDLING;
		}

		if( p_Inchannel->status == INCHANNEL_FREE )
		{
#if 0			
			// look for tarker index 
			desc_pdblist out_desc = descptor_guard->next;
			int out_stream_index = 0;
			bool found = false;
			for( ; out_desc != descptor_guard; out_desc = out_desc->next )
			{
				// 与一个实体中的所有输出流比较
				uint8_t stream_output_desc_count = out_desc->endpoint_desc.output_stream.num;
				for( out_stream_index = 0; out_stream_index < stream_output_desc_count; out_stream_index++)
				{
					if( (out_desc->endpoint_desc.output_stream.desc[out_stream_index].connect_num > 0)\
						&& (out_desc->endpoint_desc.output_stream.desc[out_stream_index].stream_id \
						== listern_stream_id ) )
					{
						found = true;
#ifdef __DEBUG__
						MSGINFO( "[ 0x%016llx(%d) -> 0x%016llx(%d) Stream ID = 0x%016llx ]",\
							out_desc->endpoint_desc.entity_id,\
							out_desc->endpoint_desc.output_stream.desc[out_stream_index].descriptor_index,\
							listern_id,\
							listern_id_index,\
							listern_stream_id );
#endif
						break;
					}
				}

				if( found )
					break;
			}

			if( listern_stream_id != 0 && (resp_status == 0))
				if( found && p_Inchannel->tarker_index != out_stream_index )// tarker must recv tx state right resonpse first?
					p_Inchannel->tarker_index = out_stream_index;
#else
			
#endif
		}

		if( listern_stream_id != 0 && (resp_status == 0))
		{// rigth response
			if( p_Inchannel->tarker_id != listern_stream_id )
				p_Inchannel->tarker_id = listern_stream_id;
	
			if( p_Inchannel->tarker_index != tarker_index )
				p_Inchannel->tarker_index = tarker_index;

			p_Inchannel->status = INCHANNEL_BUSY;
		}

		p_Inchannel->pro_status = INCHANNEL_PRO_FINISH;
	}
}

void conference_recieve_model_unit_update_by_connect_rx_state(const uint64_t tarker_id,
 															const uint16_t tarker_index,
 															const uint64_t listern_id, 
 															const uint16_t listern_id_index,
 															int resp_status )
 {
	struct list_head *p_list = NULL;
	T_Ptrconference_recieve_model p_cfc_recv_model = NULL;
	T_pInChannel_universe p_Inchannel = NULL;
	
	list_for_each( p_list, &gconferenc_recv_model_list )
	{
		if( p_list != NULL )
		{
			T_Ptrconference_recieve_model p_loop_model = NULL;
			p_loop_model = list_entry( p_list, tconference_recieve_model, list );
			if( p_loop_model != NULL && (p_loop_model->listener_id == listern_id)
				&& (listern_id != 0))
			{
				p_cfc_recv_model = p_loop_model;
				break;
			}
		}
	}

	if( p_cfc_recv_model != NULL )
	{
		list_for_each( p_list, &p_cfc_recv_model->channel_list )
		{
			if( p_list != NULL )
			{
				T_pInChannel_universe tmp = list_entry( p_list, TInChannel_universe, list );
				if( tmp != NULL && tmp->listener_index == listern_id_index )
				{
					p_Inchannel = tmp;
					break;
				}
			}
		}
	}

	if( p_Inchannel != NULL )
	{// found
		if (p_Inchannel->pro_status == INCHANNEL_PRO_PRIMED)
		{
			p_Inchannel->pro_status = INCHANNEL_PRO_HANDLING;
		}

		if (resp_status == 0)
		{//  rigth response, 
			if( (0 != tarker_id) && p_Inchannel->tarker_id != tarker_id )
				p_Inchannel->tarker_id = tarker_id;

			if( p_Inchannel->tarker_index != tarker_index )
				p_Inchannel->tarker_index = tarker_index;

			p_Inchannel->connect_failed_count = 0;// 成功连接后清零
		}
		else
		{// 连接失败次数+1
			p_Inchannel->connect_failed_count++;
		}
		
		p_Inchannel->status = INCHANNEL_BUSY;
		p_Inchannel->pro_status = INCHANNEL_PRO_FINISH;
	}
}

void conference_recieve_model_unit_update_by_disconnect_rx_state( const uint64_t tarker_id,
															const uint16_t tarker_index,
 															const uint64_t listern_id, 
 															const uint16_t listern_id_index,
 															int resp_status )
{
	struct list_head *p_list = NULL;
	T_Ptrconference_recieve_model p_cfc_recv_model = NULL;
	T_pInChannel_universe p_Inchannel = NULL;

	list_for_each( p_list, &gconferenc_recv_model_list )
	{
		if( p_list != NULL )
		{
			T_Ptrconference_recieve_model p_loop_model = NULL;
			p_loop_model = list_entry( p_list, tconference_recieve_model, list );
			if( p_loop_model != NULL && (p_loop_model->listener_id == listern_id)
				&& (listern_id != 0))
			{
				p_cfc_recv_model = p_loop_model;
				break;
			}
		}
	}

	if( p_cfc_recv_model != NULL )// foud ?
	{
		list_for_each( p_list, &p_cfc_recv_model->channel_list )
		{
			if( p_list != NULL )
			{
				T_pInChannel_universe tmp = list_entry( p_list, TInChannel_universe, list );
				if( tmp != NULL && tmp->listener_index == listern_id_index )
				{
					p_Inchannel = tmp;
					break;
				}
			}
		}
	}

	if( p_Inchannel != NULL )// foud ?
	{// found
		if(p_Inchannel->pro_status == INCHANNEL_PRO_PRIMED )
		{
			p_Inchannel->pro_status = INCHANNEL_PRO_HANDLING;
		}

		if( resp_status == 0 )// response right
		{
			p_Inchannel->tarker_id = 0;
			p_Inchannel->tarker_index = 0xffff;// not valid output

			if( p_cfc_recv_model->discut_flag && muticast_muticast_connect_manger_get_discut_self_flag() )
			{// 本机已经断开且数据库也是断开本机
				p_cfc_recv_model->discut_flag = false;
				p_cfc_recv_model->tark_discut = true;// 成功断开本机
			}

			p_Inchannel->status = INCHANNEL_FREE;
		}

		p_Inchannel->pro_status = INCHANNEL_PRO_FINISH;
	}
}

void conference_recieve_model_unit_update( subject_data_elem reflesh_data )// 更新会议接收单元模块的连接状态
{
	uint16_t data_type = reflesh_data.ctrl_msg.data_type;
	uint16_t msg_type = reflesh_data.ctrl_msg.msg_type;
	int repson_status = reflesh_data.ctrl_msg.msg_resp_status;

	if( data_type != JDKSAVDECC_SUBTYPE_ACMP || 
		(msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_TX_STATE_RESPONSE) )
	{// not acmp msg or recieve model do nothing with tx state response,in the meantime, transmit model do nothing with rx state response.
	 // Because stream Id is tarker id , transmit model also do nothing with TX state response.
		return;
	}

	switch( msg_type )
	{
		case JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_RX_STATE_RESPONSE:
		case JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_RX_STATE_COMMAND:
			conference_recieve_model_unit_update_by_get_rx_state( reflesh_data.tarker_id,
									reflesh_data.tarker_index,
									reflesh_data.listener_id,
									reflesh_data.listener_index,
									repson_status );
			break;
		case JDKSAVDECC_ACMP_MESSAGE_TYPE_CONNECT_RX_RESPONSE:
		case JDKSAVDECC_ACMP_MESSAGE_TYPE_CONNECT_RX_COMMAND:
			conference_recieve_model_unit_update_by_connect_rx_state( reflesh_data.tarker_id,
									reflesh_data.tarker_index,
									reflesh_data.listener_id,
									reflesh_data.listener_index,
									repson_status );
			break;
		case JDKSAVDECC_ACMP_MESSAGE_TYPE_DISCONNECT_RX_RESPONSE:
		case JDKSAVDECC_ACMP_MESSAGE_TYPE_DISCONNECT_RX_COMMAND:
			conference_recieve_model_unit_update_by_disconnect_rx_state( reflesh_data.tarker_id,
									reflesh_data.tarker_index,
									reflesh_data.listener_id,
									reflesh_data.listener_index,
									repson_status );
			break;
		default:
			conference_recv_unit_debug( "update cfc recv model msg type(0x%02x) info err!", msg_type );
			break;
	}
}

void conference_recieve_uinit_proccess_init( void )
{
	// 初始化观察者
	init_observer( &gconference_recieve_observer, conference_recieve_model_unit_update );
	// 加入观察者到被观察者
	attach_observer( &gconnector_subjector, &gconference_recieve_observer );
}

