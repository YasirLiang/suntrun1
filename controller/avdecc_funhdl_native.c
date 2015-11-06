#include "avdecc_funhdl_native.h"
#include "message_queue.h"
#include "send_work_queue.h"

/* ��������ԭ���ķ��ͺ�����ֻ�Ǽ򵥵ķ������ݣ�û�п����ն˵Ĵ���������Ҫ���̡�
���ﲻͣ�ط�����û�����壬����޸�����:1�����뷢�Ͷ���(ʹ���߳�������������)��
2�������߳�ͬ��(ʹ���ź���)������ο��޸ĺ�ķ����̡߳������̲߳�ɾ����ԭ����
���߳̿�����Ϊ�������ݼ��뷢�Ͷ��е��̡߳�����ʹ�ã������޸��������ݵķ��ͽӿ�
(��system_packet_tx.c), ��Ҳ������ϵͳ�Ŀ������˺����޸����£��޸�ʱ��:2015-11-5
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
			//DEBUG_INFO( "RECV pipe data: [ frame_len = %d ]", tnt.frame_len );
			
			if( result > 0 )
			{
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
*/

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
			
			if( result > 0 )
			{
				// �����������ݷ��Ͷ���
				pthread_mutex_lock( &send_wq->control.mutex );
				
				send_work_queue_message_save( &tnt );
				
				pthread_mutex_unlock( &send_wq->control.mutex ); // unlock mutex
				pthread_cond_signal( &send_wq->control.cond ); // send pthread messag
			}
			else 
			{
				assert( tnt.frame && (result >= 0) );
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

int thread_func_fn( void * pgm )
{
	fcwqueue*  p_func_wq = &fcwork_queue;
	assert( pgm && p_func_wq );
	
	proccess_func_items *p_func_items = (proccess_func_items *)pgm;	
	fcqueue_wnode *p_msg_wnode = NULL;
	
	/* ��������� */
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
		
		while( p_func_wq->work.front == NULL && p_func_wq->control.active )
		{
			pthread_cond_wait( &p_func_wq->control.cond, &p_func_wq->control.mutex );
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


