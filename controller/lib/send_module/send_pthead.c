/*send_pthread.c
**Date:2015-11-5
**
**
*/

#include "send_pthread.h"
#include <time.h>
#include "time_handle.h"
#include "send_common.h"

#define SEND_INTERVAL_TIMEOUT 1 // ���ͼ��ms

static uint8_t send_frame[TRANSMIT_DATA_BUFFER_SIZE] = {0};// ���ط��ͻ�����

int thread_send_func( void *pgm ) // ����ͬ�����ƣ������ź���.(�޸ĺ��ڴ��߳�ʹ��ͬ������2015-12-1).(����֤201512-6���߳���������)
{
	sdpwqueue*  p_send_wq = &net_send_queue;
	assert( p_send_wq );

	// ���÷��ͼ��
	over_time_set( SYSTEM_SQUEUE_SEND_INTERVAL, SEND_INTERVAL_TIMEOUT );
	
	while( 1 )
	{
		if( !over_time_listen(SYSTEM_SQUEUE_SEND_INTERVAL) )
		{
			continue;
		}
		
#ifdef SEND_DOUBLE_QUEUE_EABLE// ˫���з����̺߳�����������
		bool write_empty = is_queue_empty( &gwrite_send_queue.work );
		bool read_empty = is_queue_empty( &p_send_wq->work );

		/**
		  *������Ϊ�գ�д��Ϊ�գ�������д����
		  */ 
		if( read_empty && !write_empty )
		{
			pthread_mutex_lock( &p_send_wq->control.mutex );
			swap_sdpqueue( p_send_wq, &gwrite_send_queue );
			pthread_mutex_unlock( &p_send_wq->control.mutex );
		}

		/**
		  *��д���ж�Ϊ�գ��߳�˯��
		  */ 
		if( read_empty && write_empty )
		{
			pthread_mutex_lock( &p_send_wq->control.mutex );
			pthread_cond_wait( &p_send_wq->control.cond, &p_send_wq->control.mutex );
			pthread_mutex_unlock( &p_send_wq->control.mutex );
			continue;
		}

		/**
		  *ȡ������������
		  */
		uint8_t dest_raw[6] = {0};
		struct sockaddr_in udp_sin;
		uint32_t resp_interval_time = 0;
		p_sdpqueue_wnode p_send_wnode = NULL;

		// ��ȡ��������
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
				free( p_send_wnode ); // �ͷŶ��нڵ�
				p_send_wnode = NULL;
			}

			continue;
		}

		memset( send_frame, 0, sizeof(send_frame) );
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
			free( p_send_wnode ); // �ͷŶ��нڵ�
			p_send_wnode = NULL;
		}

		//DEBUG_LINE();
		// ready to sending data
		pthread_mutex_lock(&ginflight_pro.mutex);
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
		pthread_mutex_unlock(&ginflight_pro.mutex);

		/*��鷢��״̬*/
		over_time_set( SYSTEM_SQUEUE_SEND_INTERVAL, SEND_INTERVAL_TIMEOUT );
#else
		/**
		  *�����з����̺߳�����������
		  *
		  */ 
		uint8_t dest_raw[6] = {0};
		struct sockaddr_in udp_sin;
		uint32_t resp_interval_time = 0;
		p_sdpqueue_wnode p_send_wnode = NULL;
		uint8_t cur_msg_type = 0xff;
		uint8_t next_msg_type = 0xff;
		
		pthread_mutex_lock( &p_send_wq->control.mutex ); // lock mutex
		while( p_send_wq->work.head == NULL && p_send_wq->control.active )
		{
			DEBUG_INFO( "active = %d", p_send_wq->control.active );
			pthread_cond_wait( &p_send_wq->control.cond, &p_send_wq->control.mutex );
		}

		// ��ȡ��������
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
				free( p_send_wnode ); // �ͷŶ��нڵ�
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
			free( p_send_wnode ); // �ͷŶ��нڵ�
			p_send_wnode = NULL;
		}

		cur_msg_type = data_type;
		next_msg_type = get_send_queue_message_type( p_send_wq );
		pthread_mutex_unlock( &p_send_wq->control.mutex ); // unlock mutexpthread_mutex_unlock( &p_send_wq->control.mutex ); // unlock mutex

		// ready to sending data
		pthread_mutex_lock(&ginflight_pro.mutex);
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
		pthread_mutex_unlock(&ginflight_pro.mutex);

		over_time_set( SYSTEM_SQUEUE_SEND_INTERVAL, SEND_INTERVAL_TIMEOUT );
#if 0// ����Ҳ�п��ܷ������ݹ��죬�������ն˴����쳣��2-3-2016
		if ( (((next_msg_type == TRANSMIT_TYPE_ADP) ||(next_msg_type == TRANSMIT_TYPE_ACMP)||(next_msg_type == TRANSMIT_TYPE_AECP)) && ((cur_msg_type != TRANSMIT_TYPE_ADP) && (cur_msg_type != TRANSMIT_TYPE_ACMP) && (cur_msg_type != TRANSMIT_TYPE_AECP)))\
			|| ((next_msg_type == TRANSMIT_TYPE_UDP_SVR) && (cur_msg_type != TRANSMIT_TYPE_UDP_SVR ))\
			||((next_msg_type == TRANSMIT_TYPE_UDP_CLT) && (cur_msg_type != TRANSMIT_TYPE_UDP_CLT))\
			||((next_msg_type == TRANSMIT_TYPE_CAMERA_UART_CTRL) && (cur_msg_type != TRANSMIT_TYPE_CAMERA_UART_CTRL))\
			||((next_msg_type == TRANSMIT_TYPE_MATRIX_UART_CTRL) && (cur_msg_type != TRANSMIT_TYPE_MATRIX_UART_CTRL)))
		{// ��ǰ������Ϣ�Ķ˿�����һ��������Ϣ�Ķ˿ڲ�ͬ������ʼ��һ����Ϣ�ķ���
			int nowait_status = set_wait_message_active_state();
			assert( nowait_status == 0 );
			nowait_status = set_wait_message_idle_state();
			assert( nowait_status == 0 );
			continue;
		}
#else
		if ( (((cur_msg_type == TRANSMIT_TYPE_ADP) || (cur_msg_type == TRANSMIT_TYPE_ACMP) || (cur_msg_type == TRANSMIT_TYPE_AECP) || (cur_msg_type == TRANSMIT_TYPE_UDP_SVR ) || (cur_msg_type == TRANSMIT_TYPE_UDP_CLT))\
			&& ((next_msg_type == TRANSMIT_TYPE_CAMERA_UART_CTRL) ||(next_msg_type == TRANSMIT_TYPE_MATRIX_UART_CTRL)))\
			|| ((cur_msg_type == TRANSMIT_TYPE_CAMERA_UART_CTRL) && (next_msg_type == TRANSMIT_TYPE_MATRIX_UART_CTRL))\
			|| ((cur_msg_type == TRANSMIT_TYPE_MATRIX_UART_CTRL) && (next_msg_type == TRANSMIT_TYPE_CAMERA_UART_CTRL)))
		{// ��ǰ������Ϣ�Ķ˿�����һ��������Ϣ�Ķ˿ڲ�ͬ������ʼ��һ����Ϣ�ķ���,�������
			int nowait_status = set_wait_message_active_state();
			assert( nowait_status == 0 );
			nowait_status = set_wait_message_idle_state();
			assert( nowait_status == 0 );
			continue;
		}
#endif		
		/*������һ�����ݵ�����-���ݻ����Ӧ�����ݳ�ʱ��ʱ��������(ע:ʱ����ֻ������ϵͳ��Ӧ���ݻ�����ͷ�������ݵķ���)*/
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
#if 0
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
		}
		else
		{
			DEBUG_INFO(" not message primed success!" );
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


