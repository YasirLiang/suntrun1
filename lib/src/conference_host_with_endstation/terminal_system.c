#include "terminal_system.h"

thost_sys_state gsystem_state;
thost_sys_state gsystem_state_pre;

thost_sys_state get_terminal_system_state( void )
{
	return gsystem_state;
}

bool set_terminal_system_state( uint8_t value )
{
	bool is_real_value = ((value & TMN_RGST_STATE) || (value & VOTE_STATE) ||(value & GRADE_STATE)\
		||(value & CAMERA_PRESET) ||(value & SIGN_STATE) ||(value & DISCUSS_STATE) \
		||(value & ELECT_STATE) ||(value & INTERPOSE_STATE)) ? true:false;
	
	if( is_real_value )
	{
		gsystem_state_pre = gsystem_state;
		gsystem_state.host_state = value;
		return true;
	}

	return false;
}

