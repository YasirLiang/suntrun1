/*
**timer_pthread.c
**2015-12-28
*/

#include "timer_pthread.h"

int timer_start( pthread_t *pthread_id, void *(*timer_pro)(void *), void* input_param )
{
	int rc = pthread_create( pthread_id, NULL, timer_pro, input_param );

	return rc;
}

int timer_stop( pthread_t pthread_id )
{
	pthread_cancel( pthread_id );
	pthread_join( pthread_id, NULL );
	
	return 0;
}

