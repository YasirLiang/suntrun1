/*
**connector_subject.h
**10-3-2016
**
*/

// *******************************************************************
// file built 2016-3-10
// 建立会议系统连接表管理机制
// 系统连接者被观察者
// 系统中的存在观察者:主机的接收模块;系统的会议单元连接模块（即发送与接收模块）;
//				主机的广播（发送）模块;整个系统的连接管理模块；会议单元的麦克风模块
// ********************************************************************

#ifndef __CONNECT_SUBJECT_H__
#define __CONNECT_SUBJECT_H__

#define OBSERVER_LIST_MAX_NUM 20

#include "jdksavdecc_world.h"

typedef struct _type_command
{
	uint16_t data_type;// 数据类型
	uint16_t msg_type;// 消息类型
	uint16_t desc; // 后面可加aecp \app等的命令 ,desc 类型
	uint16_t desc_index;
	int msg_resp_status;// 命令的返回值状态;-1为发送数据超时状态
}tsubject_control_command;

typedef struct _elem_data
{
	uint64_t listener_id;
	uint64_t tarker_id;// 当响应数据是rx_state时，此时为流ID
	uint16_t listener_index;
	uint16_t tarker_index;
	bool connect_flag;// true :cnnt success :false disconnect success! 前面的结构定义
	tsubject_control_command ctrl_msg;// 控制命令后面的结构定义
}subject_data_elem;

typedef struct 
{
	void (*update)( subject_data_elem data );
}observer_t;

typedef struct _subject_t
{
	struct _elem_data sub_data;
	int observer_num;
	observer_t *observer_list[OBSERVER_LIST_MAX_NUM];
}subject_t;

extern subject_t gconnector_subjector;

void init_connector_subjector( void );

// **************************************************
int attach_observer( subject_t *subject, observer_t *observer );
int delect_observer( subject_t *subject, observer_t *observer );
int set_subject_data( subject_data_elem data, subject_t *subject );
void notify_observer( subject_t *subject );
void init_subject( subject_t *subject );
void init_observer( observer_t *observer, void (*update)(subject_data_elem) );

// *****************************************************//

#endif
