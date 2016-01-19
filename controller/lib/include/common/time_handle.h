/*
**time_handle.h
**2016/1/19
**系统的超时公共模块头文件
*/

#ifndef __TIME_HANDLE_H__
#define __TIME_HANDLE_H__

#include "jdksavdecc_world.h"

typedef enum enum_overtime_handle
{
	TASK_OTIME_HANDLE = 0,
	CASK_OTIME_HANDLE, 
	TRGST_OTIME_HANDLE,
	WAIT_TMN_RESTART,
	CSEND_GAP_HANDLE,
	COPRT_GAP_HANDLE,
	CCTRL_GAP_HANDLE,
	CCTRL_STOP_HANDLE,
	CGPS_GAP_HANDLE,
	SIGN_IN_LATE_HANDLE,
	WAIT_485_STATE_SW,
	TMN_RCV_INTERVAL_HANDLE,
	CHM_MUSIC_HANDLE,
	QUERY_TMN_GAP,
	BUZZER_TIME,
	CPT_RCV_GAP,
	SEND_MAIN_STATE_INTERVAL,
	NET_SEND_INTERVAL,
	OVER_TIME_ITEMS_NUM
}enum_overtime_handle;

void over_time_set( enum_overtime_handle handle, uint32_t over_time );
bool over_time_listen( enum_overtime_handle handle );
void over_time_stop( enum_overtime_handle handle );

#endif
