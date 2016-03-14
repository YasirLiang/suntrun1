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

tconference_trans_model gconference_model_guard;// 会议传输单元链表全局头结点

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
        	DEBUG_INFO( "stream_input_desc.descriptor_index = %d out of range:  error",stream_input_desc.descriptor_index);
		return -1;
	}

	// 检查是否不为中央控制接收模块
	memcpy( &entity_name, desc_node->endpoint_desc.entity_name, sizeof(struct jdksavdecc_string));
	if( (strcmp( &entity_name, CCU_TR_MODEL_NAME) == 0) ||\
		(strcmp( &entity_name, CCU_R_MODEL_NAME) == 0) )
	{
		DEBUG_INFO( "entity not a conference uinit %s", &entity_name );
		return -1;
	}

	//********* 
	//搜索是否存在endtity_id 的链表节点
	tconference_trans_pmodel p_temp_node = NULL;
	bool cfc_node_found = false;
	list_for_each_entry( p_temp_node, &gconference_model_guard->list, list )
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

void  trans_model_node_insert_to_list( tconference_trans_pmodel p_new_node)
{
	if( NULL == p_new_node )
		return;
	
	list_add_tail( &gconference_model_guard->list, &p_new_node->list );
}

