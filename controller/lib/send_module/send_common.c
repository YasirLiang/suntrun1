/**
*file:send_common.c
*date:2016-4-29
*
*/

//********************************
//ϵͳ���͹���ģ�齨��
//
//********************************

#include "send_common.h"

#define CHECK_SYSTEM_SEND_QUEUE_TIMEOUT 200 //  ���ϵͳ�Ƿ���������ɵĳ�ʱ���ʱ��(MS)

sem_t sem_waiting; // ���͵ȴ��ź����������߳̿ɼ�
sdpwqueue net_send_queue;// �������ݷ��͹�������

#ifdef SEND_DOUBLE_QUEUE_EABLE
sdpwqueue gwrite_send_queue; // ����˫���е�д����
#endif

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

	over_time_set( CHECK_SYSTEM_SEND_QUEUE_INDEX, CHECK_SYSTEM_SEND_QUEUE_TIMEOUT );
}

// �ݻٷ��Ͷ���
void destroy_network_send_work_queue( void )
{
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

	if( is_queue_empty(&net_send_queue.work) )
	{
		if( net_send_queue.work.trail != NULL )
			net_send_queue.work.trail = NULL;
	}
	
	pthread_mutex_unlock( &net_send_queue.control.mutex );
	
	controll_deactivate( &net_send_queue.control );
	/*bool is_su = controll_destroy( &net_send_queue.control ); 
	if( !is_su )
	{
		DABORT( is_su );
	}*/
}

void system_packet_save_send_queue( tx_data tnt )
{
	// �����������ݷ��Ͷ���
	uint16_t frame_len = tnt.frame_len;
	sdpwqueue*  send_wq = &net_send_queue;

	if( (frame_len > TRANSMIT_DATA_BUFFER_SIZE) || (frame_len < 0) )
	{
		return ;
	}
	
#ifdef SEND_DOUBLE_QUEUE_EABLE
	bool write_empty = is_queue_empty( &gwrite_send_queue.work );
	bool read_empty = is_queue_empty( &send_wq->work );

	pthread_mutex_lock( &send_wq->control.mutex );
	send_work_queue_message_save( tnt, &gwrite_send_queue );
	pthread_mutex_unlock( &send_wq->control.mutex );

	if( write_empty && read_empty )
	{
		pthread_cond_signal( &send_wq->control.cond );
	}
#else
	/**
	*������ֱ�ӱ����跢�͵�����
	*�����Ͷ�����
	*
	*/
	sdpwqueue*  send_wq = &net_send_queue;
	pthread_mutex_lock( &send_wq->control.mutex );
	
	send_work_queue_message_save( tnt, send_wq );
	
	pthread_cond_signal( &send_wq->control.cond );
	pthread_mutex_unlock( &send_wq->control.mutex ); // unlock mutex
#endif
}

void send_common_check_squeue( void )
{
	if( over_time_listen(CHECK_SYSTEM_SEND_QUEUE_INDEX) )
	{
		bool write_empty = is_queue_empty( &gwrite_send_queue.work );
		bool read_empty = is_queue_empty( &net_send_queue.work );

		if( !read_empty  || !write_empty )
		{
			if( write_empty && read_empty )
			{
				pthread_cond_signal( &net_send_queue->control.cond );
			}
		}
	}	
}

int 

