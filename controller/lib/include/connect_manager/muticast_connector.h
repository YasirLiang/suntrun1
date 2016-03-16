/*
**2015-12-14
**muticast_connector.h
*/

#ifndef __MUTICAST_CONNECTOR_H__
#define __MUTICAST_CONNECTOR_H__

#include "jdksavdecc_world.h"
#include "jdksavdecc_util.h"
#include "terminal_common.h"
#include "host_timer.h"
#include "descriptor.h"
#include "conference_recieve_unit.h"
#include "central_control_recieve_unit.h"


#define MAX_BD_CONNECT_NUM (SYSTEM_TMNL_MAX_NUM) // 最大连接数
#define MAX_CVNT_CONNECT_NUM		5 // 最大的与会连接失败的次数
#define CVNT_CONNECT_TIME_OUT  	10
#define CVNT_ONLINE_TIME_OUT  		30
#define CVNT_OUTLINE_TIME_OUT  	60

#define CVNT_MUTICAST_NAME CCU_TR_MODEL_NAME // 是中央处理接收发送模块
#define CVNT_MUTICAST_OUT_CHANNEL 0// 定义模块输出通道为0
#define CVNT_MUTICAST_IN_CHNNEL CONFERENCE_RECIEVE_UNIT_IN_CHNNEL// 定义会议接收单元输入通道为0

typedef enum _econventioner_state
{
	CVNT_OFFLINE,
	CVNT_ONLINE,
	CVNT_OUT_CONNECT,	// 未连接成功,update 成功时可改变为CVNT_ONLINE，或count为0时可置为CVNT_OFFLINE
	CVNT_STREAM_ON,
	CVNT_STREAM_OFF
}econventioner_state;

struct host_muticastor	// 主机的广播者
{
	uint16_t connect_num; 	// 已广播的数量
	uint16_t cvntr_total_num;// 与会者总数
	uint16_t current_index;  // 当前列表索引,用于超时连接表的处理
	uint16_t tarker_index;	// 索引
	uint64_t uid; 			// ID
	bool muticastor_exsit;	// 
};

typedef struct conventioner_cnnt_list_node	// 与会者连接列表节点 24个字节
{
	uint16_t state;							// 与会者的状态
	bool connect_flag;
	struct host_timer timeout;
	uint16_t count; // 从零开始计数，未连接上的次数 ，可以设置若大于一定的值 ，对其不再连接,也可以设置为零使其重新连接
	uint16_t listerner_index;
	uint64_t uid;
}conventioner_cnnt_list_node;

typedef enum _emuticast_node_pro
{
	CVNT_CHECK_PRIMED,	// 开始处理
	CVNT_CHECK_WAIT,	// 正在处理
	CVNT_CHECK_IDLE		// 处理完成
}emuticast_node_pro;

typedef struct conventioner_expend_node		// 处理连接状态的结构
{
	conventioner_cnnt_list_node* current_listener;	// 当前的被广播者
	emuticast_node_pro eelem_flags;		// 处理标志
	struct host_muticastor muticastor;				// 主机的广播者
}muticastor_connect_pro;

typedef int (*muticast_offline_callback)( uint64_t tarker_id, conventioner_cnnt_list_node* connect_node, bool success );// 广播连接表连接命令回调函数指针
typedef int (*muticast_online_callback)( uint64_t tarker_stream_id, uint64_t listern_stream_id, conventioner_cnnt_list_node* connect_node, bool success );// 广播连接表更新在线终端命令的回调函数指针

typedef struct _type_muticast_conventioner_callback
{
	uint64_t tarker_steam_id, listener_stream_id;
	conventioner_cnnt_list_node* p_cvnt_node;
	muticast_offline_callback p_offline_func;
	muticast_online_callback p_online_func;
}muticast_conventioner_callback;

int muticast_connector_connect( conventioner_cnnt_list_node* connect_node );
int muticast_connector_connect_callback( uint64_t tarker_id, conventioner_cnnt_list_node* connect_node, bool success );
int muticast_connector_proccess_online( conventioner_cnnt_list_node* connect_node );
int muticast_connector_proccess_online_callback( uint64_t tarker_stream_id, uint64_t listern_stream_id, conventioner_cnnt_list_node* connect_node, bool success );
int muticast_connector_proccess_outline( conventioner_cnnt_list_node* connect_node );
int muticast_connector_time_tick( void );
int muticast_connector_init( void );
void muticast_connector_destroy( void );
bool muticast_connector_connect_table_set( desc_pdblist desc_guard );

int muticast_connector_connect_table_init_node( const bool is_input_desc, const uint8_t *frame, int pos, size_t frame_len, const uint64_t endtity_id, const desc_pdblist desc_node );


#endif
