#include "func_proccess.h"
#include "message_queue.h"
#include "data.h"

fcwqueue fcwork_queue;		// 函数命令消息工作队列

static uint16_t gpermit;  		// 当前系统的状态
static uint16_t gpermitpre;	// 上一个系统的状态
uint8_t globle_use_dis;

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

int func_command_find_and_run( proccess_func_items * func_tables, fcwqueue*  p_func_wq )
{
	assert( func_tables && p_func_wq );
	fcqueue_wnode *p_msg_wnode = NULL;
	
	while( p_func_wq->work.front == NULL && p_func_wq->control.active )
	{
		pthread_cond_wait( &p_func_wq->control.cond, &p_func_wq->control.mutex );
	}

	p_msg_wnode = func_command_work_queue_messag_get( p_func_wq );
	if( NULL == p_msg_wnode )
	{
		DEBUG_INFO( "func work queue no node!" );
		return -1;
	}

	// proccess func command queue message
	uint16_t func_index = p_msg_wnode->job_data.func_msg_head.func_index;
	uint16_t func_cmd = p_msg_wnode->job_data.func_msg_head.func_cmd;
	uint32_t data_len = p_msg_wnode->job_data.meet_msg.data_len;
	uint8_t *p_data = p_msg_wnode->job_data.meet_msg.data_buf;
	func_tables[func_index].cmd_proccess( func_cmd, p_data, data_len );
	free( p_msg_wnode );

	return 0;
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
		pthread_cond_signal( &fcwork_queue.control.cond);
		pthread_mutex_unlock( &fcwork_queue.control.mutex );
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

	return false; 
}

bool use_dis_set( uint8_t  user, bool set )
{
	if(!(user&(TERMINAL_USE|MENU_USE|SYSTEM_USE|COMPUTER_USE)))
	{
		return false;
	}

	if(set) // 确定user的设置
	{
		globle_use_dis |= user;
	}
	else // 	取消user的设置
	{
		globle_use_dis &= (~user);
	}
	
	return true;
}

uint16_t get_sys_state( void )
{
 	return gpermit;
}


