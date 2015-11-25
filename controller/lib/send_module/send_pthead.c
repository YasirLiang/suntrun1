/*send_pthread.c
**Date:2015-11-5
**
**
*/

#include "send_pthread.h"

sem_t sem_waiting; // 发送等待信号量，所有线程可见

void init_sem_wait_can( void )
{
	sem_init( &sem_waiting, 0, 0 );
}

int thread_send_func( void *pgm ) // 加入同步机制，采用信号量
{
	sdpwqueue*  p_send_wq = &net_send_queue;
	assert( p_send_wq );

	p_sdpqueue_wnode p_send_wnode = NULL;

	while( 1 )
	{
		bool is_resp_data = false;
		pthread_mutex_lock( &p_send_wq->control.mutex );

		while( p_send_wq->work.front == NULL && p_send_wq->control.active )
		{
			DEBUG_INFO( "active = %d", p_send_wq->control.active );
			pthread_cond_wait( &p_send_wq->control.cond, &p_send_wq->control.mutex );
		}

		// 获取队列数据
		p_send_wnode = send_queue_message_get( p_send_wq );
		if( NULL == p_send_wnode )
		{
			DEBUG_INFO( "No send queue message: ERROR!" );
			pthread_mutex_unlock( &p_send_wq->control.mutex );
			continue;
		}
		
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
		assert( p_send_wnode->job_data.frame == NULL );		// free successfully and result is NULL? 
		free( p_send_wnode );
		p_send_wnode = NULL;

		/*发送下一条数据的条件-数据获得响应或数据超时或时间间隔到了(注:时间间隔只适用于系统响应数据或摄像头控制数据的发送)*/
		if( (!is_resp_data && is_wait_messsage_primed_state()) || ( is_resp_data && is_send_interval_primed_state())) 
		{
			int status = -1;
			if( !is_resp_data )
			{
				status = set_wait_message_active_state();
				assert( status == 0 );
				sem_wait( &sem_waiting );
				status = set_wait_message_idle_state();
				assert( status == 0 );
			}
			else
			{
				DEBUG_INFO( "coming start of sending response data: is primed_state = %d ", is_send_interval_primed_state());
				status = set_send_interval_wait_state();
				assert( status == 0 );
				sem_wait( &sem_waiting );
				status = set_send_interval_idle_state();
				assert( status == 0 );
			}
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

