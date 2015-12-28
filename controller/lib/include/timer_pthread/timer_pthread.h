/*
**timer_pthread.h
**2015-12-28
*/

#ifndef __TIMER_PTHREAD_H__
#define __TIMER_PTHREAD_H__

#include <pthread.h>
#include "host_controller_debug.h"

int timer_start( pthread_t *pthread_id, void *(*timer_pro)(void *), void*input_param );
int timer_stop( pthread_t pthread_id );

#endif

