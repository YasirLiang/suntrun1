/*send_work_queue.c
**Date:2015-11-5
**
**
*/

#include "send_work_queue.h"

sdpwqueue net_send_queue;// 网络数据发送工作队列

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

p_sdpqueue_wnode send_queue_message_get( sdpwqueue* send_wq )
{
	return (p_sdpqueue_wnode)queue_get( &send_wq->work );
}

int send_work_queue_message_save( tx_data* p_queue_msg )// 注:p_queue_msg中的frame元素是指向已分配堆空间的空间
{
	p_sdpqueue_wnode save_queue_node = NULL;
	assert( p_queue_msg && p_queue_msg->frame );

	if( NULL == p_queue_msg )
		return -1;

	save_queue_node = (p_sdpqueue_wnode)malloc( sizeof(sdpqueue_wnode) ); // free by send thread!
	if( NULL == save_queue_node )
	{
		DEBUG_INFO( "malloc failed: no space!" );
		return -1;
	}

	save_queue_node->job_data.data_type = p_queue_msg->data_type;
	save_queue_node->job_data.frame_len = p_queue_msg->frame_len;
	save_queue_node->job_data.notification_flag = p_queue_msg->notification_flag;
	save_queue_node->job_data.resp = p_queue_msg->resp;
	save_queue_node->job_data.frame = p_queue_msg->frame;
	memcpy( &save_queue_node->job_data.raw_dest, p_queue_msg->raw_dest.value, sizeof(struct jdksavdecc_eui48));
	memcpy( &save_queue_node->job_data.udp_sin, &p_queue_msg->udp_sin, sizeof(struct sockaddr_in));
	queue_push( &net_send_queue.work, (struct queue_node *)save_queue_node );
	
	return 0;
}


