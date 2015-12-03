#include "avdecc_funhdl_native.h"
#include "message_queue.h"
#include "send_work_queue.h"
#include "system_packet_tx.h"
#include "send_pthread.h"

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
				
#ifdef __PIPE_SEND_CONTROL_ENABLE__
				sem_post( &sem_tx );
#endif

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
						sem_wait( &sem_waiting );
						status = set_wait_message_idle_state();
						assert( status == 0 );
					}
					else
					{
						DEBUG_INFO( "coming start of sending response data: is primed_state = %d ", is_send_interval_primed_state());
						status = set_send_interval_wait_state();
						assert( status == 0 );
						uart_resp_send_interval_timer_start(); // start timer
						sem_wait( &sem_waiting );
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
来直接通过管道发送数据(见上thread_pipe_fn)，*/
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
			DEBUG_INFO( "frame len = %d ", tnt.frame_len );
			
			if( result > 0 )
			{
				// 加入网络数据发送队列
				DEBUG_INFO( "before lock" );
				pthread_mutex_lock( &send_wq->control.mutex );

				send_work_queue_message_save( &tnt, send_wq );
				int len = get_queue_length( &send_wq->work );
				DEBUG_INFO( "============>>save queue len = %d <<=============", len );

				pthread_mutex_unlock( &send_wq->control.mutex ); // unlock mutex
				DEBUG_INFO( "after lock" );
				pthread_cond_signal( &send_wq->control.cond );
				DEBUG_INFO( "after pthread_cond_signal" );
			}
			else 
			{
				assert( tnt.frame && (result >= 0) );
			}
			
#ifdef __PIPE_SEND_CONTROL_ENABLE__
			sem_post( &sem_tx );
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
#ifdef __TEST_QUEUE__ // test queue, other is in system.c, define in the circular_queue.c
		pthread_mutex_lock( &p_func_wq->control.mutex );

		while( p_func_wq->work.front == NULL && p_func_wq->control.active )
		{
			DEBUG_LINE();
			pthread_cond_wait( &p_func_wq->control.cond, &p_func_wq->control.mutex );
		}

		p_msg_wnode = func_command_work_queue_messag_get( p_func_wq );
		if( NULL == p_msg_wnode )
		{
			DEBUG_INFO( "func work queue no node!" );
			return -1;
		}

		uint16_t func_index = p_msg_wnode->job_data.func_msg_head.func_index;
		uint16_t func_cmd = p_msg_wnode->job_data.func_msg_head.func_cmd;
		DEBUG_ONINFO( "[ queue test:  %d(index)--%d(cmd) ]",  func_index, func_cmd );
		free( p_msg_wnode );

		pthread_mutex_unlock( &p_func_wq->control.mutex );
#else // normal 
		pthread_mutex_lock( &p_func_wq->control.mutex );
		
		while( p_func_wq->control.active && (p_func_wq->work.front == NULL) )
		{
			//DEBUG_INFO( "func_wq active = %d ", p_func_wq->control.active );
			pthread_cond_wait( &p_func_wq->control.cond, &p_func_wq->control.mutex );
		}

		if( !p_func_wq->control.active )
		{
			pthread_mutex_unlock( &p_func_wq->control.mutex );
			break;
		}
		
		p_msg_wnode = func_command_work_queue_messag_get( p_func_wq );
		if( NULL == p_msg_wnode )
		{
			DEBUG_INFO( "func work queue no node!" );
			pthread_mutex_unlock( &p_func_wq->control.mutex );
			continue;
		}

		pthread_mutex_unlock( &p_func_wq->control.mutex ); // unlock mutex

		// proccess func command queue message
		uint16_t func_index = p_msg_wnode->job_data.func_msg_head.func_index;
		uint16_t func_cmd = p_msg_wnode->job_data.func_msg_head.func_cmd;
		uint32_t data_len = p_msg_wnode->job_data.meet_msg.data_len;
		uint8_t *p_data = p_msg_wnode->job_data.meet_msg.data_buf;
		
		p_func_items[func_index].cmd_proccess( func_cmd, p_data, data_len );
		
		free( p_msg_wnode );
#endif	
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

