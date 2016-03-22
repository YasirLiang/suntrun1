#include "terminal_system.h"
#include "terminal_pro.h"

thost_sys_state gsystem_state;
thost_sys_state gsystem_state_pre;

void init_terminal_system_state( void )
{
	gsystem_state.host_state = TMN_RGST_STATE;
}

thost_sys_state get_terminal_system_state( void )
{
 	return gsystem_state;
}

bool set_terminal_system_state( uint8_t state_value, bool is_pre )
{	
	bool ret = false;

	if( is_pre )
	{
		if( state_value == CAMERA_PRESET )
		{
			if( gsystem_state.host_state == DISCUSS_STATE ) // 只有讨论时才可切换为CAMERA_PRESET
			{
				gsystem_state_pre.host_state = gsystem_state.host_state;
				gsystem_state.host_state = state_value;
			}

			return true;
		}

		if( (gsystem_state.host_state == CAMERA_PRESET) \
			|| (gsystem_state.host_state == INTERPOSE_STATE) )
		{
			gsystem_state_pre.host_state = state_value;
			ret = true;	
		}
		else
		{
			gsystem_state_pre.host_state = gsystem_state.host_state;
			gsystem_state.host_state = state_value;
			ret = false;
		}
	}
	else if( state_value == gsystem_state.host_state ) // 回到上一个系统的状态
	{
		if( gsystem_state.host_state == CAMERA_PRESET )
		{
			terminal_start_discuss( true );// 不必关闭mic
		}
		
		gsystem_state.host_state = gsystem_state_pre.host_state;
		ret = true;
	}
	
	return ret;
}

uint8_t get_sys_state( void )
{
 	return gsystem_state.host_state;
}

