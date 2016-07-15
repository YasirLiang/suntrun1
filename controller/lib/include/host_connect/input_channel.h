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
	INCHANNEL_BUSY// 被占用
};

enum input_channel_pro_status
{
	INCHANNEL_PRO_PRIMED = 0,// 预处理
	INCHANNEL_PRO_HANDLING,// 正在处理
	INCHANNEL_PRO_FINISH// 节点已被处理完成
};

enum universe_recieve_model_state// 输入模块通道的状态--已初始化未连接-已初始化连接-未初始化
{
	RECIEVE_MODEL_UNINIT = 0,// 未初始化且不可用，是节点可以设置的状态
	RECIEVE_MODEL_OK,// 已初始化未连接
	RECIEVE_MODEL_ALL_CHANNEL_INIT
};

#define INPUT_STREAME_OPT_PROTECT_TIME (0) // 通道操作保护时间

typedef struct _type_input_channel_universe// 连接输入的通道
{
	uint64_t tarker_id;
	uint16_t tarker_index;
	uint16_t listener_index; 		// 通道ID索引
	timetype operate_timp;// 通道操作时间戳
	timetype timetimp;// 连接时时间戳
	enum input_channel_status status;// 通道的连接状态
	enum input_channel_pro_status pro_status; // 通道节点处理的状态
	uint16_t connect_failed_count;// 连接失败次数
	struct list_head list;			// list用于已连接连接表的链表管理，
}TInChannel_universe,*T_pInChannel_universe;

typedef struct _type_input_channel// 连接输入的通道
{
	uint64_t listener_id; 		// 通道ID
	uint64_t tarker_id;
	uint16_t tarker_index;
	uint16_t listener_index; 		// 通道ID索引
	timetype operate_timp;// 通道操作时间戳
	timetype timetimp;// 连接时时间戳
	uint16_t channel_connected_count; // 通道被连接次数
	enum input_channel_status status;// 通道的连接状态
	enum input_channel_pro_status pro_status; // 通道节点处理的状态
	struct list_head list;			// 用于已连接连接表的链表管理，
}TInChannel,*T_pInChannel;

// **************************************************
int input_channel_list_add_trail( T_pInChannel p_Inputnode, struct list_head *list );
T_pInChannel intput_channel_list_node_create( void );
bool input_channel_list_node_init( T_pInChannel p_Inputnode, uint64_t channel_id, uint16_t channel_index );
int  get_input_channel_list_double_list_length( struct list_head *list_gurad );
int universe_input_channel_list_add_trail( T_pInChannel_universe p_Inputnode, struct list_head *head );
bool universe_input_channel_list_node_init( T_pInChannel_universe p_Inputnode, const uint64_t channel_id, uint16_t channel_index );
int universe_intput_channel_list_node_create( T_pInChannel_universe *pInChannelNode );

// **************************************************//

#endif
