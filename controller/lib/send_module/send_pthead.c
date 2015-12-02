/*send_pthread.c
**Date:2015-11-5
**
**
*/

#include "send_pthread.h"

sem_t sem_waiting; // ���͵ȴ��ź����������߳̿ɼ�

void init_sem_wait_can( void )
{
	sem_init( &sem_waiting, 0, 0 );
}

#ifndef __NOT_USE_SEND_QUEUE_PTHREAD__ // ������send_pthread.h
int thread_send_func( void *pgm ) // ����ͬ�����ƣ������ź���.(�޸ĺ��ڴ��߳�ʹ��ͬ������2015-12-1)
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

		// ��ȡ��������
		p_sdpqueue_wnode p_send_wnode = NULL;
		bool is_resp_data = false;
		p_send_wnode = send_queue_message_get( p_send_wq );
		if( NULL == p_send_wnode )
		{
			DEBUG_INFO( "No send queue message: ERROR!" );
			pthread_mutex_unlock( &p_send_wq->control.mutex );
			continue;
		}

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

		int len = get_queue_length( &p_send_wq->work );
		DEBUG_INFO( "============>>after queue len = %d <<=============", len );
		pthread_mutex_unlock( &p_send_wq->control.mutex ); // unlock mutex
#if 1		
		/*������һ�����ݵ�����-���ݻ����Ӧ�����ݳ�ʱ��ʱ��������(ע:ʱ����ֻ������ϵͳ��Ӧ���ݻ�����ͷ�������ݵķ���)*/
		//DEBUG_INFO( "wait (%d) -----------interval(%d)",  is_wait_messsage_primed_state(), is_send_interval_primed_state());
		if( is_wait_messsage_primed_state() ) 
		{
			int status = -1;
			if( !is_resp_data )
			{
				DEBUG_INFO( "coming start of sending >>>host data<<<: (%d) ! ", is_wait_messsage_primed_state());
				status = set_wait_message_active_state();
				assert( status == 0 );
				sem_wait( &sem_waiting );
				status = set_wait_message_idle_state();
				assert( status == 0 );
			}
			else
			{
				DEBUG_INFO( "coming start of sending response data: is primed_state = %d ", is_send_interval_primed_state());
				status = set_wait_message_active_state();
				assert( status == 0 );
				uart_resp_send_interval_timer_start(); // start timer
				sem_wait( &sem_waiting );
				resp_send_interval_timer_stop();
				status = set_wait_message_idle_state();
				assert( status == 0 );
			}
		}
		else
		{
			DEBUG_INFO("...................................." );
		}
#endif
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
#endif

