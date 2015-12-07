/*send_pthread.c
**Date:2015-11-5
**
**
*/

#include "send_pthread.h"
#include <time.h>

sem_t sem_waiting; // 发送等待信号量，所有线程可见

sdpwqueue net_send_queue;// 网络数据发送工作队列

void init_sem_wait_can( void )
{
	sem_init( &sem_waiting, 0, -1 );
}

void init_network_send_queue( void )
{
	bool is_su = false;
	
	is_su = controll_init( &net_send_queue.control );
	if( !is_su )
		DABORT( is_su );
	init_queue( &net_send_queue.work );
	
	is_su = controll_activate( &net_send_queue.control );
	if( !is_su );
		DABORT( is_su );
}

// 摧毁发送队列
void destroy_network_send_work_queue( void )
{
	//bool is_su = false;
	p_sdpqueue_wnode q_node = NULL;

	pthread_mutex_lock( &net_send_queue.control.mutex );
	
	while( !is_queue_empty( &net_send_queue.work ) ) // release node
	{
		q_node = send_queue_message_get( &net_send_queue );
		if( NULL != q_node )
		{
			free( q_node );
			q_node = NULL;	
		}
	}
	
	pthread_mutex_unlock( &net_send_queue.control.mutex );
	
	controll_deactivate( &net_send_queue.control );
	/*is_su = controll_destroy( &net_send_queue.control ); // 会被系统调用打断
	if( !is_su )
	{
		DABORT( is_su );
	}*/
}

int thread_send_func( void *pgm ) // 加入同步机制，采用信号量.(修改后不在此线程使用同步机制2015-12-1).(经验证201512-6此线程暂无问题)
{
	sdpwqueue*  p_send_wq = &net_send_queue;
	assert( p_send_wq );
	
	while( 1 )
	{
		pthread_mutex_lock( &p_send_wq->control.mutex ); // lock mutex
		while( p_send_wq->work.front == NULL && p_send_wq->control.active )
		{
			DEBUG_INFO( "active = %d", p_send_wq->control.active );
			pthread_cond_wait( &p_send_wq->control.cond, &p_send_wq->control.mutex );
		}

		// 获取队列数据
		p_sdpqueue_wnode p_send_wnode = NULL;
		bool is_resp_data = false;
		p_send_wnode = send_queue_message_get( p_send_wq );
		if( NULL == p_send_wnode )
		{
			DEBUG_INFO( "No send queue message: ERROR!" );
			pthread_mutex_unlock( &p_send_wq->control.mutex );
			continue;
		}

		int queue_len = get_queue_length( &p_send_wq->work );
		DEBUG_INFO( "after get queue len = %d ", queue_len );
		
		pthread_mutex_unlock( &p_send_wq->control.mutex ); // unlock mutex

		// ready to sending data
		is_resp_data = p_send_wnode->job_data.resp;
		tx_packet_event( p_send_wnode->job_data.data_type, 
							     p_send_wnode->job_data.notification_flag, 
							     p_send_wnode->job_data.frame, 
							     p_send_wnode->job_data.frame_len, 
							     &net_fd,// network fds
							     command_send_guard,
							     p_send_wnode->job_data.raw_dest.value,
							     &p_send_wnode->job_data.udp_sin,
							     is_resp_data );

		release_heap_space( &p_send_wnode->job_data.frame ); // free heap space mallo by write pipe thread
		assert( p_send_wnode->job_data.frame == NULL ); // free successfully and result is NULL? 
		if( NULL != p_send_wnode )
		{
			free( p_send_wnode );
			p_send_wnode = NULL;
		}
		
		/*发送下一条数据的条件-数据获得响应或数据超时或时间间隔到了(注:时间间隔只适用于系统响应数据或摄像头控制数据的发送)*/
		if( is_wait_messsage_primed_state() ) 
		{
			int status = -1;
			struct timespec timeout;
			int ret = 0;
			if ( clock_gettime( CLOCK_REALTIME, &timeout ) == -1)
			{
				 perror("clock_gettime:");
				 return -1;
			}
			
			timeout.tv_sec += 4;	// timeouts is 4 seconds
			
			if( !is_resp_data )
			{
				status = set_wait_message_active_state();
				assert( status == 0 );
				//sem_wait( &sem_waiting );
				ret = sem_timedwait( &sem_waiting, &timeout );
				if( ret == -1 )
				{
					if( errno == ETIMEDOUT )
					{
						DEBUG_INFO( "sem_timedwait(): time out!send pthread proccess continue" );
						sem_post( &sem_waiting );
					}
					else
					{
						perror( "sem_timedwait():" );
					}
				}
				status = set_wait_message_idle_state();
				assert( status == 0 );
			}
			else
			{
				status = set_wait_message_active_state();
				assert( status == 0 );
				uart_resp_send_interval_timer_start(); // start timer
				//sem_wait( &sem_waiting );
				ret = sem_timedwait( &sem_waiting, &timeout );
				if( ret == -1 )
				{
					if( errno == ETIMEDOUT )
					{
						DEBUG_INFO( "sem_timedwait(): time out! send pthread proccess continue" );
						sem_post( &sem_waiting );
					}
					else
					{
						perror( "sem_timedwait():" );
					}
				}
				
				resp_send_interval_timer_stop();
				status = set_wait_message_idle_state();
				assert( status == 0 );
			}
		}
		else
		{
			DEBUG_INFO(" not message primed success!" );
		}
	}
	
	return 0;
}

int pthread_send_network_create( pthread_t *send_pid )
{
	assert( send_pid );
	int rc = 0;

	rc = pthread_create( send_pid, NULL, (void*)&thread_send_func, NULL );
	if( rc )
	{
		DEBUG_INFO(" pthread_send_network_create ERROR; return code from pthread_create() is %d\n", rc);
		assert( rc == 0 );
	}

	return 0;
}


