#include "system_packet_tx.h"
#include "adp_controller_machine.h"
#include "acmp_controller_machine.h"
#include "aecp_controller_machine.h"
#include "udp_server_controller_machine.h"
#include "udp_client_controller_machine.h"
#include "wait_message.h"
#include "camera_uart_controller_machine.h"
#include "send_common.h" // ����SEND_DOUBLE_QUEUE_EABLE
#include "matrix_output_input.h"

sem_t sem_tx; // �ܵ����ݷ��͵ȴ��ź����������߳̿ɼ������ڹܵ����ݵĿ��Ʒ��͡�
static uint8_t gsys_tx_buf[TRANSMIT_DATA_BUFFER_SIZE] = {0};// �ܵ����ݻ������� ����ܵ��ĵ��߳�ʹ�ã�ʹ���ź���ͬ������-->sem_tx

void init_sem_tx_can( void ) // ��ʼ���ܵ������ź���->����gsys_tx_buf
{
	sem_init( &sem_tx, 0, 0 );
}

int system_raw_queue_tx( void *frame, uint16_t frame_len, uint8_t data_type, const uint8_t dest_mac[6], bool isresp )
{
	assert( frame);
	
	int ret = -1;
	if( (data_type == TRANSMIT_TYPE_ADP) || (data_type == TRANSMIT_TYPE_ACMP) || (data_type == TRANSMIT_TYPE_AECP) )
	{
		tx_data tx;

		if( frame_len > TRANSMIT_DATA_BUFFER_SIZE )
		{
			DEBUG_INFO( "frame_len bigger than pipe transmit buffer!" );
			return -1;
		}

		memset( gsys_tx_buf, 0, sizeof(gsys_tx_buf) );
		memcpy( tx.raw_dest, dest_mac, sizeof(struct jdksavdecc_eui48) );
		memcpy( gsys_tx_buf, (uint8_t*)frame, frame_len );
		tx.frame = gsys_tx_buf;
		tx.data_type = data_type;
		tx.frame_len = frame_len;
		tx.notification_flag = RUNINFLIGHT;
		tx.resp = isresp;

		ret = system_packet_save_send_queue( tx );
	}
	else
	{
		DEBUG_INFO( "ERR transmit data type" );
	}

	return ret;
}

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
*return value:-1: err; 0 send OK
**************************************************/
int system_raw_packet_tx( const uint8_t dest_mac[6], void *frame, uint16_t frame_len, bool notification, uint8_t data_type, bool isresp )
{
	assert( dest_mac && frame);
	
	if( !notification )
		return -1;

	return system_raw_queue_tx( frame, frame_len, data_type, dest_mac, isresp );
}

/**************************
*writer:YasirLiang
*change data: 2015/10/15
*change cotent: delect the last param "int write_fd"
*	before change:void system_udp_queue_tx( void *frame, uint16_t frame_len, uint8_t data_type,int write_fd,  const struct sockaddr_in *sin )
*	after change:void system_udp_queue_tx( void *frame, uint16_t frame_len, uint8_t data_type, const struct sockaddr_in *sin )
*
*/
int system_udp_queue_tx( void *frame, uint16_t frame_len, uint8_t data_type, const struct sockaddr_in *sin )
{
	assert( sin && frame );
	
	int ret = -1;
	if( (data_type == TRANSMIT_TYPE_UDP_SVR) || (data_type == TRANSMIT_TYPE_UDP_CLT) )
	{
		tx_data tx;
		bool resp = is_conference_deal_data_response_type( frame, CONFERENCE_RESPONSE_POS );// Э��ڶ����ֽ�λ8Ϊ��Ӧ��־only userful between upper computer and host controller AS SO FAR (150909)
		
		if( frame_len > TRANSMIT_DATA_BUFFER_SIZE )
		{
			DEBUG_INFO( "frame_len bigger than pipe transmit buffer!" );
			return -1;
		}
		
		memset( gsys_tx_buf, 0, sizeof(gsys_tx_buf) );
		memcpy( gsys_tx_buf, frame, frame_len );
		tx.frame = gsys_tx_buf;
		tx.data_type = data_type;
		tx.frame_len = frame_len;
		tx.notification_flag = RUNINFLIGHT;
		tx.resp = resp;
		memcpy( &tx.udp_sin, sin, sizeof( struct sockaddr_in ) );

		ret = system_packet_save_send_queue( tx );
	}
	else
	{
		DEBUG_INFO( "transmit data type not udp clt or srv" );
	}

	return ret;
}

/**************************
*writer:YasirLiang
*change data: 2015/10/15
*change cotent: delect the last param "int fd"
*	before change:void system_udp_packet_tx( const struct sockaddr_in *sin, void *frame, uint16_t frame_len, bool notification, uint8_t data_type, int fd )
*	after change:void system_udp_packet_tx( const struct sockaddr_in *sin, void *frame, uint16_t frame_len, bool notification, uint8_t data_type )
*
*/
int system_udp_packet_tx( const struct sockaddr_in *sin, void *frame, uint16_t frame_len, bool notification, uint8_t data_type )
{
	assert( sin && frame);
	
	if( !notification )
		return -1;
	
	return system_udp_queue_tx( frame, frame_len, data_type, sin );
}

/***********************************************
*Writer:YasirLiang
*Data: 2015/11/25
*Name: system_uart_packet_tx
*Param:
*	frame:the sending data 
*	frame_len:the length of sending data
*	data_type: the data type of sending data by system sending
*Return Value:None
************************************************/
int system_uart_queue_tx( void *frame, uint16_t frame_len, uint8_t data_type, bool isresp )
{
	assert( frame );
	
	int ret = -1;
	
	if( (data_type == TRANSMIT_TYPE_CAMERA_UART_CTRL) || (data_type == TRANSMIT_TYPE_MATRIX_UART_CTRL) )
	{
		tx_data tx;
		bool resp = isresp; // no need camera response data 

		if( frame_len > TRANSMIT_DATA_BUFFER_SIZE )
		{
			DEBUG_INFO( "frame_len bigger than pipe transmit buffer!" );
			return -1;
		}

		memset( gsys_tx_buf, 0, TRANSMIT_DATA_BUFFER_SIZE );
		memcpy( gsys_tx_buf, frame, frame_len );
		tx.frame = gsys_tx_buf;
		tx.data_type = data_type;
		tx.frame_len = frame_len;
		tx.notification_flag = RUNINFLIGHT;
		tx.resp = resp;

		ret = system_packet_save_send_queue( tx );
	}
	else
	{
		DEBUG_INFO( "transmit data type not uart data!" );
	}

	return ret;
}

/***********************************************
*Writer:YasirLiang
*Data: 2015/11/25
*Name: system_uart_packet_tx
*Func:send data to uart output
*Param:
*	frame:the sending data 
*	frame_len:the length of sending data
*	notification: decide whether send
*	data_type: the data type of sending data by system sending
*Return Value:None
************************************************/

int system_uart_packet_tx( void *frame, uint16_t frame_len, bool notification, uint8_t data_type, bool isresp )
{
	assert( frame);
	
	if( !notification )
		return -1;
	
	return system_uart_queue_tx( frame, frame_len, data_type, isresp );
}

/****************************
*Writer: YasirLiang 
*Date: 2016/04/29
*Name:system_tx
*����: ϵͳ��������(raw udp...)�����������ܽӿ�
*����:
*	frame:���ݻ�����ָ��
*	frame_len:���ݳ���
*	notification:�Ƿ���������(����)����(����)
*	data_type:�������ݡ�������������
*	isresp:�Ƿ�����Ӧ����
*	dest_mac:�ն�Ŀ�ĵ�ַ��1722�㲥��ַ
*	sin:udp Ŀ�ĵ�ַ
*����ֵ:���ʹ��󷵻�-1 
*****************************/
int system_tx( void *frame, 
				uint16_t frame_len, 
				bool notification, 
				uint8_t data_type, 
				bool isresp,
				const uint8_t dest_mac[6],
				const struct sockaddr_in *sin )
{
	uint8_t genre_tx = data_type;
	uint8_t raw_dest[6] = {0};
	struct sockaddr_in  tmp_sin;
	int ret = -1;
	 
	switch( genre_tx )
	{
		case TRANSMIT_TYPE_ADP:
		case TRANSMIT_TYPE_ACMP:
		case TRANSMIT_TYPE_AECP:
			if( dest_mac != NULL )
			{
				memcpy( raw_dest, dest_mac, 6 );
				ret = system_raw_packet_tx( raw_dest, frame, frame_len, notification, data_type, isresp );
			}
			break;
		case TRANSMIT_TYPE_UDP_SVR:
		case TRANSMIT_TYPE_UDP_CLT:
			 if( sin != NULL )
			 {
				memcpy( &tmp_sin, sin, sizeof(struct sockaddr_in) );
				ret = system_udp_packet_tx( &tmp_sin, frame, frame_len, notification,  data_type );
			 }
			break;
		case TRANSMIT_TYPE_CAMERA_UART_CTRL:
		case TRANSMIT_TYPE_MATRIX_UART_CTRL:
			ret = system_uart_packet_tx( frame, frame_len, notification, data_type, isresp );
			break;
		default:
			DEBUG_INFO("SYSTEM TX ERR: NO such data type(%d)", genre_tx );
			break;
	}

	return ret;
}

/****************************
*writer:YasirLiang ---2015/09/09
*����: �������ݷ��ͺ���,Э���޹�
*����:
*	type:������������
*	notification_flag:���ͱ�־
*	frame:�������ݻ�����
*	frame_len:���ݳ���
*	file_dec:�׽���������
*	guard:inflight ��������ͷ���.ע��:inflight_frame��Ҫ���·����ڴ�ռ�������������ݣ�
*		��Ϊtx_packet_event���ý�������ͷ�frame�Ŀռ䣬��inflight �����������һֱ������ϵͳ�У�
*		ֱ������ɹ������������ݡ�
*����ֵ:
*state: ע��frame(������)�ĳ��ȱ������50���ֽڣ�������ڴ�Խ��.
*****************************/
int tx_packet_event( uint8_t type,
					bool notification_flag,  
					uint8_t *frame, 
					uint16_t frame_len, 
					struct fds *file_dec, 
					inflight_plist guard, 
					uint8_t dest_mac[6], 
					struct sockaddr_in* sin, 
					bool resp, 
					uint32_t *interval_time )
{
	int server_fd = 0;
	int client_fd = 0;
	bool istx = false;
#ifndef SEND_DOUBLE_QUEUE_EABLE
	bool right_packet = true;
#endif
	struct sockaddr_in sin_event;
	
	if( notification_flag == RUNINFLIGHT )
	{
	 	server_fd = file_dec->udp_server_fd;
		client_fd = file_dec->udp_client_fd;
		memcpy( &sin_event, sin, sizeof(struct sockaddr_in) );
		istx = true;
	}

	// check the valid address,if not valid use the whole values
	uint64_t dest_addr = 0;
	uint8_t dest[6];
	convert_eui48_to_uint64(dest_mac, &dest_addr);
	if( dest_addr != 0)
		memcpy( dest, dest_mac, 6 );
	else
		memcpy( dest, jdksavdecc_multicast_adp_acmp.value, 6 );

	assert( interval_time );
	if( istx )
	{
		if( type == TRANSMIT_TYPE_ADP )
		{
			transmit_adp_packet_to_net( frame, frame_len, NULL, false, dest, resp, interval_time );
		}
		else if( type == TRANSMIT_TYPE_ACMP )
		{
			transmit_acmp_packet_network( frame, frame_len, NULL, false, dest, resp, interval_time );
		}
		else if( type == TRANSMIT_TYPE_AECP )
		{
			transmit_aecp_packet_network( frame, frame_len, NULL, false, dest, resp, interval_time );
		}
		else if( type == TRANSMIT_TYPE_UDP_SVR )// host as client send data to udp server using client fd
		{ 
			transmit_udp_packet_server( client_fd, frame, frame_len, NULL, false, &sin_event, resp, interval_time );// δ��ɣ�ԭ����Э��û��
		}
		else if( type == TRANSMIT_TYPE_UDP_CLT )// host as server send data to udp client using server fd
		{ 
			transmit_udp_client_packet( server_fd, frame, frame_len, NULL, false, &sin_event, resp, interval_time );
		}
		else if( type == TRANSMIT_TYPE_CAMERA_UART_CTRL )
		{ 
			transmit_camera_uart_control_packet( frame, frame_len, false, resp, interval_time ); 
		}
		else if( type == TRANSMIT_TYPE_MATRIX_UART_CTRL )
		{
#ifdef __DEBUG__
			int i = 0;
			fprintf( stdout, "Matrix command string => " );
			for( i = 0; i < frame_len; i++ )	
				fprintf( stdout, "%c", frame[i] );
			fprintf( stdout, "\n" );
#endif
			matrix_output_transmit_uart_control_packet( frame, frame_len, false, resp, interval_time );
		}
		else 
		{
			DEBUG_INFO("NO match transmit data type, Please check!");
#ifndef SEND_DOUBLE_QUEUE_EABLE
			right_packet = false;
#endif
		}
#ifndef SEND_DOUBLE_QUEUE_EABLE
		if( right_packet )
		{
			int status = 0;
			status = set_wait_message_primed_state();
			assert( status == 0 );
		}
#endif
	}
	else
	{
		DEBUG_INFO( "nothing entry send!" );
		return -1;
	}

	return 0;
}

