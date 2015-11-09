/**
*wait_messag.h
*date:2015-11-5
*
*/

#ifndef __WAIT_MESSAGE_H__
#define __WAIT_MESSAGE_H__

#include "jdksavdecc_world.h"

enum wait_message
{
	WAIT_IDLE,
	WAIT_PRIMED,
	WAIT_ACTIVE
};

enum
{
	WAIT_TIMEOUT = 1
};

struct wait_send_massge
{
	enum wait_message emr;
	int completions_status; // 完成状态 0 表示正常
	uint32_t notification_id;
};

void init_wait_message( void );
int set_wait_message_idle_state( void );
int set_wait_message_primed_state( void );
int set_wait_message_active_state( void );
int set_wait_message_status( int status );
bool is_wait_messsage_idle_state( void );
bool is_wait_messsage_primed_state( void );
bool is_wait_messsage_active_state( void );


#endif
