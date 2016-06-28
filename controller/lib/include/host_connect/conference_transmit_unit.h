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

#include "connector_subject.h"
#include "output_channel.h"
#include "terminal_common.h"
#include "descriptor.h"
#include "host_timer.h"
#include "global.h"

#define CONFERENCE_OUTPUT_INDEX conference_uint_transmit_uint_ouput// 定义为零通道输出
#define CONFERENCE_OUTCHANNEL_MAX_NUM 8// 定义模块最大输出数量
#define CONFERENCE_PER_MODEL_OUT_MAX_NUM 2// 定义同时能最大的输出数量

#define CONFERENCE_TRANSMIT_UNIT_NAME conference_uint_name

typedef struct _confenrence_trans_model
{
	uint64_t tarker_id;
	tmnl_pdblist confenrence_node;
	host_timer model_speak_time;// 是会议终端已发言时间
	TOutChannel out_ch; // 输出通道，表为空则无输出
	struct list_head list; 
}tconference_trans_model, *tconference_trans_pmodel;// 会议传输模块

// *************************************************
void  trans_model_node_insert_to_list( tconference_trans_pmodel p_new_node);
int conference_transmit_unit_init( const uint8_t *frame, int pos, size_t frame_len, const uint64_t endtity_id, const desc_pdblist desc_node );
int trans_model_unit_connect( uint64_t tarker_id, const tmnl_pdblist p_tmnl_node );// return -1; means that there is no ccu reciever model 
int trans_model_unit_disconnect( uint64_t tarker_id, const tmnl_pdblist p_tmnl_node ); // return -1 means talker not connect
void trans_model_unit_update( subject_data_elem connect_info );// 更新传输模块的连接状态, 并发送通知会议系统协议的消息
void conference_transmit_model_init( void );
int conference_transmit_model_node_destroy( uint64_t tarker_id );

// *************************************************//

#endif
