/*
*file:log.c
*build date:2016-4-6
*
*/

/**
//描述:信息采集系统框架源文件
//
//
*/

#include "log.h"
#include "enum.h"

#define vsprintf_s vsnprintf
#define InterlockedExchangeAdd __sync_fetch_and_add

static void default_log( void *log_user_obj, int32_t log_level, const char *log_msg, int32_t time_stamp_ms )
{
}

void log_init( tstrlog_base *base_log )
{
	assert( base_log );
	if( base_log == NULL )
		return;
	
	memset( base_log, 0, sizeof(tstrlog_base) );
	base_log->log_level = LOGGING_LEVEL_ERROR;
	base_log->read_index = 0;
	base_log->write_index = 0;
	base_log->user_obj = NULL;
	base_log->callback_func = default_log;
	base_log->missed_log_event_cnt = 0;
}


void set_log_level( int32_t new_log_level, tstrlog_base *base_log )
{
	assert( base_log );
	if( base_log == NULL )
		return;
	
	base_log->log_level = new_log_level;
}

void post_log_msg( tstrlog_base *base_log, int32_t level, const char *fmt, ... )
{
	uint32_t *p_write = NULL, read_index;
	assert( base_log );
	if( base_log == NULL )
		return;

	if (level <= base_log->log_level)
	{
		va_list arglist;
		uint32_t index;

		p_write = &base_log->write_index;
		read_index = base_log->read_index;
		if ((*p_write - read_index) > LOG_BUF_COUNT)
		{
			base_log->missed_log_event_cnt++;
			return;
		}
		
		index = InterlockedExchangeAdd(p_write, 1);
		va_start(arglist, fmt);
		vsprintf_s(base_log->log_buf[index % LOG_BUF_COUNT].msg, sizeof(base_log->log_buf[0].msg), fmt, arglist);  // Write to log_buf using write_index
		va_end(arglist);
		base_log->log_buf[index % LOG_BUF_COUNT].level = level;
		base_log->log_buf[index % LOG_BUF_COUNT].time_stamp_ms = 0;

		if( NULL != base_log->post_log_event )
		{
			base_log->post_log_event();
		}
	}
}

void set_log_callback(void (*new_log_callback) (void *, int32_t, const char *, int32_t), void *p, tstrlog_base *base_log )
{
	assert( base_log );
	if( base_log == NULL )
		return;

	base_log->callback_func = new_log_callback;
	base_log->user_obj = p;
}

uint32_t missed_log_event_count( tstrlog_base *base_log )
{
	assert( base_log );
	if( base_log == NULL )
		return 0;
	
    	return base_log->missed_log_event_cnt;
}

