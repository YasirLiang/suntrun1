/*
  *file:system_1722_recv_handle.h
  *data:2016-07-08
  *Author:������
  *descriptor:����˫��Ϣ��ʽ���л��ƣ�
  *�����ݵĴ���Ȩ�������ݴ����̣߳�
  *���ͨ���ⷽ�������ܽ������(�������������봦�������)��
  *�ٿ��ǽ��ջ�������Ӧ�û���������
  *
  */

#ifndef __SYSTEM_1722_RECV_HANDLE_H__
#define __SYSTEM_1722_RECV_HANDLE_H__

#ifndef RECV_DOUBLE_LINK_QUEUE_EN
//#define RECV_DOUBLE_LINK_QUEUE_EN // ʹ�ܽ���˫��ʽ����,�����ԣ��˻��Ʋ���������ϵͳ��
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

typedef struct _trecv_raw_queue_work_node   // raw���ݽ��ն��еĹ����ڵ�
{
	queue_node *next; 			// �������нڵ��������(�����нڵ�)
	trecv_raw_data job_data; 			// �������������ʽ
}recv_raw_queue_wnode, *p_recv_raw_queue_wnode;

typedef struct _trecv_raw_data_work_queue // ϵͳ���ݷ��Ͷ���
{
	data_control control;	// ��������
	queue work;			// ��������
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