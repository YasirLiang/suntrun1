#include "terminal_system.h"

thost_sys_state gsystem_state;
thost_sys_state gsystem_state_pre;

thost_sys_state get_terminal_system_state( void )
{
	return gsystem_state;
}

bool set_terminal_system_state( uint8_t value )
{
	if( value & 0xff )
	{
		gsystem_state_pre = gsystem_state;
		gsystem_state.host_state = value;
		return true;
	}

	return false;
}

