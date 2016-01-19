/*
**time_handle.c
**2016/1/19
**系统的超时公共模块源文件
*/

#include "time_handle.h"
#include "host_timer.h"

host_timer time_handle_pro[OVER_TIME_ITEMS_NUM];

void over_time_set( enum_overtime_handle handle, uint32_t over_time )
{
	host_timer_start( over_time, &time_handle_pro[handle] );
}

bool over_time_listen( enum_overtime_handle handle )
{
	return host_timer_timeout( &time_handle_pro[handle] );
}

void over_time_stop( enum_overtime_handle handle )
{
	host_timer_stop( &time_handle_pro[handle] );
}

