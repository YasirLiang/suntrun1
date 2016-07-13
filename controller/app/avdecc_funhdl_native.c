#include "avdecc_funhdl_native.h"
#include "message_queue.h"
#include "send_work_queue.h"
#include "system_packet_tx.h"
#include "send_common.h"
#include "control_matrix_common.h"
#include "time_handle.h"
#include "avdecc_manage.h"// �����նˣ������������Ƴ��ն�
#include "send_common.h" // ����SEND_DOUBLE_QUEUE_EABLE
#include "camera_pro.h"
#include "system_1722_recv_handle.h"

#ifdef __NOT_USE_SEND_QUEUE_PTHREAD__ // ��send_pthead.h�ж���

/* ��������ԭ���ķ��ͺ�����ֻ�Ǽ򵥵ķ������ݣ�û�п����ն˵Ĵ���������Ҫ���̡�
���ﲻͣ�ط�����û�����壬����޸�����:1�����뷢�Ͷ���(ʹ���߳�������������)��
2�������߳�ͬ��(ʹ���ź���)������ο��޸ĺ�ķ����̡߳������̲߳�ɾ����ԭ����
���߳̿�����Ϊ�������ݼ��뷢�Ͷ��е��̡߳�����ʹ�ã������޸��������ݵķ��ͽӿ�
(��system_packet_tx.c), ��Ҳ������ϵͳ�Ŀ������˺����޸����£��޸�ʱ��:2015-11-5
����˵��(2015-12-1):�Դ˺������������޸�:����ź���ͬ�����ƣ�ʹ�ô˺�������δ���ֶ�ջ���󣬵�����һ
�������ǹܵ��ܹ�������������ȷ��Ͷ��еķ����������٣���Ҳ�Ǻ��ڴ������������һ����������*/
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
				
				/*������һ�����ݵ�����-���ݻ����Ӧ�����ݳ�ʱ��ʱ��������(ע:ʱ����ֻ������ϵͳ��Ӧ���ݻ�����ͷ�������ݵķ���)*/
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
/*2015-12-1ע:ʹ�ô��߳��뷢�Ͷ����̷߳�����(��ʹ�÷��Ͷ��еķ����������ݵĻ���)�������ڴ�����
�⵽�ƻ����³�����������ֱ������Ҳδ��������ʹ�ö����__NOT_USE_SEND_QUEUE_PTHREAD__
��ֱ��ͨ���ܵ���������(����thread_pipe_fn)��(2015-12-6 ����ʹ�ô��߳�) (2015-12-7 ����ʹ�ô��߳�, ������ȫ�޸���
֮ǰ���ڴ汼��������ԭ����ԭ���Է���ĶԿռ�û�н��л���ı���(���߳�֮��))*/
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
				// �����������ݷ��Ͷ���
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
					
					sem_post( &sem_tx ); // ����tnt.frame ָ���ͬ������
					continue;
				}

				if( frame_len > TRANSMIT_DATA_BUFFER_SIZE )
				{
					pthread_mutex_unlock( &send_wq->control.mutex ); // unlock mutex
					pthread_cond_signal( &send_wq->control.cond );
					
					sem_post( &sem_tx ); // ����tnt.frame ָ���ͬ������
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
			
				sem_post( &sem_tx ); // ����tnt.frame ָ���ͬ������, ����ʹ�ã�ԭ����д�ܵ����߳�����߳�ʹ����ͬһ���ڴ�
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
	
	/* ��������� */
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

#if 0// (2016-3-2)�����в�ͨ����Ϊ����������Ϊ��ʱ��ϵͳ������Ҳ�͵ò���ִ�� ���¾�����ʱ���ֵģ�Ҳ����ϵͳ����ֹͣͶƱ��ѡ�����ֵ�ԭ��
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
		system_1722_recv_handle_raw_data();// ��֡����1722���ݱ���
#else
#endif
		system_register_terminal_pro();
		terminal_sign_in_pro();// ע���ն�,�ڻ�ȡϵͳ��Ϣ�ɹ���ÿ��һ����ʱ��ע��һ��
		terminal_vote_proccess();// �ն�ͶƱ����
		terminal_query_sign_vote_pro();// ��ѯ�ն˵�ǩ����ͶƱ���
		avdecc_manage_discover_proccess();// ϵͳ��ʱ�����ն�
		send_common_check_squeue();// ��鷢�Ͷ��з�������
		camera_pro();// ����ͷ��������
		terminal_over_time_speak_pro();
		central_control_recieve_unit_event_pro();// ������յ�Ԫ�¼�����

		/*
		  *���ڽ������ݴ�����˵���ʾ����
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

