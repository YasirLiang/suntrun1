#include "avdecc_funhdl_native.h"
#include "message_queue.h"
#include "send_work_queue.h"
#include "system_packet_tx.h"
#include "send_common.h"
#include "control_matrix_common.h"
#include "time_handle.h"
#include "avdecc_manage.h"// 发现终端，读描述符，移除终端
#include "send_common.h" // 包含SEND_DOUBLE_QUEUE_EABLE
#include "camera_pro.h"
#include "system_1722_recv_handle.h"
#include "terminal_pro.h"
#include "muticast_connect_manager.h"
#include "global.h"

int thread_func_fn( void * pgm )
{
	fcwqueue*  p_func_wq = &fcwork_queue;
	assert( pgm && p_func_wq );
	
	proccess_func_items *p_func_items = (proccess_func_items *)pgm;	
	fcqueue_wnode *p_msg_wnode = NULL;
	
	/* 处理命令函数 */
	while( 1 )
	{
		pthread_mutex_lock( &p_func_wq->control.mutex );
		
		while( p_func_wq->control.active && (p_func_wq->work.head == NULL) )
		{
			pthread_cond_wait( &p_func_wq->control.cond, &p_func_wq->control.mutex );
			DEBUG_INFO( "active = %d", p_func_wq->control.active );
		}

		if( !p_func_wq->control.active )
		{
			pthread_mutex_unlock( &p_func_wq->control.mutex );
			break;
		}

		p_msg_wnode = func_command_work_queue_messag_get( p_func_wq );
		if( p_func_wq->work.head == NULL )
		{
			if( p_func_wq->work.trail != NULL ) 
			{// while queue isempty,make sure the queue trail not to be used again!!!!
				p_func_wq->work.trail = NULL;
			}
		}
		if( NULL == p_msg_wnode )
		{
			DEBUG_INFO( "func work queue no node!" );
			pthread_mutex_unlock( &p_func_wq->control.mutex );
			continue;
		}
		
		// save func command queue message
		uint16_t func_index = p_msg_wnode->job_data.func_msg_head.func_index;
		uint16_t func_cmd = p_msg_wnode->job_data.func_msg_head.func_cmd;
		uint32_t data_len = p_msg_wnode->job_data.meet_msg.data_len;
		uint8_t func_data[SUB_DATA_TYPE_SIZE] = {0};
		if( data_len > SUB_DATA_TYPE_SIZE )
		{
			if( NULL != p_msg_wnode )
			{
				free( p_msg_wnode );
				p_msg_wnode = NULL;
			}
		
			pthread_mutex_unlock( &p_func_wq->control.mutex ); // unlock mutex
			continue;
		}
		
		memcpy( func_data, p_msg_wnode->job_data.meet_msg.data_buf, data_len );
		if( NULL != p_msg_wnode )
		{
			free( p_msg_wnode );
			p_msg_wnode = NULL;
		}
		
		pthread_mutex_unlock( &p_func_wq->control.mutex ); // unlock mutex

		DEBUG_INFO( "Run func comand index = %d", func_index );
		p_func_items[func_index].cmd_proccess( func_cmd, func_data, data_len );// run command
	}
	
	return 0;
}

int pthread_handle_cmd_func( pthread_t *pid, const proccess_func_items *p_items )
{
	assert( pid );
	int rc = 0;

	rc = pthread_create( pid, NULL, (void*)&thread_func_fn, (void*)p_items );
	if( rc )
	{
		DEBUG_INFO(" pthread_handle_cmd_func ERROR; return code from pthread_create() is %d\n", rc);
		assert( rc == 0 );
	}

	return 0;
}

#define INPUT_MSG_LEN	6
extern void input_recv_pro(unsigned char *p_buf, unsigned recv_len);
extern unsigned char gcontrol_sur_recv_buf[INPUT_MSG_LEN]; 
extern volatile unsigned char gcontrol_sur_msg_len; 
#define SYS_BUF_RECV_COUNT 2
int pthread_recv_data_fn( void *pgm )
{
	static int static_buf_num = 0;

	while( 1 )
	{
		unsigned long us_per_ms = 1000;
        	unsigned long interval_ms = 0/*TIME_PERIOD_1_MILLISECONDS*/;
		struct timeval tempval;

		tempval.tv_sec = interval_ms/1000;  
	        tempval.tv_usec = (interval_ms%1000)*us_per_ms;
	        select( 0, NULL, NULL, NULL, &tempval );

		if( static_buf_num >= SYS_BUF_RECV_COUNT )
			static_buf_num = 0;
		switch( (static_buf_num++) % SYS_BUF_RECV_COUNT )
		{
			case 0:// buffer 1
				if (gregister_tmnl_pro.rgs_state == RGST_IDLE)
				{
					pthread_mutex_lock(&ginflight_pro.mutex);
					upper_computer_recv_message_get_pro();
					pthread_mutex_unlock(&ginflight_pro.mutex);
				}
			break;
			case 1:// buffer 2
				control_matrix_common_recv_message_pro();
			break;
			default:
				break;
		}
        
		system_register_terminal_pro();
		if (gregister_tmnl_pro.rgs_state == RGST_IDLE)
		{
			terminal_sign_in_pro();// 注册终端,在获取系统信息成功后，每隔一定的时间注册一个
			terminal_vote_proccess();// 终端投票处理
			terminal_query_sign_vote_pro();// 查询终端的签到与投票结果

#ifdef __ARM_BACK_TRACE__
                        /*
			  *串口接收数据处理与菜单显示处理
			  */
			if( gcontrol_sur_msg_len > 0 )
			{
				input_recv_pro( gcontrol_sur_recv_buf, gcontrol_sur_msg_len );
				gcontrol_sur_msg_len = 0;
			}
#endif

			muticast_connect_manger_timeout_event_image();
		}
		
		avdecc_manage_discover_proccess();// 系统定时发现终端
		send_common_check_squeue();// 检查发送队列发送数据
		camera_pro();// 摄像头处理流程
		terminal_over_time_speak_pro();//
		terminal_after_time_mic_state_pro();
                Terminal_micCallbackPro();
	}
	
	return 0;
}

int pthread_proccess_recv_data_create( pthread_t *pid, void * pgm )
{
	assert( pid );
	int rc = 0;

	rc = pthread_create( pid, NULL, (void*)&pthread_recv_data_fn, NULL );
	if( rc )
	{
		DEBUG_INFO(" pthread_proccess_recv_data_create ERROR; return code from pthread_create() is %d\n", rc);
		assert( rc == 0 );
	}

	return 0;
}

