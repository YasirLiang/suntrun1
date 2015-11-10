#include "terminal_system.h"

thost_sys_state gsystem_state;
thost_sys_state gsystem_state_pre;

thost_sys_state get_terminal_system_state( void )
{
	return gsystem_state;
}

bool set_terminal_system_state( uint8_t state_value, bool is_pre )
{
	bool is_real_value = ((state_value & TMN_RGST_STATE) || (state_value & VOTE_STATE) ||(state_value & GRADE_STATE)\
		||(state_value & CAMERA_PRESET) ||(state_value & SIGN_STATE) ||(state_value & DISCUSS_STATE) \
		||(state_value & ELECT_STATE) ||(state_value & INTERPOSE_STATE)) ? true:false;
	bool ret = false;

	if( is_pre )
	{
		if( is_real_value )
		{
			gsystem_state_pre = gsystem_state;
			gsystem_state.host_state = state_value;
			ret = true;
		}
	}
	else if( state_value == gsystem_state.host_state ) // 回到上一个系统的状态
	{
		gsystem_state = gsystem_state_pre;
		ret = true;
	}
	
	return ret;
}

