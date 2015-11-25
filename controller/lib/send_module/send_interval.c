/*send_interval.c
**date: 2015/11/25
**
**
*/

#include "send_interval.h"

static tsend_interval_native send_interval_pro;

void send_interval_timer_state_avail( timetype timeout, tsend_interval_native* target )
{
	assert( target );
	send_interval_timer_update( timeout, target );
}

void send_interval_timer_start( timetype timeout, tsend_interval_native* target )
{
	assert( target );
	target->timeout.running = true;
	target->timeout.elapsed = false;	// 超时标志
	target->timeout.count_time = timeout;
	target->timeout.start_time = get_current_time();
}

void send_interval_timer_stop(  tsend_interval_native* target )
{
	assert( target );
	target->timeout.running = false;
	target->timeout.elapsed = false;	// 超时标志
}

void send_interval_timer_update( timetype timeout, tsend_interval_native* target )
{
	assert( target );
	target->timeout.running = true;
	target->timeout.elapsed = false;	// 超时标志
	target->timeout.count_time = timeout;
	target->timeout.start_time = get_current_time();
}

bool send_interval_timer_timeout( tsend_interval_native* target )
{
	assert( target );
	if( target->timeout.running && !target->timeout.elapsed )
        {
        	uint32_t elapsed_ms;
            	timetype current_time = get_current_time();
            	elapsed_ms = (uint32_t)( current_time - target->timeout.start_time );

           	if(elapsed_ms > target->timeout.count_time)
            	{
                	target->timeout.elapsed = true;
            	}		
	}

	return target->timeout.elapsed;
}

bool is_send_interval_timer_timeout( void )
{
	return (send_interval_timer_timeout( &send_interval_pro ));
}

void uart_resp_send_interval_timer_start( void )
{
	send_interval_timer_start( SEND_INTERVAL_TIME, &send_interval_pro );
}

void send_interval_init( void )
{
	send_interval_timer_stop( &send_interval_pro );
	send_interval_pro.send_interval_flag = INTERVAL_IDLE;
}

int set_send_interval_idle_state( void )
{
	if( send_interval_pro.send_interval_flag != INTERVAL_WAIT )
	{
		return -1;
	}
	else
	{
		send_interval_pro.send_interval_flag = INTERVAL_IDLE;
	}

	return 0;
}

int set_send_interval_primed_state( void )
{
	if( send_interval_pro.send_interval_flag != INTERVAL_IDLE )
	{
		return -1;
	}
	else
	{
		send_interval_pro.send_interval_flag = INTERVAL_PRIMED;
	}

	return 0;
}

int set_send_interval_wait_state( void )
{
	if( send_interval_pro.send_interval_flag != INTERVAL_PRIMED )
	{
		return -1;
	}
	else
	{
		send_interval_pro.send_interval_flag = INTERVAL_WAIT;
	}

	return 0;
}

bool is_send_interval_idle_state( void )
{
	return (send_interval_pro.send_interval_flag == INTERVAL_IDLE);
}

bool is_send_interval_primed_state( void )
{
	return (send_interval_pro.send_interval_flag == INTERVAL_PRIMED);
}

bool is_send_interval_wait_state( void )
{
	return (send_interval_pro.send_interval_flag == INTERVAL_WAIT );
}


