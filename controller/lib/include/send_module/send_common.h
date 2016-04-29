/**
*file:send_common.h
*date:2016-4-29
*
*/

//********************************
//ϵͳ���͹���ģ�齨��
//
//********************************

#ifndef __SEND_COMMON_H__
#define __SEND_COMMON_H__

#include <semaphore.h>
#include "send_work_queue.h"
#include "wait_message.h"
#include "circular_link_queue.h"
#include "control_data.h"
#include "send_interval.h"

#define SEND_DOUBLE_QUEUE_EABLE // ʹ�����ڣ����ڷ���˫����

extern sem_t sem_waiting; // ���͵ȴ��ź����������߳̿ɼ�
extern sdpwqueue net_send_queue;// �������ݷ��͹�������

extern void init_network_send_queue( void );
extern void init_sem_wait_can( void );
extern void destroy_network_send_work_queue( void );
extern void system_packet_save_send_queue( tx_data tnt );

#endif
