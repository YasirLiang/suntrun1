/*
  *file:system_1722_recv_handle.c
  *data:2016-07-08
  *Author:梁永富
  *descriptor:增加双消息链式队列机制，
  *把数据的处理权交由数据处理线程，
  *如果通过这方法还不能解决问题(大数据量接收与处理的问题)，
  *再考虑接收缓冲区与应用缓冲区机制
  *
  */
#include "system_1722_recv_handle.h"
#include "send_common.h"// the file "SEND_DOUBLE_QUEUE_EABLE" define in
#include "controller_machine.h"
#include "raw_network.h"
#include "handle_1722_packet_from_raw.h"

#ifdef RECV_DOUBLE_LINK_QUEUE_EN
recv_raw_data_wqueue grecv_wqueue;
recv_raw_data_wqueue gapp_wqueue;

int system_1722_recv_handle_raw_data(void)
{
	p_recv_raw_queue_wnode raw_data_node = NULL;
	raw_data_node = system_1722_recv_handle_get_queue_node();
	if (raw_data_node != NULL)
	{
		uint64_t dest_addr = 0;
		uint64_t default_native_dest = 0;
		struct jdksavdecc_frame *frame = &raw_data_node->job_data.frame;
		raw_net_1722_user_info *raw_usr_obj = (raw_net_1722_user_info *)gp_controller_machine->unit_1722_net->network_1722_user_obj;

		convert_eui48_to_uint64( frame->dest_address.value, &dest_addr );
		convert_eui48_to_uint64( jdksavdecc_multicast_adp_acmp.value, &default_native_dest );
		if( frame->ethertype == JDKSAVDECC_AVTP_ETHERTYPE && \
			((((uint8_t)(dest_addr >> 40)) & 0x01) ||(dest_addr == raw_usr_obj->mac)))
		{	
		       	int rx_status = -1;
		       	bool is_notification_id_valid = false;
		        uint16_t operation_id = 0;
		       	bool is_operation_id_valid = false;
			
			uint8_t subtype = jdksavdecc_common_control_header_get_subtype( frame->payload, ZERO_OFFSET_IN_PAYLOAD );

			if (subtype == JDKSAVDECC_SUBTYPE_61883_IIDC)
			{// proccessing audio in this case, because of not use lock of "ginflight_pro.mutex"
				
			}
			else
			{
				pthread_mutex_lock(&ginflight_pro.mutex);
				rx_raw_packet_event( frame->dest_address.value, frame->src_address.value, &is_notification_id_valid, endpoint_list, frame->payload, frame->length, &rx_status, operation_id, is_operation_id_valid );
				pthread_mutex_unlock(&ginflight_pro.mutex);
			}
#ifndef SEND_DOUBLE_QUEUE_EABLE
			if( ((rx_status == 0) && is_wait_messsage_active_state()) || (acmp_recv_resp_err && is_wait_messsage_active_state()) )
			{
				int msr_status = 0;
				msr_status = set_wait_message_status( rx_status );
				assert( msr_status == 0 );
				sem_post( &sem_waiting ); 
				acmp_recv_resp_err = false;
			}
#endif
		}

		free(raw_data_node);
		raw_data_node = NULL;
	}

	return 0;
}

int system_1722_recv_handle_swap_queue(void)
{
	queue tmp_queue;

	tmp_queue.head = grecv_wqueue.work.head;
	tmp_queue.trail =grecv_wqueue.work.trail;
	grecv_wqueue.work.head = gapp_wqueue.work.head;
	grecv_wqueue.work.trail = gapp_wqueue.work.trail;
	gapp_wqueue.work.head = tmp_queue.head;
	gapp_wqueue.work.trail =  tmp_queue.trail;

	return 0;
}

/*此函数确保了当队列为空时，队尾节点是空的*/
p_recv_raw_queue_wnode system_1722_recv_handle_get_queue_node(void)
{
	bool write_empty = is_queue_empty( &grecv_wqueue.work );
	bool read_empty = is_queue_empty( &gapp_wqueue.work );

	/**
	  *读队列为空，写不为空，交换读写队列,必须上锁
	  */ 
	if( read_empty && !write_empty )
	{
		pthread_mutex_lock( &grecv_wqueue.control.mutex );
		system_1722_recv_handle_swap_queue();
		pthread_mutex_unlock( &grecv_wqueue.control.mutex );
	}

	p_recv_raw_queue_wnode queue_node = NULL;
	if (!read_empty)
	{
		queue_node = (p_recv_raw_queue_wnode)queue_get(&gapp_wqueue.work);
		if( gapp_wqueue.work.head == NULL )
		{ // while queue isempty,make sure the queue trail not to be used again!!!!  
			if( gapp_wqueue.work.trail != NULL )
			{
				gapp_wqueue.work.trail = NULL;
			}
		}
	}

	return queue_node;
}

int system_1722_recv_handle_save_queue_node(struct jdksavdecc_frame *frame)
{
	if (frame == NULL)
		return -1;

	pthread_mutex_lock( &grecv_wqueue.control.mutex );
	// save to grecv_wqueue
	p_recv_raw_queue_wnode raw_queue_node = (p_recv_raw_queue_wnode)malloc(sizeof(recv_raw_queue_wnode));//must free in app handle pthread
	if (raw_queue_node != NULL)
	{
		memcpy( &raw_queue_node->job_data.frame, frame, sizeof(struct jdksavdecc_frame));// 拷贝数据
		raw_queue_node->job_data.last_recv_tick = get_current_time();
		queue_push(&grecv_wqueue.work, (struct queue_node *)raw_queue_node);
	}
	
	pthread_mutex_unlock( &grecv_wqueue.control.mutex );

	return 0;
}


void system_1722_recv_handle_init(void)
{
	bool is_su = false;
	
	is_su = controll_init( &grecv_wqueue.control );// 只初始化了recv的控制参数,锁
	if( !is_su )
		DABORT( is_su );
	
	init_queue( &grecv_wqueue.work );
	init_queue( &gapp_wqueue.work );
	
	is_su = controll_activate( &grecv_wqueue.control );
	is_su = controll_activate( &gapp_wqueue.control );
	if( !is_su );
		DABORT( is_su );
}

void system_1722_recv_handle_destroy( void )
{
	queue_node * q_node = NULL;

	pthread_mutex_lock( &grecv_wqueue.control.mutex );
	
	while( !is_queue_empty( &grecv_wqueue.work ) ) // release node
	{
		q_node = queue_get(&grecv_wqueue.work);
		if( NULL != q_node )
		{
			free( q_node );
			q_node = NULL;	
		}
	}

	if( is_queue_empty(&grecv_wqueue.work) )
	{
		if( grecv_wqueue.work.trail != NULL )
			grecv_wqueue.work.trail = NULL;
	}
	
	pthread_mutex_unlock( &grecv_wqueue.control.mutex );
	
	controll_deactivate( &grecv_wqueue.control );

	while( !is_queue_empty( &gapp_wqueue.work ) ) // release node
	{
		q_node = queue_get(&gapp_wqueue.work);
		if( NULL != q_node )
		{
			free( q_node );
			q_node = NULL;	
		}
	}

	if( is_queue_empty(&gapp_wqueue.work) )
	{
		if( gapp_wqueue.work.trail != NULL )
			gapp_wqueue.work.trail = NULL;
	}

	controll_deactivate( &gapp_wqueue.control );
}

#else

int system_1722_recv_handle_raw_data(void)
{
	return 0;
}

void system_1722_recv_handle_init(void)
{
	
}

void system_1722_recv_handle_destroy( void )
{

}

#endif
