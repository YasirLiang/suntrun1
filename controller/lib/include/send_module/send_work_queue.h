/*send_work_queue.h
**Date:2015-11-5
**
**
*/

#ifndef __SEND_WORK_QUEUE_H__
#define __SEND_WORK_QUEUE_H__

#include "circular_link_queue.h"
#include "system_packet_tx.h"

#define SEND_QUEUE_MAX_SIZE 120 // 最大的队列元素

//#pragma pack(1)
typedef struct _type_send_data
{
	uint8_t data_type;		// 发送数据类型,为enum transmit_data_type中的类型
	bool notification_flag;	// 发送标志
	bool resp;				// 响应数据
	uint8_t *frame;		// 需发送的数据缓冲区,大小为2048，这里使用堆空间，原因是函数结束后栈空间会被释放
	uint8_t raw_dest[6];// (注意:若参数为下面类型时，使用函数tx_packet_event传第七个参数时，malloc总是出错，因为第七个参数的类型时const uint8_t dest_mac[6],实参与形参类型不一致着实出问题啊,有const修饰的形参得注意实参的传递啊!!!!! )
	//struct jdksavdecc_eui48 raw_dest;	//raw packet
	uint16_t frame_len;		// 缓冲区大小
	struct sockaddr_in udp_sin;//udp addr
}tsend_data;
//#pragma pack()

typedef struct _tsend_data_pthread_queue_work_node   // 会议协议命令数据发送队列的工作节点
{
	queue_node *next; 			// 工作队列节点的链表区(即队列节点)
	tsend_data job_data; 			// 发送数据命令格式
}sdpqueue_wnode, *p_sdpqueue_wnode;

typedef struct _tsend_data_pthread_work_queue // 系统数据发送队列
{
	data_control control;	// 控制数据
	queue work;			// 工作队列
}sdpwqueue;

extern uint8_t get_send_queue_message_type( sdpwqueue* send_wq );
extern p_sdpqueue_wnode send_queue_message_get( sdpwqueue* send_wq );
extern int send_work_queue_message_save( tx_data p_queue_msg, sdpwqueue *sd_work_quue );// 注:p_queue_msg中的frame元素是指向已分配堆空间的空间
extern bool swap_sdpqueue( sdpwqueue *sdpw_queue1, sdpwqueue *sdpw_queue2 );// 交换发送队列

#endif
