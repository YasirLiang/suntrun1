/*send_work_queue.h
**Date:2015-11-5
**
**
*/

#ifndef __SEND_WORK_QUEUE_H__
#define __SEND_WORK_QUEUE_H__

#include "circular_link_queue.h"
#include "system_packet_tx.h"

typedef struct _tsend_data_pthread_queue_work_node   // ����Э���������ݷ��Ͷ��еĹ����ڵ�
{
	queue_node *next; 			// �������нڵ��������(�����нڵ�)
	tx_data job_data; 			// �������������ʽ
}sdpqueue_wnode, *p_sdpqueue_wnode;

typedef struct _tsend_data_pthread_work_queue // ϵͳ���ݷ��Ͷ���
{
	data_control control;	// ��������
	queue work;			// ��������
}sdpwqueue;

extern sdpwqueue net_send_queue;// �������ݷ��͹�������

extern void init_network_send_queue( void );
extern p_sdpqueue_wnode send_queue_message_get( sdpwqueue* send_wq );
extern int send_work_queue_message_save( tx_data* p_queue_msg );// ע:p_queue_msg�е�frameԪ����ָ���ѷ���ѿռ�Ŀռ�


#endif