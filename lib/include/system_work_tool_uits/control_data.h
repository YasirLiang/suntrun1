#ifndef __CONTOL_DATA_H__
#define __CONTOL_DATA_H__

#include "jdksavdecc_world.h"
#include <pthread.h>

typedef struct data_control
{
	pthread_mutex_t mutex; // 互斥对象
	pthread_cond_t cond;	// 条件变量，可以睡眠
	int active;				// 告诉线程此数据结构是否活动
}data_control;

bool controll_activate( data_control* p_controll );
bool controll_deactivate( data_control* p_controll );
bool controll_init( data_control *p_controll );
bool controll_destroy( data_control *p_controll );


#endif

