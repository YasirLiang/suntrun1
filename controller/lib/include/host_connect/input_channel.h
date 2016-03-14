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

enum input_channel_status// 输入通道的状态
{
	INCHANNEL_UNAVAILABLE = 0, // 不可用
	INCHANNEL_FREE,// 空闲
	INCHANNEL_BUSY,// 被占用
};

typedef struct _type_input_channel// 连接输入的通道
{
	uint64_t listener_id; 		// 通道ID
	uint64_t tarker_id;
	uint16_t tarker_index;
	uint16_t listener_index; 		// 通道ID索引
	timetype timetimp;// 连接时时间戳
	enum input_channel_status status;// 通道的状态
	struct list_head list;			// 用于已连接连接表的链表管理，
}TInChannel,*T_pInChannel;

#endif
