/*
**host_timer.h
**2015-12-14
**系统超时机制的基础模块
*/

#ifndef __HOST_TIMER_H__
#define __HOST_TIMER_H__

#include "host_time.h"

typedef struct host_timer			// 主机定时器
{
	bool running;					// 运行
	bool elapsed;					// 超时标志
	uint32_t start_time;			// 开始时间
	uint32_t count_time;			// 超时时间
}host_timer;

extern void host_timer_state_avail( timetype timeout, host_timer* target );
extern void host_timer_start( timetype timeout, host_timer* target );
extern inline void host_timer_stop( host_timer* target );
extern void host_timer_update( timetype timeout, host_timer* target );
extern bool host_timer_timeout( host_timer* target );


#endif
