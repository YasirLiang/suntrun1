/*
**check_timer.c ϵͳ��鶨ʱ���̼߳��䴦��
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
