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
	assert( p_queue_msg );

	if( NULL == p_queue_msg )
		return -1;

	p_sdpqueue_wnode save_queue_node = NULL;
	save_queue_node = (p_sdpqueue_wnode)malloc( sizeof(sdpqueue_wnode) );
	if( NULL == save_queue_node )
	{
		DEBUG_INFO( "malloc failed: no space!" );
		return -1;
	}

	memcpy( &save_queue_node->job_data, p_queue_msg, sizeof(tx_data));
	queue_push( &net_send_queue.work, (struct queue_node *)save_queue_node );

	return 0;
}


