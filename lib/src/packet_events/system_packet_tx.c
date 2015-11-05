#include "system_packet_tx.h"
#include "adp_controller_machine.h"
#include "acmp_controller_machine.h"
#include "aecp_controller_machine.h"
#include "udp_server_controller_machine.h"
#include "udp_client_controller_machine.h"
#include "wait_message.h"

/************************************************
*Name:	system_raw_packet_tx
*Function:	to send a frame data to system unnamed pipe	
*parameters:
*		dest_mac:	destination of sending data 
*		frame:	sending data
*		frame_len:	sending date len
*		notification: decide data whether send
*		data_type: type of data sending 
*		isresp:  ture:response data of host controller;false: 
*return value:none
**************************************************/
void system_raw_packet_tx( const uint8_t dest_mac[6], void *frame, uint16_t frame_len, bool notification, uint8_t data_type, bool isresp )
{
	assert( dest_mac && frame);
	
	if( notification )
		system_raw_queue_tx( frame, frame_len, data_type, dest_mac, isresp );
}

void system_raw_queue_tx( void *frame, uint16_t frame_len, uint8_t data_type, const uint8_t dest_mac[6], bool isresp )
{
	assert( frame);
	
	int ret = 0;
	if( (data_type == TRANSMIT_TYPE_ADP) || (data_type == TRANSMIT_TYPE_ACMP) || (data_type == TRANSMIT_TYPE_AECP) )
	{
		tx_data tx;
		uint8_t *tran_buf;
		memset(&tx.udp_sin, 0, sizeof( struct sockaddr_in ) );

		// heap using later free by reading pipe thread.tran_buf space must to be free! 
		tran_buf = allot_heap_space( TRANSMIT_DATA_BUFFER_SIZE, &tran_buf );
		memcpy( tran_buf, frame, frame_len );
		tx.frame = tran_buf;
		tx.data_type = data_type;
		tx.frame_len = frame_len;
		tx.notification_flag = RUNINFLIGHT;
		tx.resp = isresp;
		memcpy(tx.raw_dest.value, dest_mac, 6);
		
		if( ret == write_pipe_tx(&tx, sizeof(tx_data)))
		{
			DEBUG_INFO( "ERR transmit data to PIPE" );
			assert(-1 != ret);
		}
	}
	else
	{
		DEBUG_INFO( "ERR transmit data type" );
		return;
	}
}

/**************************
*writer:YasirLiang
*change data: 2015/10/15
*change cotent: delect the last param "int fd"
*	before change:void system_udp_packet_tx( const struct sockaddr_in *sin, void *frame, uint16_t frame_len, bool notification, uint8_t data_type, int fd )
*	after change:void system_udp_packet_tx( const struct sockaddr_in *sin, void *frame, uint16_t frame_len, bool notification, uint8_t data_type )
*
*/
void system_udp_packet_tx( const struct sockaddr_in *sin, void *frame, uint16_t frame_len, bool notification, uint8_t data_type )
{
	assert( sin && frame);
	
	if( notification )
		system_udp_queue_tx( frame, frame_len, data_type, sin );
}

/**************************
*writer:YasirLiang
*change data: 2015/10/15
*change cotent: delect the last param "int write_fd"
*	before change:void system_udp_queue_tx( void *frame, uint16_t frame_len, uint8_t data_type,int write_fd,  const struct sockaddr_in *sin )
*	after change:void system_udp_queue_tx( void *frame, uint16_t frame_len, uint8_t data_type, const struct sockaddr_in *sin )
*
*/
void system_udp_queue_tx( void *frame, uint16_t frame_len, uint8_t data_type, const struct sockaddr_in *sin )
{
	assert( sin && frame);
	
	int ret = 0;
	
	if( (data_type == TRANSMIT_TYPE_UDP_SVR) || (data_type == TRANSMIT_TYPE_UDP_CLT) )
	{
		tx_data tx;
		uint8_t *tran_buf;
		bool resp = is_conference_deal_data_response_type( frame, CONFERENCE_RESPONSE_POS );// 协议第二个字节位8为响应标志only userful between upper computer and host controller AS SO FAR (150909)
		memset( tx.raw_dest.value, 0, 6 );

		// heap using later free by reading pipe thread.its space must to be free! 
		tran_buf = allot_heap_space( TRANSMIT_DATA_BUFFER_SIZE, &tran_buf );
		memcpy( tran_buf, frame, frame_len );
		tx.frame = tran_buf;
		tx.data_type = data_type;
		tx.frame_len = frame_len;
		tx.notification_flag = RUNINFLIGHT;
		tx.resp = resp;
		memcpy(&tx.udp_sin, sin, sizeof( struct sockaddr_in ) );
		if( ret == write_pipe_tx(&tx, sizeof(tx_data)))
		{
			DEBUG_INFO( "ERR transmit data to PIPE" );
			assert(-1 != ret);
		}
	}
	else
	{
		DEBUG_INFO( "transmit data type not udp clt or srv" );
		return;
	}
}


/****************************
*writer:YasirLiang ---2015/09/09
*功能: 网络数据发送函数,协议无关
*参数:
*	type:网络数据类型
*	notification_flag:发送标志
*	frame:发送数据缓冲区
*	frame_len:数据长度
*	file_dec:套接字描述符
*	guard:inflight 命令链表头结点.注意:inflight_frame需要重新分配内存空间来存放网络数据，
*		因为tx_packet_event调用结束后会释放frame的空间，而inflight 命令链表则会一直存在于系统中，
*		直到程序成功发送网络数据。
*返回值:无
****************************/
void tx_packet_event( uint8_t type, bool notification_flag,  uint8_t *frame, uint16_t frame_len, struct fds *file_dec, inflight_plist guard, const uint8_t dest_mac[6], struct sockaddr_in* sin, const bool resp)
{
	int server_fd = 0;
	int client_fd = 0;
	bool istx = false;
	struct sockaddr_in sin_event;
	uint8_t subtype = jdksavdecc_common_control_header_get_subtype( frame, ZERO_OFFSET_IN_PAYLOAD );
	
	if( notification_flag == RUNINFLIGHT )
	{
	 	server_fd = file_dec->udp_server_fd;
		client_fd = file_dec->udp_client_fd;
		memcpy( &sin_event, sin, sizeof(struct sockaddr_in) );
		istx = true;
	}

	// check the valid address,if not valid use the whole values
	uint64_t dest_addr = 0;
	struct jdksavdecc_eui48 dest;
	convert_eui48_to_uint64(dest_mac, &dest_addr);
	if( dest_addr != 0)
		memcpy(&dest, dest_mac, 6 );
	else
		memcpy(&dest, jdksavdecc_multicast_adp_acmp.value, 6);
	
	if( istx )
	{
		switch( type )
		{
			
			case TRANSMIT_TYPE_ADP:
			{
				if( subtype == JDKSAVDECC_SUBTYPE_ADP )
					transmit_adp_packet_to_net( frame, frame_len, guard, false, dest.value, resp );// 未完成

				else
				{
					DEBUG_INFO("Err ADP data!");
					break;
				}
			}
			break;
			case TRANSMIT_TYPE_ACMP:
			{
				if( subtype == JDKSAVDECC_SUBTYPE_ACMP )			
					transmit_acmp_packet_network( frame, frame_len, guard, false, dest.value, resp );
				else
				{
					DEBUG_INFO("Err ACMP data!");
					break;
				}
			}
			break;
			case TRANSMIT_TYPE_AECP:	// 这里包含了主机与终端通信的协议
			{
				if( subtype == JDKSAVDECC_SUBTYPE_AECP )
				{
					transmit_aecp_packet_network( frame, frame_len, guard, false, dest.value, resp );
				}
				else
				{
					DEBUG_INFO("Err AECP data!");
					break;
				}
			}
			break;
			case TRANSMIT_TYPE_UDP_SVR: // host as client send data to udp server using client fd
			{
				transmit_udp_packet_server( client_fd, frame, frame_len, guard, false, &sin_event, resp );// 未完成，原因是协议没定
			}
			break;
			case TRANSMIT_TYPE_UDP_CLT: // host as server send data to udp client using server fd
			{
				transmit_udp_client_packet( server_fd, frame, frame_len, guard, false, &sin_event, resp );
			}
			break;
			default:
				DEBUG_INFO("NO match transmit data type, Please check!");
				break;
		}
		if( !resp )
		{
			set_wait_message_primed_state();
		}
	}
	else
	{
		DEBUG_INFO( "nothing entry send!" );
		return;
	}
}

