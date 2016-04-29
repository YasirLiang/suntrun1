/*
**check_timer.c 系统检查定时器线程及其处理
**2015-12-28
**注:可通过如下方式使定时器线程工作:	
	pthread_t timer_pthread;
	check_timer_create( &timer_pthread );在timerfd可用的情况下，没有必要这么做。
*/

#include "check_timer.h"
#include "avdecc_funhdl.h"
#include "wait_message.h"
#include "send_common.h"
#include "send_interval.h"

pthread_t check_timer_pthread;// 检查定时器线程

void* check_timer_fn( void *param )
{
	unsigned long us_per_ms = 1000;
        unsigned long interval_ms = TIME_PERIOD_25_MILLISECONDS;
		
	while( 1 )
	{
		struct timeval tempval;
	        tempval.tv_sec = interval_ms/1000;  
	        tempval.tv_usec = (interval_ms%1000)*us_per_ms;  
	        select( 0, NULL, NULL, NULL, &tempval ); 
		check_timer_proccess();
	}

	pthread_exit( NULL );
}

void check_timer_proccess( void )
{
	//terminal_mic_speak_limit_time_manager_event();
    	time_tick_event( endpoint_list, command_send_guard );
	profile_system_file_write_timeouts();
	//muticast_connector_time_tick();

	if( is_inflight_timeout && is_wait_messsage_active_state() )
	{
		set_wait_message_status( WAIT_TIMEOUT );	
		sem_post( &sem_waiting );
	}
	
	is_inflight_timeout = false; 

	if((is_wait_messsage_active_state()) && (is_send_interval_timer_timeout()))// check uart or resp data timeout
	{
		sem_post( &sem_waiting ); 
	}
}

int check_timer_create( pthread_t *check_pthread )
{
	int rc = 0;
	rc = timer_start( check_pthread,  check_timer_fn, NULL );
	if( rc )
	{
		DEBUG_INFO( "check timer create faild!exit!" );
		exit( -1 );
	}

	check_timer_pthread = *check_pthread;

	return rc;
}

void check_timer_destroy( void )
{
	timer_stop( check_timer_pthread );
}

