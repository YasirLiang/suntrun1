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

typedef struct _type_input_channel
{
	uint64_t listener_id;
	uint16_t listen_index;
	struct list_head list;
}Input_Channel;

typedef struct _type_output_channel// 连接输入的通道
{
	uint16_t tarker_index;
	Input_Channel input_head; // 输入, 为空则无连接
	struct list_head list;
}TOutChannel, *T_pOutChannel;

#endif
