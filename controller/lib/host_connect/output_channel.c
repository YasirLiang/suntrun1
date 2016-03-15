/*
**output_channel.c
**11-3-2016
**
*/

// ****************************
// file built 2016-3-11
// 建立会议系统连接表管理机制
// 输入通道基础模块
// ****************************

#include "output_channel.h"

bool output_channel_insert_node_to_list( struct list_head *head, T_pOutChannel p_node )
{
	assert( head && p_node );
	if( head == NULL || p_node == NULL )
		return false;

	list_add_tail( &p_node->list, head );
	
	return true;
}

// 根据index初始化输出通道节点
bool output_channel_node_init_by_index( T_pOutChannel p_node, uint16_t tarker_index  )
{
	assert( p_node );
	if( p_node == NULL )
		return false;
	
	memset( p_node, 0, sizeof(TOutChannel));
	p_node->tarker_index = tarker_index;

	return true;
}

T_pOutChannel out_channel_node_create_can_init( void )
{
	T_pOutChannel p_node = (T_pOutChannel)malloc( sizeof(TOutChannel));
	assert( p_node );

	return p_node;
}

bool input_connect_node_insert_node_to_list( struct list_head *head, Input_pChannel p_node )
{
	assert( head && p_node );
	if( head == NULL || p_node == NULL )
		return false;

	list_add_tail( &p_node->list, head );
	
	return true;
}


bool input_connect_node_init_by_index( Input_pChannel p_node, uint64_t  listen_id, uint16_t listen_index  )
{
	assert( p_node );
	if( p_node == NULL )
		return false;
	
	memset( p_node, 0, sizeof(Input_Channel));
	p_node->listener_id = listen_id;
	p_node->listen_index = listen_index;

	return true;
}

Input_pChannel input_connect_node_create( void )
{
	Input_pChannel p_node = ( Input_pChannel )malloc( sizeof(Input_Channel));
	assert( p_node );

	return p_node;
}
