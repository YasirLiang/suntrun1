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
//#define __TEST_SEND_QUEUE__ // 测试发送网络数据的发送队列
#endif
#endif

//#define __NOT_USE_SEND_QUEUE_PTHREAD__ // 决定不使用发送队列的大数据发送机制

extern sem_t sem_waiting; // 发送等待信号量，所有线程可见
extern sdpwqueue net_send_queue;// 网络数据发送工作队列

extern void init_network_send_queue( void );
extern void init_sem_wait_can( void );
extern void destroy_network_send_work_queue( void );
int thread_send_func( void *pgm );
extern int pthread_send_network_create( pthread_t *send_pid );


#endif
