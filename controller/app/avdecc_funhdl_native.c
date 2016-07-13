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

#ifdef __NOT_USE_SEND_QUEUE_PTHREAD__ // 在send_pthead.h中定义

/* 这是网络原来的发送函数，只是简单的发送数据，没有考虑终端的处理数据需要过程。
这里不停地发数据没有意义，因此修改如下:1、加入发送队列(使用线程锁与条件变量)。
2、加入线程同步(使用信号量)。具体参考修改后的发送线程。而这线程不删除，原因是
此线程可以作为网络数据加入发送队列的线程。即可使用，不用修改网络数据的发送接口
(见system_packet_tx.c), 但也增加了系统的开销。此函数修改如下；修改时间:2015-11-5
新增说明(2015-12-1):对此函数作了如下修改:添加信号量同步机制，使用此函数发送未发现堆栈错误，但是有一
个问题是管道能够处理的数据量比发送队列的方案数据量少，这也是后期大数据量处理的一个考虑问题*/
int thread_pipe_fn( void *pgm ) 
{
	struct fds *kfds = ( struct fds* )pgm;
	struct fds thr_fds;
	memcpy( &thr_fds, kfds, sizeof(struct fds));

	while(1)
	{
		if( check_pipe_read_ready( thr_fds.tx_pipe[PIPE_RD]) )
		{
			tx_data tnt;
			int result = read_pipe_tx( &tnt, sizeof(tx_data) );
			DEBUG_INFO( "RECV pipe data: [ frame_len = %d ]", tnt.frame_len );
			
			if( result > 0 )
			{
				bool is_resp_data = tnt.resp;
				tx_packet_event( tnt.data_type, 
							     tnt.notification_flag, 
							     tnt.frame, 
							     tnt.frame_len, 
							     &thr_fds,
							     command_send_guard,
							     tnt.raw_dest.value,
							     &tnt.udp_sin,
							     tnt.resp );
				release_heap_space( &tnt.frame ); // free heap space mallo by write pipe thread
				assert( tnt.frame == NULL );		// free successfully and result is NULL? 
				
				/*发送下一条数据的条件-数据获得响应或数据超时或时间间隔到了(注:时间间隔只适用于系统响应数据或摄像头控制数据的发送)*/
				if( (!is_resp_data && is_wait_messsage_primed_state()) || ( is_resp_data && is_send_interval_primed_state())) 
				{
					int status = -1;
					if( !is_resp_data )
					{
						DEBUG_INFO( "coming start of sending >>>host data<<<: (%d) ! ", is_wait_messsage_primed_state());
						status = set_wait_message_active_state();
						assert( status == 0 );
						uart_resp_send_interval_timer_start(); // start timer
#ifndef SEND_DOUBLE_QUEUE_EABLE
						sem_wait( &sem_waiting );
#endif
						status = set_wait_message_idle_state();
						assert( status == 0 );
					}
					else
					{
						DEBUG_INFO( "coming start of sending response data: is primed_state = %d ", is_send_interval_primed_state());
						status = set_send_interval_wait_state();
						assert( status == 0 );
						uart_resp_send_interval_timer_start(); // start timer
#ifndef SEND_DOUBLE_QUEUE_EABLE
						sem_wait( &sem_waiting );
#endif
						status = set_send_interval_idle_state();
						assert( status == 0 );
						resp_send_interval_timer_stop();
					}

					resp_send_interval_timer_stop();
				}
			}
			else 
			{
				assert( tnt.frame && result >= 0 );
			}
		}
		else
		{
			DEBUG_INFO( "read pipe is not ready!" );
			continue;
		}
	}

	return 0;
}
#else 
/*2015-12-1注:使用此线程与发送队列线程发现了(即使用发送队列的发送网络数据的机制)发现了内存数据
遭到破坏导致程序奔溃的现象，直到今天也未解决，因此使用定义宏__NOT_USE_SEND_QUEUE_PTHREAD__
来直接通过管道发送数据(见上thread_pipe_fn)，(2015-12-6 重新使用此线程) (2015-12-7 重新使用此线程, 并且完全修复了
之前堆内存奔溃的现象，原因是原来对分配的对空间没有进行互斥的保护(在线程之间))*/
int thread_pipe_fn( void *pgm )
{
	struct fds *kfds = ( struct fds* )pgm;
	struct fds thr_fds;
	sdpwqueue*  send_wq = &net_send_queue;
	assert( send_wq );

	memcpy( &thr_fds, kfds, sizeof(struct fds));

	while(1)
	{
		if( check_pipe_read_ready( thr_fds.tx_pipe[PIPE_RD]) )
		{
			tx_data tnt;
			int result = read_pipe_tx( &tnt, sizeof(tx_data) );
#if 0			
			if( result > 0 )
			{
				// 加入网络数据发送队列
				uint8_t* frame_buf = NULL;
				uint16_t frame_len = tnt.frame_len;
				
				pthread_mutex_lock( &send_wq->control.mutex );

				// heap using later free by sending thread.frame_buf space must to be free!
				frame_buf = allot_heap_space( TRANSMIT_DATA_BUFFER_SIZE, &frame_buf );
				if( NULL == frame_buf )
				{
					DEBUG_INFO( "system_raw_queue_tx Err: allot space for frame failed!" );
					pthread_mutex_unlock( &send_wq->control.mutex ); // unlock mutex
					pthread_cond_signal( &send_wq->control.cond );
					
					sem_post( &sem_tx ); // 用于tnt.frame 指针的同步操作
					continue;
				}

				if( frame_len > TRANSMIT_DATA_BUFFER_SIZE )
				{
					pthread_mutex_unlock( &send_wq->control.mutex ); // unlock mutex
					pthread_cond_signal( &send_wq->control.cond );
					
					sem_post( &sem_tx ); // 用于tnt.frame 指针的同步操作
					continue;
				}

				memset( frame_buf, 0, TRANSMIT_DATA_BUFFER_SIZE );
				memcpy( frame_buf, tnt.frame, tnt.frame_len );
				tnt.frame = frame_buf;	//  change the tnt frame buf to heap space
				send_work_queue_message_save( &tnt, send_wq );
				
				int queue_len = get_queue_length( &send_wq->work );
				DEBUG_INFO( "save queue len = %d ", queue_len );

				pthread_mutex_unlock( &send_wq->control.mutex ); // unlock mutex
				pthread_cond_signal( &send_wq->control.cond );
			
				sem_post( &sem_tx ); // 用于tnt.frame 指针的同步操作, 必须使用，原因是写管道的线程与此线程使用了同一块内存
			}
			else 
			{
				assert( tnt.frame && (result >= 0) );
			}

#else
#if 0
		if( result > 0 )
		{
			
			uint32_t resp_interval_time = 0;
			tx_packet_event( tnt.data_type, 
								     tnt.notification_flag, 
								     tnt.frame, 
								     tnt.frame_len, 
								     &net_fd,// network fds
								     command_send_guard,
								     tnt.raw_dest,
								     &tnt.udp_sin,
								     tnt.resp,
								     &resp_interval_time );
			sem_post( &sem_tx );			
		}
		else 
		{
			sem_post( &sem_tx );
			assert( tnt.frame && (result >= 0) );
		}
#else		
		if( result > 0 )
		{			
			system_packet_save_send_queue( tnt );
			sem_post( &sem_tx );
		}
		else 
		{
			sem_post( &sem_tx );
			assert( tnt.frame && (result >= 0) );
		}
#endif

#endif
		}
		else
		{
			DEBUG_INFO( "read pipe is not ready!" );
			continue;
		}
	}

	return 0;
}
#endif

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

#if 0// (2016-3-2)这样行不通，因为当队列永不为空时，系统的命令也就得不到执行 最新经调试时发现的，也就是系统不能停止投票与选举评分的原因
		pthread_mutex_lock( &net_send_queue.control.mutex );
		if( !is_queue_empty( &net_send_queue.work ) )
		{
			pthread_mutex_unlock( &p_func_wq->control.mutex ); // unlock mutex
			pthread_mutex_unlock( &net_send_queue.control.mutex ); // unlock mutex
			continue;
		}
		pthread_mutex_unlock( &net_send_queue.control.mutex );
#endif		
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

int pthread_handle_pipe( pthread_t *p_trd, struct fds *kfds )
{
	int rc = 0;
	
	rc = pthread_create( p_trd, NULL, ( void* )&thread_pipe_fn, kfds );
	if ( rc )
	{
		DEBUG_INFO(" Pthread_Handle_Pipe ERROR; return code from pthread_create() is %d\n", rc);
		assert( rc == 0 );
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
				pthread_mutex_lock(&ginflight_pro.mutex);
				upper_computer_recv_message_get_pro();
				pthread_mutex_unlock(&ginflight_pro.mutex);
			break;
			case 1:// buffer 2
				control_matrix_common_recv_message_pro();
			break;
			default:
				break;
		}

#ifdef RECV_DOUBLE_LINK_QUEUE_EN
		system_1722_recv_handle_raw_data();// 按帧处理1722数据报文
#else
#endif
		system_register_terminal_pro();
		terminal_sign_in_pro();// 注册终端,在获取系统信息成功后，每隔一定的时间注册一个
		terminal_vote_proccess();// 终端投票处理
		terminal_query_sign_vote_pro();// 查询终端的签到与投票结果
		avdecc_manage_discover_proccess();// 系统定时发现终端
		send_common_check_squeue();// 检查发送队列发送数据
		camera_pro();// 摄像头处理流程
		terminal_over_time_speak_pro();
		central_control_recieve_unit_event_pro();// 中央接收单元事件处理

		/*
		  *串口接收数据处理与菜单显示处理
		  */
		if( gcontrol_sur_msg_len > 0 )
		{
			input_recv_pro( gcontrol_sur_recv_buf, gcontrol_sur_msg_len );
			gcontrol_sur_msg_len = 0;
		}
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

