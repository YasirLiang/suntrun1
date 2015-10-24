#ifndef __CONTOL_DATA_H__
#define __CONTOL_DATA_H__

#include <pthread.h>
#include "jdksavdecc_world.h"

typedef struct data_control
{
	pthread_mutex_t mutex; // 互斥对象
	pthread_cond_t cond;	// 条件变量，可以睡眠
	int active;				// 告诉线程此数据结构是否活动
}data_control;

#endif

