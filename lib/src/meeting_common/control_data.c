/*control_data.c
**
**
**
*/

#include "control_data.h"
#include "host_controller_debug.h"

bool controll_activate( data_control* p_controll )
{
	DABORT( p_controll );
	
	if( pthread_mutex_lock(&p_controll->mutex) )
		return false;
	if( phread_mutex_unlock(&(p_controll->mutex)) )
		return false;
	if( pthread_cond_signal(&p_controll->cond) )
		return false;

	p_controll->active = 1;

	return true;
}

bool controll_deactivate( data_control* p_controll )
{
	DABORT( p_controll );
	
	if( pthread_mutex_lock(&p_controll->mutex) )
		return false;
	if( phread_mutex_unlock(&(p_controll->mutex)) )
		return false;
	if( pthread_cond_signal(&p_controll->cond) )
		return false;

	p_controll->active = 0;

	return true;
}

bool controll_init( data_control *p_controll )
{
	DABORT( p_controll );
	
	if( pthread_mutex_init( &p_controll->mutex, NULL ) )
		return false;
	if( pthread_cond_init( &p_controll->cond, NULL ))
		return false;

	p_controll->active = 0;

	return true;
}

bool controll_destroy( data_control *p_controll )
{
	DABORT( p_controll );
	
	if( pthread_mutex_destroy( &p_controll->mutex) )
		return false;
	if( pthread_cond_destroy( &p_controll->cond) )
		return false;
		
	p_controll->active = 1;

	return true;
}


