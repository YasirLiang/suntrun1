/*send_work_queue.h
**Date:2015-11-5
**
**
*/

#ifndef __SEND_WORK_QUEUE_H__
#define __SEND_WORK_QUEUE_H__

#include "circular_link_queue.h"
#include "system_packet_tx.h"

#define SEND_QUEUE_MAX_SIZE 120 // ���Ķ���Ԫ��

//#pragma pack(1)
typedef struct _type_send_data
{
	uint8_t data_type;		// ������������,Ϊenum transmit_data_type�е�����
	bool notification_flag;	// ���ͱ�־
	bool resp;				// ��Ӧ����
	uint8_t *frame;		// �跢�͵����ݻ�����,��СΪ2048������ʹ�öѿռ䣬ԭ���Ǻ���������ջ�ռ�ᱻ�ͷ�
	uint8_t raw_dest[6];// (ע��:������Ϊ��������ʱ��ʹ�ú���tx_packet_event�����߸�����ʱ��malloc���ǳ�����Ϊ���߸�����������ʱconst uint8_t dest_mac[6],ʵ�����β����Ͳ�һ����ʵ�����Ⱑ,��const���ε��βε�ע��ʵ�εĴ��ݰ�!!!!! )
	//struct jdksavdecc_eui48 raw_dest;	//raw packet
	uint16_t frame_len;		// ��������С
	struct sockaddr_in udp_sin;//udp addr
}tsend_data;
//#pragma pack()

typedef struct _tsend_data_pthread_queue_work_node   // ����Э���������ݷ��Ͷ��еĹ����ڵ�
{
	queue_node *next; 			// �������нڵ��������(�����нڵ�)
	tsend_data job_data; 			// �������������ʽ
}sdpqueue_wnode, *p_sdpqueue_wnode;

typedef struct _tsend_data_pthread_work_queue // ϵͳ���ݷ��Ͷ���
{
	data_control control;	// ��������
	queue work;			// ��������
}sdpwqueue;

extern uint8_t get_send_queue_message_type( sdpwqueue* send_wq );
extern p_sdpqueue_wnode send_queue_message_get( sdpwqueue* send_wq );
extern int send_work_queue_message_save( tx_data p_queue_msg, sdpwqueue *sd_work_quue );// ע:p_queue_msg�е�frameԪ����ָ���ѷ���ѿռ�Ŀռ�
extern bool swap_sdpqueue( sdpwqueue *sdpw_queue1, sdpwqueue *sdpw_queue2 );// �������Ͷ���

#endif
