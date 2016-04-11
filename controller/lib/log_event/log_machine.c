/*
*file:log_machine.c
*build date:2016-4-6
*
*/

/**
//描述:信息采集系统框架源文件
//
//
*/
#include "log_machine.h"
#include "enum.h"

tstrlog_pimp gp_log_imp = NULL;

void log_machine_post_event( void )
{
	assert( gp_log_imp );
	if( gp_log_imp != NULL )
		sem_post( &gp_log_imp->log_waiting );
}

void *log_machine_thread( void* param )
{
	uint32_t write_index, read_index;
	while( true )
	{
		sem_wait( &gp_log_imp->log_waiting );

		write_index = gp_log_imp->log.write_index;
		read_index = gp_log_imp->log.read_index;
		if( (write_index- read_index) > 0 )
		{
			gp_log_imp->log.callback_func( gp_log_imp->log.user_obj,
										gp_log_imp->log.log_buf[read_index % LOG_BUF_COUNT].level,
										gp_log_imp->log.log_buf[read_index % LOG_BUF_COUNT].msg,
										gp_log_imp->log.log_buf[read_index % LOG_BUF_COUNT].time_stamp_ms );

			gp_log_imp->log.read_index++;
		}
		else
		{
			break;
		}
	}

	return NULL;
}

tstrlog_pimp log_machine_create( void (*callback_func)(void *, int32_t, const char *, int32_t), int32_t log_level, void *user_obj )
{
	gp_log_imp = malloc( sizeof(tstrlog_imp) );
	if(gp_log_imp == NULL )
	{
		printf( "malloc failed: No space for tstrlog_imp malloc\n" );
	}
	else
	{
		gp_log_imp->log.log_init = log_init;
		gp_log_imp->log.log_init( &gp_log_imp->log );
		gp_log_imp->log.set_log_level = set_log_level;
		gp_log_imp->log.post_log_msg = post_log_msg;
		gp_log_imp->log.post_log_event = log_machine_post_event;
		gp_log_imp->log.set_log_callback = set_log_callback;
		gp_log_imp->log.missed_log_event_count = missed_log_event_count;
		gp_log_imp->log.callback_func = callback_func;
		gp_log_imp->log.log_level = log_level;
		gp_log_imp->log.user_obj = user_obj;

		sem_init( &gp_log_imp->log_waiting, 0, 0 );

		int rc;
		rc = pthread_create( &gp_log_imp->h_thread, NULL, log_machine_thread, NULL );
		if( rc )
		{
			printf("ERROR; return code from pthread_create() is %d\n", rc );
			if( NULL != gp_log_imp )
			{
				free( gp_log_imp );
				gp_log_imp = NULL;
			}
		}
	}
	
	return gp_log_imp;
}

void log_machine_destroy( void )
{
	if( NULL != gp_log_imp )
	{
		free( gp_log_imp );
		gp_log_imp = NULL;
	}
}
