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

int send_work_queue_message_save( tx_data* p_queue_msg, sdpwqueue *sd_work_quue )// 注:p_queue_msg中的frame元素是指向已分配堆空间的空间
{
	p_sdpqueue_wnode save_queue_node = NULL;
	uint16_t frame_len = 0;
	assert( p_queue_msg && p_queue_msg->frame );

	if( NULL == p_queue_msg )
		return -1;

	if( queue_size(&sd_work_quue->work) > SEND_QUEUE_MAX_SIZE )
	{
		DEBUG_INFO( "size queue out of brank!" );
		return -1;
	}

	//DEBUG_INFO( "send queue frame len = %d ", p_queue_msg->frame_len );
	frame_len = p_queue_msg->frame_len;
	if( frame_len > TRANSMIT_DATA_BUFFER_SIZE )
	{
		DEBUG_INFO( " out of brank! ", frame_len );
		return -1;
	}
	
	save_queue_node = (p_sdpqueue_wnode)malloc( sizeof(sdpqueue_wnode) ); // free by send thread!
	if( NULL == save_queue_node )
	{
		DEBUG_INFO( "malloc failed: no space!" );
		return -1;
	}

	save_queue_node->job_data.data_type = p_queue_msg->data_type;
	save_queue_node->job_data.frame_len = frame_len;
	memcpy( save_queue_node->job_data.frame, p_queue_msg->frame, frame_len );
	save_queue_node->job_data.notification_flag = p_queue_msg->notification_flag;
	save_queue_node->job_data.resp = p_queue_msg->resp;
	memcpy( &save_queue_node->job_data.raw_dest, p_queue_msg->raw_dest.value, sizeof(struct jdksavdecc_eui48));
	memcpy( &save_queue_node->job_data.udp_sin, &p_queue_msg->udp_sin, sizeof(struct sockaddr_in));
	queue_push( &sd_work_quue->work, (struct queue_node *)save_queue_node );
	
	return 0;
}


