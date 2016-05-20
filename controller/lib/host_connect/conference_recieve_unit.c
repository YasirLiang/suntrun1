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

#if 0
uint64_t gacmp_tx_tarker_stream_id = 0;
uint64_t gacmp_rx_listerner_stream_id = 0;
conventioner_cnnt_list_node* conventioner_cnnt_node = NULL;
#endif

LIST_HEAD(gconferenc_recv_model_list);

observer_t gconference_recieve_observer;// 用于更新被广播单元模块的连接状态

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
	struct list_head *loop_tmp = NULL;

	assert( p_cur_model != NULL || (p_next_model != NULL));
	if( p_cur_model == NULL || (p_next_model == NULL) )
		return false;

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

 int conference_recieve_model_init( const uint8_t *frame, int pos, size_t frame_len, const desc_pdblist desc_node, const uint64_t endtity_id )
 {
 	solid_pdblist solid_node = NULL;
	struct jdksavdecc_string entity_name;// 终端avb名字
	struct jdksavdecc_descriptor_stream stream_input_desc; // Structure containing the stream_output_desc fields
	ssize_t ret = jdksavdecc_descriptor_stream_read( &stream_input_desc, frame, pos, frame_len );
        if (ret < 0)
        {
        	DEBUG_INFO( "conference_recieve_model_init: stream_input_desc_read error" );
		return -1;
        }

	if( stream_input_desc.descriptor_index >= CONFERENCE_RECIEVE_UNIT_IN_CHNNEL_MAX_NUM )
	{
        	DEBUG_INFO( "conference_recieve_model_init : descriptor_index = %d out of range:  error",stream_input_desc.descriptor_index);
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
		DEBUG_INFO( "not a right conference recieve model = %s:ring name is %s", \
			(char*)entity_name.value, CONFERENCE_RECIEVE_UNIT_NAME );
		return -1;
	}

	solid_node = search_endtity_node_endpoint_dblist( endpoint_list, endtity_id );
	if( solid_node == NULL )
	{
		DEBUG_INFO( "central control not found solid endtity( id = 0x%016llx)", endtity_id );
		return -1;
	}

	/**
	 *1、寻找gconferenc_recv_model_list是否存在endtity_id节点
	 *2、若endtity_id存在直接在寻找此endtity_id是否存在input通道信息节点，
	 *		若不存在则创造input通道信息节点
	 *3、若endtity_id没找到则创造，包括input通道信息节点，
	 */
	T_Ptrconference_recieve_model p_cfc_recv_model = NULL;
	T_pInChannel_universe *p_input_node = NULL;
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
			universe_input_channel_list_add_trail( p_input_node, p_cfc_recv_model->channel_list );
		}
	}

	return 0;
 }

void conference_recieve_model_unit_update( subject_data_elem reflesh_data )// 更新会议接收单元模块的连接状态
{
	uint16_t data_type = reflesh_data.ctrl_msg.data_type;
	uint16_t msg_type = reflesh_data.ctrl_msg.msg_type;
	int repson_status = reflesh_data.ctrl_msg.msg_resp_status;

#if 0	
	if( data_type != JDKSAVDECC_SUBTYPE_ACMP || 	(gmuti_connect_pro.eelem_flags != CVNT_CHECK_WAIT) )
	{// not acmp msg or not proccess muticator connect mamager
		return;
	}

	if( msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_RX_STATE_RESPONSE )
	{
		if( repson_status == 0 )
			gacmp_rx_listerner_stream_id = reflesh_data.listener_id;
	}
	else if( msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_TX_STATE_RESPONSE )
	{
		gacmp_tx_tarker_stream_id = reflesh_data.tarker_id;
		muticast_connector_proccess_online_callback( gacmp_tx_tarker_stream_id, 
												gacmp_rx_listerner_stream_id, 
												conventioner_cnnt_node, 
												(repson_status==ACMP_STATUS_SUCCESS)?true:false );// update rx tx success
		gacmp_tx_tarker_stream_id = 0;
		gacmp_rx_listerner_stream_id = 0;
		conventioner_cnnt_node = NULL;
	}
	else if( msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_CONNECT_RX_RESPONSE )
	{
		if( reflesh_data.ctrl_msg.msg_resp_status == JDKSAVDECC_ACMP_STATUS_LISTENER_EXCLUSIVE )
		{// search for wrong cfc_recieve uinit muticastor and disconnect wrong link
			uint64_t listerner_id = reflesh_data.listener_id;
			uint16_t listerner_index = reflesh_data.listener_index;
			
		}
		
		muticast_connector_connect_callback( reflesh_data.tarker_id, 
										conventioner_cnnt_node, 
										(repson_status==ACMP_STATUS_SUCCESS)?true:false );

		conventioner_cnnt_node = NULL;
	}
	else if( msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_DISCONNECT_RX_RESPONSE )
	{
		
	}
#endif
}

void conference_recieve_uinit_init( void )
{
	// 初始化观察者
	init_observer( &gconference_recieve_observer, conference_recieve_model_unit_update );
	// 加入观察者到被观察者
	attach_observer( &gconnector_subjector, &gconference_recieve_observer );
}

