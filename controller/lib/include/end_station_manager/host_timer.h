/*
**host_timer.h
**2015-12-14
**ϵͳ��ʱ���ƵĻ���ģ��
*/

#ifndef __HOST_TIMER_H__
#define __HOST_TIMER_H__

#include "host_time.h"

typedef struct host_timer			// ������ʱ��
{
	bool running;					// ����
	bool elapsed;					// ��ʱ��־
	uint32_t start_time;			// ��ʼʱ��
	uint32_t count_time;			// ��ʱʱ��
}host_timer;

extern void host_timer_state_avail( timetype timeout, host_timer* target );
extern void host_timer_start( timetype timeout, host_timer* target );
extern inline void host_timer_stop( host_timer* target );
extern void host_timer_update( timetype timeout, host_timer* target );
extern bool host_timer_timeout( host_timer* target );


#endif
