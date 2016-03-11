/*
**out_channel.h
**11-3-2016
**
*/

// ****************************
// file built 2016-3-11
// 建立会议系统连接表管理机制
// 输出通道基础模块
// ****************************

#ifndef __OUTPUT_CHANNEL_H__
#define __OUTPUT_CHANNEL_H__

enum output_channel_status// 输出通道的状态
{
	OUTCHANNEL_UNAVAILABLE,// 不可用
	OUTCHANNEL_OFFLINE,// 可连接
	OUTCHANNEL_ONLINE = 0, // 已连接在线
};

typedef struct _type_output_channel// 连接输入的通道
{
	uint64_t tarker_id; 		// 通道ID
	uint16_t tarker_index; 	// 通道ID索引
	enum output_channel_status status;// 通道的状态
	timetype timetimp;			// 连接时时间戳
	struct list_head list;			// 用于已连接连接表的链表管理，
}TOutChannel;


#endif
