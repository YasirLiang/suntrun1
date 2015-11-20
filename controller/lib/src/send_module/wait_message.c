/**
*wait_message.c
*date:2015-11-5
*
*/

#include "wait_message.h"

struct wait_send_massge wait_msg;

void init_wait_message( void )
{
	wait_msg.emr = WAIT_IDLE;
	wait_msg.completions_status = 0;
}

int set_wait_message_idle_state( void )
{
	if( wait_msg.emr != WAIT_ACTIVE )
	{
		return -1;
	}
	else
	{
		wait_msg.emr = WAIT_IDLE;
	}

	return 0;
}

int set_wait_message_primed_state( void )
{
	if( wait_msg.emr != WAIT_IDLE )
	{
		return -1;
	}
	else
	{
		wait_msg.emr = WAIT_PRIMED;
	}

	return 0;
}

int set_wait_message_active_state( void )
{
	if( wait_msg.emr != WAIT_PRIMED )
	{
		return -1;
	}
	else
	{
		wait_msg.emr = WAIT_ACTIVE;
	}

	return 0;
}

int set_wait_message_status( int status )
{
	if( wait_msg.emr != WAIT_ACTIVE )
	{
		return -1;
	}
	else
	{
		wait_msg.completions_status = status;
	}

	return 0;
}

bool is_wait_messsage_idle_state( void )
{
	return wait_msg.emr == WAIT_IDLE;
}

bool is_wait_messsage_primed_state( void )
{
	return wait_msg.emr == WAIT_PRIMED;
}

bool is_wait_messsage_active_state( void )
{
	return wait_msg.emr == WAIT_ACTIVE;
}

