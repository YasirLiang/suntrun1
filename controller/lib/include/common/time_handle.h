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
	OVER_TMN_RESTART,// 定时注册
	CSEND_GAP_HANDLE,
	COPRT_GAP_HANDLE,
	CCTRL_GAP_HANDLE,
	CCTRL_STOP_HANDLE,
	CGPS_GAP_HANDLE,
	SIGN_IN_LATE_HANDLE,// 超时签到
	WAIT_485_STATE_SW,
	TMN_RCV_INTERVAL_HANDLE,
	CHM_MUSIC_HANDLE,
	QUERY_TMN_GAP,
	BUZZER_TIME,
	CPT_RCV_GAP,
	MATRIX_RCV_GAP,
	SEND_MAIN_STATE_INTERVAL,
	NET_SEND_INTERVAL,
	MIND_UPPER_CMPT_SIGN_RESULT,// 提醒上位机终端签到
	SENDING_INTERVAL,// 发送间隔
	CCU_TRANS_CONNECT_BEGIN_TIME, // ccu 发送开始超时时间
	SYSTEM_SQUEUE_SEND_INTERVAL, // 系统发送队列的发送间隔
	CHECK_SYSTEM_SEND_QUEUE_INDEX, //  检查系统是否发送数据完成
	CHANGE_MUTICASTOR_TIMEOUT_INDEX,// 改变广播者的超时时间
	CHANGE_MUTICASTOR_INTERVAL,// 改变广播者的间隔时间
	STOP_CAMERA_INTERVAL,// 停止摄像头
	DISCUSS_MODE_SPEAK_AFTER,// 发言
	SIG_TMNL_REGISTER,
	
	OVER_TIME_ITEMS_NUM
}enum_overtime_handle;

void over_time_set( enum_overtime_handle handle, uint32_t over_time );
bool over_time_listen( enum_overtime_handle handle );
void over_time_stop( enum_overtime_handle handle );

#endif
