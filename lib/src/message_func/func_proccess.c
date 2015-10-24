#include "func_proccess.h"

fcwqueue fcwork_queue;

void init_func_command_work_queue( void )
{
	bool is_su = false;
	
	is_su = controll_init( &fcwork_queue.control );
	if( !is_su )
		DABORT( is_su );
	init_queue( &fcwork_queue.work );
	
	is_su = controll_activate( &fcwork_queue.control );
	if( !is_su )
		DABORT( is_su );
}

