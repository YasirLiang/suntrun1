/*send_pthread.h
**Date:2015-11-5
**
**
*/

#ifndef __SEND_PTHREAD_H__
#define __SEND_PTHREAD_H__

#include <semaphore.h>
#include "send_work_queue.h"
#include "wait_message.h"
#include "circular_link_queue.h"
#include "control_data.h"
#include "send_interval.h"

#ifdef __DEBUG__
#ifndef __TEST_SEND_QUEUE__
//#define __TEST_SEND_QUEUE__ // ���Է����������ݵķ��Ͷ���
#endif
#endif

//#define __NOT_USE_SEND_QUEUE_PTHREAD__ // ������ʹ�÷��Ͷ��еĴ����ݷ��ͻ���

extern sem_t sem_waiting; // ���͵ȴ��ź����������߳̿ɼ�
extern sdpwqueue net_send_queue;// �������ݷ��͹�������

extern void init_network_send_queue( void );
extern void init_sem_wait_can( void );
extern void destroy_network_send_work_queue( void );
int thread_send_func( void *pgm );
extern int pthread_send_network_create( pthread_t *send_pid );


#endif
