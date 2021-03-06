#include "message_queue.h"
#include "func_proccess.h"

int func_command_work_queue_messag_save( fcqueue_data_elem *p_node, fcwqueue *p_fcwq )
{
	uint16_t msg_len = 0;
	if( !(p_node && p_fcwq))
	{
#ifdef __DEBUG__
		DEBUG_INFO("error fcqueue_data_elem node or fcwqueue!");
		assert( p_node && p_fcwq );
#else 
		return -1;
#endif
	}

	msg_len = p_node->meet_msg.data_len;
	if( msg_len > MAX_FUNC_MSG_LEN )
	{
		DEBUG_INFO( ">>>>>>>>>>>-----func message length out of brank!-----<<<<<<" );
		return -1;
	}

	p_fcqueue_wnode p_work_node = NULL;
	p_work_node = (p_fcqueue_wnode)malloc( sizeof(fcqueue_wnode) );
	if( NULL == p_work_node )
	{
#ifdef __DEBUG__
		DEBUG_INFO("fcqueue_wnode malloc err: no space for mallo!");
		assert( NULL != p_work_node );
#else 
		return -1;
#endif
	}

	p_work_node->job_data.func_msg_head.func_index = p_node->func_msg_head.func_index;
	p_work_node->job_data.func_msg_head.func_cmd = p_node->func_msg_head.func_cmd;
	p_work_node->job_data.meet_msg.data_len = msg_len;
	memcpy( p_work_node->job_data.meet_msg.data_buf, p_node->meet_msg.data_buf, msg_len );

	queue_push( &p_fcwq->work, (struct queue_node *)p_work_node );

	return 0;
}

/******************************
**
**功能:从函数命令工作队列中获取队列节点
**
*******************************/
p_fcqueue_wnode func_command_work_queue_messag_get( fcwqueue *p_fcwq )
{
	return (p_fcqueue_wnode)queue_get(&p_fcwq->work);
}

