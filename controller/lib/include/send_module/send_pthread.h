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
//#define __TEST_SEND_QUEUE__ // ���Է����������ݵķ��Ͷ���
#endif
#endif

//#define __NOT_USE_SEND_QUEUE_PTHREAD__ // ��ʹ�÷��Ͷ��еĴ����ݷ��ͻ���

extern int pthread_send_network_create( pthread_t *send_pid );

#endif
