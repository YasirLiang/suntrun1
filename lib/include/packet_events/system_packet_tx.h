#ifndef __SYSTEM_PACKET_TX_H__
#define __SYSTEM_PACKET_TX_H__

#include "jdksavdecc_world.h"
#include "jdksavdecc_pdu.h"
#include "linked_list_unit.h"
#include "pipe.h"
#include "avdecc_net.h"

#define TRANSMIT_DATA_BUFFER_SIZE 2048
#define CONFERENCE_RESPONSE_POS 1

enum transmit_data_type
{
	TRANSMIT_TYPE_ADP,
	TRANSMIT_TYPE_ACMP,
	TRANSMIT_TYPE_AECP,
	TRANSMIT_TYPE_UDP_SVR,
	TRANSMIT_TYPE_UDP_CLT
};

typedef struct transmit_data
{
	uint8_t data_type;		// 发送数据类型,为enum transmit_data_type中的类型
	bool notification_flag;	// 发送标志
	bool resp;				// 响应数据
	uint8_t *frame;		// 需发送的数据缓冲区,大小为2048，这里使用堆空间，原因是函数结束后栈空间会被释放
	uint16_t frame_len;		// 缓冲区大小
	struct sockaddr_in udp_sin;//udp addr
	struct jdksavdecc_eui48 raw_dest;	//raw packet
}tx_data,*ptr_tx_data;

struct fds;

void system_raw_queue_tx( void *frame, uint16_t frame_len, uint8_t data_type, const uint8_t dest_mac[6], bool isresp );
void system_raw_packet_tx( const uint8_t dest_mac[6], void *frame, uint16_t frame_len, bool notification, uint8_t data_type, bool isresp );
void system_udp_packet_tx( const struct sockaddr_in *sin, void *frame, uint16_t frame_len, bool notification, uint8_t data_type, int fd );
void system_udp_queue_tx( void *frame, uint16_t frame_len, uint8_t data_type,int write_fd,  const struct sockaddr_in *sin );
void tx_packet_event( uint8_t type, bool notification_flag,  uint8_t *frame, uint16_t frame_len, struct fds *file_dec, inflight_plist guard, const uint8_t dest_mac[6], struct sockaddr_in* sin,const bool resp);


#endif
