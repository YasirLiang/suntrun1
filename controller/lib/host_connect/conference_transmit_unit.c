/*
**conference_transmit_unit.c
**9-3-2016
**
*/

// ********************************
// file built 2016-3-9
// 建立会议系统连接表管理机制
// 会议单元发送模块
// ********************************

#include "conference_transmit_unit.h"
#include "jdksavdecc_aem_descriptor.h"
#include "central_control_recieve_unit.h"
#include "util.h"
#include "terminal_pro.h"

tconference_trans_model gconference_model_guard;// 会议传输单元链表全局头结点

observer_t gconference_trans_observer;// 用于更新会议单元模块的连接状态

void  trans_model_node_insert_to_list( tconference_trans_pmodel p_new_node)
{
	assert( NULL != p_new_node );
	if( NULL == p_new_node )
		return;
	
	list_add_tail(  &p_new_node->list, &gconference_model_guard.list );
}

// 初始化会议单元输出通道信息
int conference_transmit_unit_init( const uint8_t *frame, int pos, size_t frame_len, const uint64_t endtity_id, const desc_pdblist desc_node )
{
	struct jdksavdecc_string entity_name;// 终端avb名字
	struct jdksavdecc_descriptor_stream stream_output_desc; // Structure containing the stream_output_desc fields
	ssize_t ret = jdksavdecc_descriptor_stream_read( &stream_output_desc, frame, pos, frame_len );
        if (ret < 0)
        {
        	DEBUG_INFO( "avdecc_read_descriptor_error: stream_input_desc_read error" );
		return -1;
        }

	if( stream_output_desc.descriptor_index > CONFERENCE_OUTCHANNEL_MAX_NUM )
	{
        	DEBUG_INFO( "stream_input_desc.descriptor_index = %d out of range:  error",stream_output_desc.descriptor_index);
		return -1;
	}

	// 检查是否不为中央控制接收模块
	memcpy( &entity_name, &desc_node->endpoint_desc.entity_name, sizeof(struct jdksavdecc_string));
	if( (strcmp((char*) &entity_name, CCU_TR_MODEL_NAME) == 0) ||\
		(strcmp( (char*)&entity_name, CCU_R_MODEL_NAME) == 0) )
	{
		DEBUG_INFO( "entity not a conference uinit %s", (char*)&entity_name.value);
		return -1;
	}

	//********* 
	//搜索是否存在endtity_id 的链表节点
	tconference_trans_pmodel p_temp_node = NULL;
	bool cfc_node_found = false;
	list_for_each_entry( p_temp_node, &gconference_model_guard.list, list )
	{
		if( p_temp_node->tarker_id == endtity_id )
		{
			cfc_node_found = true;
			break;
		}
	}

	if( cfc_node_found )
	{
		T_pOutChannel p_node = out_channel_node_create_can_init();
		if( p_node != NULL )
		{
			output_channel_node_init_by_index( p_node, stream_output_desc.descriptor_index );
			output_channel_insert_node_to_list( &p_temp_node->out_ch.list, p_node );
		}
	}
	else
	{
		p_temp_node = (tconference_trans_pmodel)malloc( sizeof(tconference_trans_model) );
		if( p_temp_node != NULL )
		{
			INIT_ZERO(p_temp_node, sizeof(tconference_trans_model));
			INIT_LIST_HEAD( &p_temp_node->out_ch.list );
			INIT_LIST_HEAD( &p_temp_node->out_ch.input_head.list );
			p_temp_node->tarker_id = endtity_id;

			T_pOutChannel p_node = out_channel_node_create_can_init();
			if( p_node != NULL )
			{
				output_channel_node_init_by_index( p_node, stream_output_desc.descriptor_index );
				output_channel_insert_node_to_list( &p_temp_node->out_ch.list, p_node );
			}
			
			trans_model_node_insert_to_list( p_temp_node );
		}
	}

	return 0;
}

// 初始化会议传输单元的会议单元节点
int conference_transmit_unit_init_conference_node( const tmnl_pdblist p_tmnl_node, const uint64_t tarker_id )
{
	tconference_trans_pmodel p_temp_node = NULL;
	bool found = false;
	
	assert( NULL != p_tmnl_node );
	if( NULL == p_tmnl_node )
	{
		return -1;
	}

	list_for_each_entry( p_temp_node, &gconference_model_guard.list, list )
	{
		if( p_temp_node->tarker_id == tarker_id )
		{
			if( p_temp_node->confenrence_node == NULL )
				p_temp_node->confenrence_node = p_tmnl_node;
			
			found = true;
			break;
		}
	}

	return (found?0:-1);
}

int trans_model_unit_connect( uint64_t tarker_id, const tmnl_pdblist p_tmnl_node )// return -1; means that there is no ccu reciever model 
{
	if( 0 == conference_transmit_unit_init_conference_node( p_tmnl_node, tarker_id ) )
		return ccu_recv_model_talk( tarker_id, CONFERENCE_OUTPUT_INDEX );

	return -1;
}

int trans_model_unit_disconnect( uint64_t tarker_id, const tmnl_pdblist p_tmnl_node ) // return -1 means talker not connect
{
	if( 0 == conference_transmit_unit_init_conference_node( p_tmnl_node, tarker_id ) )
		return ccu_recv_model_untalk( tarker_id, CONFERENCE_OUTPUT_INDEX );

	return -1;
}

void trans_model_unit_update( subject_data_elem connect_info )// 更新传输模块的连接状态, 并发送通知会议系统协议的消息
{
	const bool cnnt_flag = connect_info.connect_flag;
	tconference_trans_pmodel p_temp_node = NULL;

	if( !cnnt_flag )// connferenc Mic close
	{
		list_for_each_entry( p_temp_node, &gconference_model_guard.list, list )
		{
			if( p_temp_node->tarker_id == connect_info.tarker_id )
			{
				T_pOutChannel p_Outnode = NULL;
				
				if( NULL != p_temp_node->confenrence_node)
					terminal_mic_status_set_callback( false, p_temp_node->confenrence_node );
				
				list_for_each_entry(p_Outnode, &p_temp_node->out_ch.list, list)
				{
					if( p_Outnode->tarker_index == connect_info.tarker_index )
					{
						Input_pChannel Input_pnode = NULL;
						list_for_each_entry( Input_pnode, &p_Outnode->list, list )
						{
							if(  Input_pnode->listener_id == connect_info.listener_id&& \
								(Input_pnode->listen_index== connect_info.listener_index))// found?
							{
								__list_del_entry(&Input_pnode->list);// delect connect input node
								if( Input_pnode != NULL )
								{
									free(Input_pnode);
									Input_pnode = NULL;	
								}

								DEBUG_INFO( "conference unit tranmist  model update.......Success!(tarker :index)(0x%016llx:%d)--(listen :index)(0x%016llx:%d)",\
										connect_info.tarker_id, connect_info.tarker_index, connect_info.listener_id, connect_info.listener_index );
								return;
							}
						}
					}
				}
			}
		}
	}
	else// connferenc Mic open
	{
		list_for_each_entry( p_temp_node, &gconference_model_guard.list, list )
		{
			if( p_temp_node->tarker_id == connect_info.tarker_id )
			{
				T_pOutChannel p_Outnode = NULL;				
				list_for_each_entry(p_Outnode, &p_temp_node->out_ch.list, list)
				{
					if( p_Outnode->tarker_index == connect_info.tarker_index )
					{
						Input_pChannel Input_pnode = NULL;
						Input_pnode = input_connect_node_create();
						if( Input_pnode == NULL )
						{
							DEBUG_INFO( "connect info not save Success!!!" );
							return;
						}

						input_connect_node_init_by_index( Input_pnode, connect_info.listener_id, connect_info.listener_index );
						input_connect_node_insert_node_to_list( &p_Outnode->list, Input_pnode );

						if( NULL != p_temp_node->confenrence_node)
							terminal_mic_status_set_callback( true, p_temp_node->confenrence_node );
	
						DEBUG_INFO( "conference unit tranmist  model update.......Success!(tarker :index)(0x%016llx:%d)-- (listen :index)(0x%016llx:%d)",\
							connect_info.tarker_id, connect_info.tarker_index, connect_info.listener_id, connect_info.listener_index );
						return;
					}
				}

				break;// if found and updata failed, return, not search again;
			}
		}
	}
	
}

void conference_transmit_model_init( void )
{
	INIT_LIST_HEAD( &gconference_model_guard.list );

	// 初始化观察者
	init_observer( &gconference_trans_observer, trans_model_unit_update );
	// 加入观察者到被观察者
	attach_observer( &gconnector_subjector, &gconference_trans_observer );
}

