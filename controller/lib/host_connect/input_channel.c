/*
**input_channel.c
**11-3-2016
**
*/

// ****************************
// file built 2016-3-11
// 建立会议系统连接表管理机制
// 输入通道基础模块
// ****************************

#include "input_channel.h"


int input_channel_list_add_trail( T_pInChannel p_Inputnode, struct list_head *list )
{
	assert( p_Inputnode && list );
	if( p_Inputnode == NULL || list == NULL )
		return -1;

	list_add_tail( &p_Inputnode->list, list );
	
	return 0;
}

T_pInChannel intput_channel_list_node_create( void )
{
	T_pInChannel pInChannelNode = (T_pInChannel)malloc( sizeof(TInChannel) );
	
	return pInChannelNode;
}

bool input_channel_list_node_init( T_pInChannel p_Inputnode, uint64_t channel_id, uint16_t channel_index )
{
	assert( p_Inputnode );
	if( p_Inputnode == NULL )
		return false;

	p_Inputnode->listener_id = channel_id;
	p_Inputnode->listener_index = channel_index;
	p_Inputnode->status = INCHANNEL_FREE;
	p_Inputnode->timetimp = 0;// 表示未连接
	p_Inputnode->tarker_id = 0;
	p_Inputnode->tarker_index = 0xffff;
	p_Inputnode->channel_connected_count = 0;
	p_Inputnode->operate_timp = 0;
	p_Inputnode->pro_status = INCHANNEL_PRO_FINISH;

	return true;
}

// 链表长度
int  get_input_channel_list_double_list_length( struct list_head *list_gurad )
{
	int i = 0;
	struct list_head *list_pos = NULL; 

	assert( list_gurad );
	list_for_each( list_pos, list_gurad )
	{
		i++;
	}
	
	return i;
}

int universe_input_channel_list_add_trail( T_pInChannel_universe p_Inputnode, struct list_head *head )
{
	assert( p_Inputnode && head );
	if( p_Inputnode == NULL || head == NULL )
		return -1;

	list_add_tail( &p_Inputnode->list, head );
	
	return 0;
}

bool universe_input_channel_list_node_init( T_pInChannel_universe p_Inputnode, const uint64_t channel_id, uint16_t channel_index )
{
	assert( p_Inputnode );
	if( p_Inputnode == NULL )
		return false;

	p_Inputnode->tarker_id = 0; // not useful
	p_Inputnode->tarker_index = 0;
	p_Inputnode->listener_index = channel_index;
	p_Inputnode->status = INCHANNEL_FREE;
	p_Inputnode->pro_status = INCHANNEL_PRO_FINISH;
	p_Inputnode->timetimp = 0;// 表示未连接
	p_Inputnode->connect_failed_count = 0;

	return true;
}

int universe_intput_channel_list_node_create( T_pInChannel_universe *pInChannelNode )
{
	if( NULL == pInChannelNode )
		return -1;
	
	*pInChannelNode = (T_pInChannel_universe)malloc( sizeof(TInChannel_universe) );
	if( *pInChannelNode == NULL )
		return -1;
	
	return 0;
}

