/*
  *file:system_1722_recv_handle.h
  *data:2016-07-08
  *Author:梁永富
  *descriptor:增加双消息链式队列机制，
  *把数据的处理权交由数据处理线程，
  *如果通过这方法还不能解决问题(大数据量接收与处理的问题)，
  *再考虑接收缓冲区与应用缓冲区机制
  *
  */

#ifndef __SYSTEM_1722_RECV_HANDLE_H__
#define __SYSTEM_1722_RECV_HANDLE_H__

#ifndef RECV_DOUBLE_LINK_QUEUE_EN
//#define RECV_DOUBLE_LINK_QUEUE_EN // 使能接收双链式队列,经测试，此机制不能运用在系统中
#endif

#ifdef RECV_DOUBLE_LINK_QUEUE_EN
#include "circular_link_queue.h"
#include "jdksavdecc_frame.h"
#include "control_data.h"

typedef struct _type_recv_raw_data
{
	struct jdksavdecc_frame frame;
	uint32_t last_recv_tick;
}trecv_raw_data;

typedef struct _trecv_raw_queue_work_node   // raw数据接收队列的工作节点
{
	queue_node *next; 			// 工作队列节点的链表区(即队列节点)
	trecv_raw_data job_data; 			// 发送数据命令格式
}recv_raw_queue_wnode, *p_recv_raw_queue_wnode;

typedef struct _trecv_raw_data_work_queue // 系统数据发送队列
{
	data_control control;	// 控制数据
	queue work;			// 工作队列
}recv_raw_data_wqueue;

int system_1722_recv_handle_raw_data(void);
p_recv_raw_queue_wnode system_1722_recv_handle_get_queue_node(void);
int system_1722_recv_handle_save_queue_node(struct jdksavdecc_frame *frame);
void system_1722_recv_handle_init(void);
void system_1722_recv_handle_destroy( void );

#else
int system_1722_recv_handle_raw_data(void);
void system_1722_recv_handle_init(void);
void system_1722_recv_handle_destroy( void );
#endif

#endif