/*
**host_timer.c
**2015-12-14
**系统超时机制的基础模块
*/

#include "host_timer.h"

void host_timer_state_avail( timetype timeout, host_timer* target )
{
	assert( target );
	host_timer_update( timeout, target);
}

void host_timer_start( timetype timeout, host_timer* target )
{
	assert( target );
	target->running = true;
	target->elapsed = false;	// 超时标志
	target->count_time = timeout;
	target->start_time = get_current_time();
}

inline void host_timer_stop( host_timer* target )
{
	assert( target );
	target->elapsed = false;
	target->running = false;
}

void host_timer_update( timetype timeout, host_timer* target )
{
	assert( target );
	target->running = true;
	target->elapsed = false;	// 超时标志
	target->count_time = timeout;
	target->start_time = get_current_time();
}

bool host_timer_timeout( host_timer* target )
{
	assert( target );
	if( target->running && !target->elapsed )
        {
        	uint32_t elapsed_ms;
            	timetype current_time = get_current_time();
            	elapsed_ms = (uint32_t)(current_time - target->start_time);

           	if( elapsed_ms > target->count_time )
            	{
                	target->elapsed = true;
            	}		
	}

	return target->elapsed;
}

