/*
**check_timer.c 系统检查定时器线程及其处理
**2015-12-28
**
*/

#ifndef __CHECK_TIMER_H__
#define __CHECK_TIMER_H__

#include "timer_pthread.h"

void* check_timer_fn( void *param );
void check_timer_proccess( void );
int check_timer_create( pthread_t *check_pthread );
void check_timer_destroy( void );


#endif
