#include "func_proccess.h"
#include "message_queue.h"
#include "data.h"
#include "terminal_system.h"

fcwqueue fcwork_queue;						// 函数命令消息工作队列

uint8_t globle_use_dis = TERMINAL_USE |COMPUTER_USE |MENU_USE; 	// 当前用户

void init_func_command_work_queue( void )
{
	bool is_su = false;
	
	is_su = controll_init( &fcwork_queue.control );
	if( !is_su )
		DABORT( is_su );
	init_queue( &fcwork_queue.work );
	
	is_su = controll_activate( &fcwork_queue.control );
	if( !is_su );
		DABORT( is_su );
}

/*************************************
**
**
**
**
**功能:从函数命令处理命令列表获取命令信息，
**		并存入循环队列fcwork_queue后通知
**		命令处理线程
**************************************/
bool find_func_command_link( uint8_t user, uint16_t cfc_cmd, uint16_t func_cmd, const uint8_t *pdata, const uint16_t data_len )
{
	int i = 0;
	enum_func_link func_link = 0xffff;
  	uint16_t func_cmd_pre = 0xFFFF;
  	uint16_t index=0;
  	fcqueue_data_elem queue_data_elem;
  	
	if( !fcwork_queue.control.active )
		return false;
	
	if( (data_len + sizeof(func_message_head)) > MAX_FUNC_MSG_LEN)
	{
#ifdef __DEBUG__
		DEBUG_INFO( "func message data len is max!" );
		assert( (data_len + sizeof(func_message_head)) <= MAX_FUNC_MSG_LEN);
#else
		return false;
#endif
	}

	if( !(globle_use_dis & user))
	{
#ifdef __DEBUG__
		DEBUG_INFO( "globle_use_dis init err!" );
		assert( globle_use_dis & user );
#else
		return false;
#endif
	}
		
	for( i = 0; i < MAX_FUNC_LINK_ITEMS; i++ )
	{
		uint8_t user_tmp = func_link_tables[i].user;
		uint16_t cmd_tmp = func_link_tables[i].cmd;
		if( ( user_tmp == user )&& ( cmd_tmp == cfc_cmd ))
		{
			func_link = func_link_tables[i].func_cmd_link;
			func_cmd_pre = func_link_tables[i].func_cmd;
			break;
		}
	}

	if( i >= MAX_FUNC_LINK_ITEMS )
	{
#ifdef __DEBUG__
		DEBUG_INFO( " func command link and cmd not found: check user or cfc_cmd" );
		return false;
#else
		return false;
#endif
	}

	index = MAX_PROCCESS_FUNC;
	for( i = 0; i < MAX_PROCCESS_FUNC; i++)
	{
		if( proccess_func_link_tables[i].func_cmd_link == func_link)
		{
			index = i;
			break;
		}
	}

	// save message
	//DEBUG_INFO( " pro func index = %d/%d; system state = %02x", index, i,  get_sys_state() );
	if( (index <  MAX_PROCCESS_FUNC) && (proccess_func_link_tables[i].permit & get_sys_state()))
	{
		queue_data_elem.func_msg_head.func_index = index;
		if( func_cmd_pre != 0 )
		{
			queue_data_elem.func_msg_head.func_cmd = func_cmd_pre;
		}
		else
		{
			queue_data_elem.func_msg_head.func_cmd = func_cmd;
		}

		queue_data_elem.meet_msg.data_len = data_len;
		memcpy( queue_data_elem.meet_msg.data_buf, pdata, data_len );

		// thread lock and save data
		pthread_mutex_lock( &fcwork_queue.control.mutex );
		
		func_command_work_queue_messag_save( &queue_data_elem, &fcwork_queue );
		
		pthread_mutex_unlock( &fcwork_queue.control.mutex );
		pthread_cond_signal( &fcwork_queue.control.cond);
	}
	else
	{
#ifdef __DEBUG__
		DEBUG_INFO( "index proccess func err or sys state not permit!" );
		return false;
#else
		return false;
#endif
	}

	return true; 
}

bool use_dis_set( uint8_t  user, bool set )
{
	if(!(user&(TERMINAL_USE|MENU_USE|SYSTEM_USE|COMPUTER_USE)))
	{
		return false;
	}

	if(set)
	{
		globle_use_dis |= user;
	}
	else 
	{
		globle_use_dis &= (~user);
	}
	
	return true;
}

