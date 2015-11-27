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

extern sem_t sem_waiting; // ���͵ȴ��ź����������߳̿ɼ�
void init_sem_wait_can( void );
int pthread_send_network_create( pthread_t *send_pid );


#endif
