/*
**conference_transmit_unit.h
**9-3-2016
**
*/

// ********************************
// file built 2016-3-9
// 建立会议系统连接表管理机制
// 会议单元发送模块
// ********************************

#ifndef __CONFERENCE_TRANSMIT_UNIT_H__
#define __CONFERENCE_TRANSMIT_UNIT_H__

#define CONFERENCE_OUTPUT_INDEX 0// 定义为零通道输出
#define CONFERENCE_OUTCHANNEL_MAX_NUM 8// 定义模块最大输出数量
#define CONFERENCE_PER_MODEL_OUT_MAX_NUM 2// 定义同时能最大的输出数量

typedef struct _confenrence_trans_model
{
	uint64_t tarker_id;
	tmnl_pdblist confenrence_node;
	host_timer model_speak_time;
	TOutChannel out_ch; // 输出通道，表为空则无输出
	struct list_head list; 
}tconference_trans_model, *tconference_trans_pmodel;// 会议传输模块

#endif