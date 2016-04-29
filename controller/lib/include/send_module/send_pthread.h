/*send_pthread.h
**Date:2015-11-5
**
**
*/

#ifndef __SEND_PTHREAD_H__
#define __SEND_PTHREAD_H__

#include <pthread.h>

#ifdef __DEBUG__
#ifndef __TEST_SEND_QUEUE__
//#define __TEST_SEND_QUEUE__ // 测试发送网络数据的发送队列
#endif
#endif

//#define __NOT_USE_SEND_QUEUE_PTHREAD__ // 不使用发送队列的大数据发送机制

extern int pthread_send_network_create( pthread_t *send_pid );

#endif
