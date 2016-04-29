/**
*file:send_common.h
*date:2016-4-29
*
*/

//********************************
//系统发送公共模块建立
//
//********************************

#ifndef __SEND_COMMON_H__
#define __SEND_COMMON_H__

#include <semaphore.h>
#include "send_work_queue.h"
#include "wait_message.h"
#include "circular_link_queue.h"
#include "control_data.h"
#include "send_interval.h"

#define SEND_DOUBLE_QUEUE_EABLE // 使能网口，串口发送双队列

extern sem_t sem_waiting; // 发送等待信号量，所有线程可见
extern sdpwqueue net_send_queue;// 网络数据发送工作队列

extern void init_network_send_queue( void );
extern void init_sem_wait_can( void );
extern void destroy_network_send_work_queue( void );
extern void system_packet_save_send_queue( tx_data tnt );

#endif
