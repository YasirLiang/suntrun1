/*
**central_control_recieve_unit.h
**9-3-2016
**
*/

// ****************************
// file built 2016-3-9
// 建立会议系统连接表管理机制
// 中心控制单元接收模块
// ****************************

#ifndef __CENTRAL_CONTROL_RECIEVE_UNIT_H__
#define __CENTRAL_CONTROL_RECIEVE_UNIT_H__

#include "entity.h"
#include "descriptor.h"
#include "input_channel.h"
#include "connector_subject.h"
#include "global.h"

//************************************

#define CCU_TR_MODEL_NAME central_control_unit_transmit_name// 中央控制接收发送模块名字
#define CCU_R_MODEL_NAME central_control_unit_name // 中央控制接收模块的名字

#define CCU_TR_MODEL_MAX_NUM (central_control_unit_max_num + 21) // 中央控制单元接收发送模块的最大总数
#define CCU_APIECE_TR_MODEL_CHANNEL_MAX_NUM 4 //中央控制单元每个接收发送模块的最大的通道数 
#define CCU_TR_MODEL_CHANNEL_MAX_NUM (CCU_TR_MODEL_MAX_NUM*CCU_APIECE_TR_MODEL_CHANNEL_MAX_NUM)// 中央控制单元接收发送模块的通道总数

//*****************************
#define CCU_TR_MODEL_RECIEVE_ENABLE // 使能中央控制单元接收发送模块的接收功能
#ifndef CCU_TR_MODEL_RECIEVE_ENABLE
#define CCU_TR_MODEL_RECIEVE_DISABLE// 不使能中央控制单元接收发送模块的接收功能
#endif
//*****************************////

enum recieve_model_state// 输入模块通道的状态--已初始化未连接-已初始化连接-未初始化
{
	CCU_RECIEVE_MODEL_UNINIT = 0,// 未初始化且不可用，是节点可以设置的状态
	CCU_RECIEVE_MODEL_OK,// 已初始化未连接
	CCU_RECIEVE_MODEL_ALL_CHANNEL_INIT,
};

typedef struct _type_central_control_recieve_model// 接收模块
{
	uint64_t entity_id;
	solid_pdblist solid_pnode; // 指向adp链表节点
	desc_pdblist desc_pnode;// 指向desc链表节点
	TInChannel connect_channel_head;// 模块已连接表输入通道
	TInChannel unconnect_channel_head;// 模块已连接表输入通道
	enum recieve_model_state model_state;
	uint8_t channel_num;// 通道数
	uint8_t chanel_connect_num;// 通道已连接数
	uint32_t model_last_time;// 最后连接时间 
}TccuRModel;

enum channel_list_pro_flags
{
	CH_ALLOT_PRIMED = 0,// 预处理
	CH_ALLOT_HANDLING, // 正在处理
	CH_ALLOT_FINISH// 处理完成
};

enum channel_pro_stype
{
	CH_DISCONNECT,
	CH_CONNECT
};

typedef struct _type_channel_alloction_proccess// 通道分配处理结构
{
	TInChannel *p_current_input_channel;// 指向通道表中当前可以被分配的模块,若为NULL，则表示无可用通道
	uint8_t elem_num;// 通道的个数
	uint8_t elem_can_use_num;// 能够使用的通道的总数
	uint8_t cnnt_num;// 已连接个数
	enum channel_list_pro_flags pro_eflags;// 处理过程参数
	enum channel_pro_stype pro_stype;
}tchannel_allot_pro;

extern uint16_t CCRU_canUsedInStreams;

// *****************************************//

// *****************************************
int init_central_control_recieve_unit_by_entity_id( const uint8_t *frame, int pos, size_t frame_len, const desc_pdblist desc_node, const uint64_t endtity_id );
void central_control_recieve_ccu_model_state_update( subject_data_elem connect_info );
int ccu_recv_model_talk( uint64_t  talker_id, uint16_t talker_index );
int ccu_recv_model_untalk( const uint64_t  talker_id, const uint16_t talker_index );
void central_control_recieve_uinit_init_list( void );
bool ccu_recv_model_talker_connected( uint64_t  talker_id, uint16_t talker_index );
bool ccu_recv_model_talker_connected_listener_id_index( uint64_t  talker_id, uint16_t talker_index, uint64_t *out_listen, uint16_t* listen_index );
extern void central_control_recieve_uinit_destroy(void);
extern uint8_t central_control_recieve_get_input_num(void);
extern void central_control_recieve_uinit_free_connect_node(uint64_t id);
extern void CCRU_inputUpate(void);

// *****************************************//

#endif

