/*
**input_channel.h
**11-3-2016
**
*/

// ****************************
// file built 2016-3-11
// 建立会议系统连接表管理机制
// 输入通道基础模块
// ****************************

#ifndef __INPUT_CHANNEL_H__
#define __INPUT_CHANNEL_H__

#include "jdksavdecc_world.h"
#include "list.h"
#include "host_time.h"

enum input_channel_status// 输入通道的状态
{
	INCHANNEL_UNAVAILABLE = 0, // 不可用
	INCHANNEL_FREE,// 空闲
	INCHANNEL_BUSY,// 被占用
};

enum input_channel_pro_status
{
	INCHANNEL_PRO_PRIMED = 0,// 预处理
	INCHANNEL_PRO_HANDLING,// 正在处理
	INCHANNEL_PRO_FINISH// 节点已被处理完成
};

typedef struct _type_input_channel// 连接输入的通道
{
	uint64_t listener_id; 		// 通道ID
	uint64_t tarker_id;
	uint16_t tarker_index;
	uint16_t listener_index; 		// 通道ID索引
	timetype timetimp;// 连接时时间戳
	enum input_channel_status status;// 通道的连接状态
	enum input_channel_pro_status pro_status; // 通道节点处理的状态
	struct list_head list;			// 用于已连接连接表的链表管理，
}TInChannel,*T_pInChannel;

// **************************************************
int input_channel_list_add_trail( T_pInChannel p_Inputnode, struct list_head *list );
T_pInChannel intput_channel_list_node_create( void );
bool input_channel_list_node_init( T_pInChannel p_Inputnode, uint64_t channel_id, uint16_t channel_index );
int  get_input_channel_list_double_list_length( struct list_head *list_gurad );

// **************************************************//

#endif
