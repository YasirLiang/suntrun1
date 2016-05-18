/*
**conferenc_recieve_unit.h
**9-3-2016
**
*/

// ****************************
// file built 2016-3-9
// 建立会议系统连接表管理机制
// 系统会议单元单元接收模块
// ****************************

#ifndef __CONFERENCE_RECIEVE_UNIT_H__
#define __CONFERENCE_RECIEVE_UNIT_H__

#include "input_channel.h"
#include "descriptor.h"
#include "entity.h"
#include "global.h"
#include "connector_subject.h"

#define CONFERENCE_RECIEVE_UNIT_NAME conference_uint_name
#define CONFERENCE_RECIEVE_UNIT_IN_CHNNEL_MAX_NUM 4// 最大输入通道
#define CONFERENCE_MUTICASTED_INPUT_CHANNEL conference_uint_recieve_uint_input// 定义广播会议单元的连接通道

typedef struct _type_conference_recieve_model
{
	uint64_t listener_id;
	solid_pdblist solid_pnode; // 指向adp链表节点
	desc_pdblist desc_pnode;// 指向desc链表节点
	struct list_head channel_list;// 通道节点表；表节点类型是TInChannel_universe
	struct list_head *p_ccu_muticast_channel;// 中央连接单元对本模块广播的输入通道定义表节点类型是TInChannel_universe
	uint16_t channel_num;// 通道数
	uint16_t chanel_connect_num;// 模块已连接数
	struct list_head list;
}tconference_recieve_model, *T_Ptrconference_recieve_model;// 会议接收单元

typedef struct _type_conference_recieve_model_muticasted_pro
{
	tconference_recieve_model *p_cur_recieve_unit;// 当前接收模块
	TInChannel_universe *p_cur_crecv_channel_node;// 当前接收的通道节点指针
}tcrModelMuticasted_pro;// 中央广播者连接会议接收单元处理参数

#endif
