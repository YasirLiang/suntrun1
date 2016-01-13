/*send_work_queue.c
**Date:2015-11-5
**
**
*/

#include "send_work_queue.h"

p_sdpqueue_wnode send_queue_message_get( sdpwqueue* send_wq )
{
	return (p_sdpqueue_wnode)queue_get( &send_wq->work );
}

int send_work_queue_message_save( tx_data queue_msg, sdpwqueue *sd_work_quue )// 注:p_queue_msg中的frame元素是指向已分配堆空间的空间
{
	p_sdpqueue_wnode save_queue_node = NULL;
	uint16_t frame_len = 0;
	assert( queue_msg.frame );
	
	if( queue_size(&sd_work_quue->work) > SEND_QUEUE_MAX_SIZE )
	{
		DEBUG_INFO( "size queue out of brank!" );
		return -1;
	}

	//DEBUG_INFO( "send queue frame len = %d ", queue_msg->frame_len );
	frame_len = queue_msg.frame_len;
	if( (frame_len > TRANSMIT_DATA_BUFFER_SIZE) || (frame_len <= 0))
	{
		DEBUG_INFO( " out of brank !or little!!! frame len = %d", frame_len );
		return -1;
	}
	
	//DEBUG_INFO(" sizeof struct sockaddr_in = %d", sizeof(struct sockaddr_in) );
	//DEBUG_INFO(" sizeof tsend_data = %d", sizeof(tsend_data) );
	//DEBUG_INFO(" sizeof sdpqueue_wnode = %d", sizeof(sdpqueue_wnode) );
	save_queue_node = (p_sdpqueue_wnode)malloc( sizeof(sdpqueue_wnode) ); // free by send thread!
	if( NULL == save_queue_node )
	{
		DEBUG_INFO( "malloc failed: no space!" );
		return -1;
	}

	save_queue_node->job_data.frame = (uint8_t*)malloc( frame_len * sizeof(uint8_t) );
	if( save_queue_node->job_data.frame == NULL )
	{
		DEBUG_INFO( "malloc failed: no send queue space!" );
		return -1;
	}

	save_queue_node->job_data.data_type = queue_msg.data_type;
	save_queue_node->job_data.frame_len = frame_len;
	memcpy( save_queue_node->job_data.frame, queue_msg.frame, frame_len );
	save_queue_node->job_data.notification_flag = queue_msg.notification_flag;
	save_queue_node->job_data.resp = queue_msg.resp;
	memcpy( save_queue_node->job_data.raw_dest, queue_msg.raw_dest, sizeof(struct jdksavdecc_eui48));
	memcpy( &save_queue_node->job_data.udp_sin, &queue_msg.udp_sin, sizeof(struct sockaddr_in));
	queue_push( &sd_work_quue->work, (struct queue_node *)save_queue_node );
	
	return 0;
}


