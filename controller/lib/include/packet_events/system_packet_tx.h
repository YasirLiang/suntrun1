#ifndef __SYSTEM_PACKET_TX_H__
#define __SYSTEM_PACKET_TX_H__

#include "jdksavdecc_world.h"
#include "jdksavdecc_pdu.h"
#include "linked_list_unit.h"
#include "pipe.h"
#include "avdecc_net.h"
#include <semaphore.h>

#define TRANSMIT_DATA_BUFFER_SIZE 2048
#define CONFERENCE_RESPONSE_POS 1

enum transmit_data_type
{
	TRANSMIT_TYPE_ADP,
	TRANSMIT_TYPE_ACMP,
	TRANSMIT_TYPE_AECP,
	TRANSMIT_TYPE_UDP_SVR, // send data to udp server
	TRANSMIT_TYPE_UDP_CLT, // send data to udp client

	TRANSMIT_TYPE_UART_CTRL	// send data to uart for controling the camera (2015-11-25 add)
};

typedef struct transmit_data
{
	uint8_t data_type;		// ������������,Ϊenum transmit_data_type�е�����
	bool notification_flag;	// ���ͱ�־
	bool resp;				// ��Ӧ����
	uint8_t *frame;		// �跢�͵����ݻ�����,��СΪ2048������ʹ�öѿռ䣬ԭ���Ǻ���������ջ�ռ�ᱻ�ͷ�
	struct jdksavdecc_eui48 raw_dest;	//raw packet
	uint16_t frame_len;		// ��������С
	struct sockaddr_in udp_sin;//udp addr
}tx_data,*ptr_tx_data;

struct fds;
extern sem_t sem_tx; // �ܵ����ݷ��͵ȴ��ź����������߳̿ɼ������ڹܵ����ݵĿ��Ʒ��͡�
void init_sem_tx_can( void );

int system_raw_queue_tx( void *frame, uint16_t frame_len, uint8_t data_type, const uint8_t dest_mac[6], bool isresp );
void system_raw_packet_tx( const uint8_t dest_mac[6], void *frame, uint16_t frame_len, bool notification, uint8_t data_type, bool isresp );
void system_udp_packet_tx( const struct sockaddr_in *sin, void *frame, uint16_t frame_len, bool notification, uint8_t data_type );
int system_udp_queue_tx( void *frame, uint16_t frame_len, uint8_t data_type,  const struct sockaddr_in *sin );
void system_uart_packet_tx( void *frame, uint16_t frame_len, bool notification, uint8_t data_type, bool isresp );
int system_uart_queue_tx( void *frame, uint16_t frame_len, uint8_t data_type, bool isresp );
void tx_packet_event( uint8_t type,
					bool notification_flag,  
					uint8_t *frame, 
					uint16_t frame_len, 
					struct fds *file_dec, 
					inflight_plist guard, 
					const uint8_t dest_mac[6], 
					struct sockaddr_in* sin, 
					const bool resp, 
					uint32_t *interval_time );


#endif
