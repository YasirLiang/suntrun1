/*send_pthread.c
**Date:2015-11-5
**
**
*/

#include "send_pthread.h"
#include <time.h>

sem_t sem_waiting; // 发送等待信号量，所有线程可见
sdpwqueue net_send_queue;// 网络数据发送工作队列
static uint8_t send_frame[TRANSMIT_DATA_BUFFER_SIZE] = {0};// 本地发送缓冲区

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
	/*is_su = controll_destroy( &net_send_queue.control ); // 会被信号打断
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
		uint8_t dest_raw[6] = {0};
		struct sockaddr_in udp_sin;
		uint32_t resp_interval_time = 0;
		p_sdpqueue_wnode p_send_wnode = NULL;
		
		pthread_mutex_lock( &p_send_wq->control.mutex ); // lock mutex
		while( p_send_wq->work.head == NULL && p_send_wq->control.active )
		{
			DEBUG_INFO( "active = %d", p_send_wq->control.active );
			pthread_cond_wait( &p_send_wq->control.cond, &p_send_wq->control.mutex );
		}

		// 获取队列数据
		p_send_wnode = send_queue_message_get( p_send_wq );
		if( p_send_wq->work.head == NULL )
		{ // while queue isempty,make sure the queue trail not to be used again!!!!  
			if( p_send_wq->work.trail != NULL )
			{
				p_send_wq->work.trail = NULL;
			}
		}
		
		if( NULL == p_send_wnode )
		{
			DEBUG_INFO( "No send queue message: ERROR!" );
			pthread_mutex_unlock( &p_send_wq->control.mutex );
			continue;
		}

		uint16_t send_frame_len = p_send_wnode->job_data.frame_len;
		uint8_t data_type = p_send_wnode->job_data.data_type;
		bool notification_flag = p_send_wnode->job_data.notification_flag;
		bool is_resp_data = p_send_wnode->job_data.resp;
		if( send_frame_len > TRANSMIT_DATA_BUFFER_SIZE )
		{
			if( p_send_wnode->job_data.frame != NULL )
			{
				free( p_send_wnode->job_data.frame );
				p_send_wnode->job_data.frame = NULL;
			}
			
			if( NULL != p_send_wnode )
			{
				free( p_send_wnode ); // 释放队列节点
				p_send_wnode = NULL;
			}

			pthread_mutex_unlock( &p_send_wq->control.mutex ); // unlock mutexpthread_mutex_unlock( &p_send_wq->control.mutex ); // unlock mutex
			continue;
		}

		memcpy( send_frame, p_send_wnode->job_data.frame, send_frame_len );
		memcpy( dest_raw, p_send_wnode->job_data.raw_dest, 6 );
		memcpy( &udp_sin, &p_send_wnode->job_data.udp_sin, sizeof(struct sockaddr_in));
		if( p_send_wnode->job_data.frame != NULL )
		{
			free( p_send_wnode->job_data.frame );
			p_send_wnode->job_data.frame = NULL;
		}
		
		if( NULL != p_send_wnode )
		{
			free( p_send_wnode ); // 释放队列节点
			p_send_wnode = NULL;
		}

		pthread_mutex_unlock( &p_send_wq->control.mutex ); // unlock mutexpthread_mutex_unlock( &p_send_wq->control.mutex ); // unlock mutex

		// ready to sending data
		tx_packet_event( data_type, 
					    notification_flag, 
					    send_frame, 
					    send_frame_len, 
					    &net_fd, 
					    command_send_guard, 
					    dest_raw, 
					    &udp_sin, 
					    is_resp_data, 
					    &resp_interval_time );
		
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
#if 1
				sem_wait( &sem_waiting );
#else
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
#endif
				status = set_wait_message_idle_state();
				assert( status == 0 );
			}
			else
			{
				status = set_wait_message_active_state();
				assert( status == 0 );
				if( resp_interval_time > 0 )
				{
					resp_send_interval_timer_start( resp_interval_time ); // start timer useful as all response data
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
				}

				status = set_wait_message_idle_state();
				assert( status == 0 );
			}

			//DEBUG_INFO( "send queue node END!<<<<<<<<<<<<<<<<<<<<<<<<<" );
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


