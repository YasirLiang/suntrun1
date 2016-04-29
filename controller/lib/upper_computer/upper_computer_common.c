#include "upper_computer_common.h"
#include "upper_computer_data_parser.h"
#include "udp_client_controller_machine.h"
#include "upper_computer_data_form.h"
#include "ring_buf.h"
#include "time_handle.h"
#include "wait_message.h"
#include "send_common.h"

tchar_ring_buf grecv_ucmpt_buf_pro;// 上位机环形接受缓冲区 处理参数
#define RECV_FROM_UPPTER_BUF_SIZE 2048 // 上位机环形接受缓冲区 大小
uint8_t grecv_from_uppter_buf[RECV_FROM_UPPTER_BUF_SIZE];//上位机环形接受缓冲区
struct socket_info_s upper_udp_client;		    // 上位机的通信信息
bool  is_upper_udp_client_connect = false;
thost_upper_cmpt_msg grecv_upper_cmpt_msg;// 上位机消息结构体
trecv_cmpt_pro grecv_cmpt_msg_pro;//  处理上位机结构

#ifdef __PRINTF_UPD_PACKET__
void test_udp_printf(const void *pri_load, size_t load_len, char *msg)
{
	uint8_t *p = ( uint8_t * )pri_load;
	int i = 0;

	fprintf( stdout, "%s  ",  msg);
	for( ; i < ( int )load_len; i++ )
		fprintf( stdout, "%02x ", *(p + i) );
	fprintf( stdout, "\n" );
}
#endif

void upper_cmpt_command_askbuf_set( struct host_upper_cmpt *askbuf, uint8_t deal_type, uint8_t command, const void *data, uint16_t data_len )
{
	assert( askbuf );
	askbuf->common_header.state_loader = UPPER_COMPUTER_DATA_LOADER;
	askbuf->common_header.deal_type = deal_type;
	askbuf->common_header.command = command;
	askbuf->common_header.data_len = data_len;

	if( (data_len != 0) && (NULL != data))
	{
		if( data_len <= DATA_PAYLOAD_LEN_MAX )
			memcpy( askbuf->data_payload , (uint8_t *)data, data_len );// 这里是从低字节开始拷贝的，所以data必须协议数据传输的顺序一致
		else
		{	
			DEBUG_INFO( "error upper computer trasmit deal data:too length!" );
			assert( data_len <= DATA_PAYLOAD_LEN_MAX );
		}
	}
}

/****************************
*writer:YasirLiang
*first date: 2015/10/15
*Name:	upper_computer_send
*Func:	send upper computer data to system pipe
*param:
*	send_data: data of upper computer deal data except deal check data
*Return value: real ready send frame data lenght
*	
*/
int upper_computer_send( struct host_upper_cmpt* data_send )
{
	assert( data_send );
	struct host_upper_cmpt_frame upper_send_frame;
	int ret = 0;

	memset( &upper_send_frame, 0, sizeof(struct host_upper_cmpt_frame) );
	upper_send_frame.payload_len = OTHER_DATA_LENGHT + data_send->common_header.data_len;
	ret = conference_host_to_upper_computer_form_msg( &upper_send_frame, data_send );// need to set check first in this function
	if( ret < 0 )
	{
		DEBUG_INFO( "computer frame form failed!");
		assert( ret >= 0 );
	}

	if( is_upper_udp_client_connect )
	{
#ifdef __PRINTF_UPD_PACKET__
		//DEBUG_INFO( " (ret =%d)?= (pay len = %d)", ret, upper_send_frame.payload_len );
		test_udp_printf( upper_send_frame.payload, upper_send_frame.payload_len, "Udp Client Send Data:" );
#endif
		system_udp_packet_tx( &upper_udp_client.sock_addr, upper_send_frame.payload, upper_send_frame.payload_len, RUNINFLIGHT, TRANSMIT_TYPE_UDP_CLT );
	}

	return (upper_send_frame.payload_len);
}

/*************************************
*Writer:	YasirLiang
*Date: 2015/10/19
*Name:send_upper_computer_command
*Func: send upper and host deal command data to upper computer
*Param:
*	data: data except common head, while where is not data, data be setted NULL and data len is zero
*/
int  send_upper_and_host_deal_command( uint8_t deal_type, uint8_t command, const void *data, uint16_t data_len )
{
	struct host_upper_cmpt askbuf;
	//DEBUG_INFO( "sizeof struct host_upper_cmpt = %d ", sizeof(struct host_upper_cmpt) );
	memset( &askbuf, 0, sizeof(struct host_upper_cmpt));
	
	upper_cmpt_command_askbuf_set( &askbuf, deal_type, command, data, data_len);
	upper_computer_send( &askbuf );

	return 0;
}

/*************************************
*Writer:	YasirLiang
*Date: 2015/10/16
*Name:host_controller_machine_reply_upper_computer
*Func: reply upper and host deal data to upper computer( response interface )
*Param:
*	data: data except common head, while where is not data, data be setted NULL and data len is zero
*/
int host_controller_machine_reply_upper_computer( uint8_t deal_type, uint8_t command, const void *data, uint16_t data_len )
{
	struct host_upper_cmpt askbuf;
	memset( &askbuf, 0, sizeof(struct host_upper_cmpt));
	deal_type |= CMPT_MSG_TYPE_RESPONSE;
	
	upper_cmpt_command_askbuf_set( &askbuf, deal_type, command, data, data_len );
	upper_computer_send( &askbuf );

	return 0;
}

// 上报主机错误
void upper_computer_reply_error( uint8_t *recv_msg )
{
	assert( recv_msg );
	uint8_t deal_type = get_host_upper_cmpt_deal_type( recv_msg, ZERO_OFFSET_IN_PAYLOAD );
	uint8_t cmpt_cmd = get_host_upper_cmpt_command_type( recv_msg, ZERO_OFFSET_IN_PAYLOAD );
	deal_type |= (deal_type & CMPT_MSG_TYPE_MARK) | CMPT_MSG_TYPE_CPT_DIS ;

	host_controller_machine_reply_upper_computer( deal_type, cmpt_cmd, NULL, 0);  
}

/************************************************
*Writer:	YasirLiang
*Date: 2015/10/16
*Name:proccess_udp_client_msg_recv
*Func: proccess data recv from udp client
*Param:
*	frame:receive data buf
*	frame_len: receive data(execpt check data 2016/1/19 )
*return value:
*		none
**************************************************/
void proccess_udp_client_msg_recv( uint8_t *frame, int frame_len, int *status )
{
	uint8_t protocol_type = get_host_upper_cmpt_deal_type( frame, ZERO_OFFSET_IN_PAYLOAD );
	uint8_t cmpt_cmd = get_host_upper_cmpt_command_type( frame, ZERO_OFFSET_IN_PAYLOAD );
	*status = (int)(protocol_type & CMPT_MSG_TYPE_RESPONSE);
	
	DEBUG_RECV( frame, frame_len, "Udp client Recv");
	if(  !(protocol_type & CMPT_MSG_TYPE_RESPONSE) ) // not a response data
	{
		// proccess upper computer data 
		if( !find_func_command_link( COMPUTER_USE, cmpt_cmd, protocol_type, frame, (uint16_t)frame_len ) )
		{
			// send proccess err
			upper_computer_reply_error( frame );
		}
	}
	else // update send data inflight command 
	{
		udp_client_update_inflight_comand( frame, frame_len );
	}
}

/*************************************************
*Writer:	YasirLiang
*Date: 2016/1/19
*Name:upper_computer_common_recvmesssage_save
*Func: save recv data from udp client
*Param:
*	frame:receive data buf
*	frame_len: receive data 
*return value:0:normal
*		-1:error
***************************************************/
int upper_computer_common_recv_messsage_save( int fd, struct sockaddr_in *sin_in, bool udp_exist, socklen_t sin_len, uint8_t *frame, uint16_t frame_len )
{
	int ret = -1;
	const uint16_t msg_len = frame_len;
	uint16_t point = 0;
	
	if( sin_in == NULL || frame == NULL )
		return ret;

	if( msg_len <= UPPER_RECV_BUF_MAX )
	{
		upper_udp_client.sock_fd = fd;
		memcpy( &upper_udp_client.sock_addr, sin_in, sizeof(struct sockaddr_in) );
		upper_udp_client.sock_len = sin_len;
		is_upper_udp_client_connect = udp_exist;

		point = 0;
		while( point < msg_len )
		{
			char_ring_buf_save( &grecv_ucmpt_buf_pro, frame[point++] );
		}
	}
	
	return ret;
}

/*************************************************
*Writer:	YasirLiang
*Date: 2016/1/19
*Name:upper_computer_recv_message_get_pro
*Func: get recv ring buf data and proccess it
*Param:none 
*return value:none
***************************************************/
void upper_computer_recv_message_get_pro( void )
{
	uint8_t ch_temp;
	bool ret = false;
	while( char_ring_buf_get( &grecv_ucmpt_buf_pro, &ch_temp ))
	{
		ret = upper_computer_comm_recv_msg_pro( &grecv_upper_cmpt_msg, ch_temp );
		over_time_set( CPT_RCV_GAP, 10 );
	}

	if( over_time_listen(CPT_RCV_GAP) )
	{
		grecv_cmpt_msg_pro.msg_len = 0;
		over_time_stop(CPT_RCV_GAP);
	}
}

bool upper_computer_comm_recv_msg_pro( thost_upper_cmpt_msg *pmsg, uint8_t save_char )
{
	if( pmsg == NULL )
	{
		grecv_cmpt_msg_pro.msg_len = 0;
		return false;
	}

	if( (grecv_cmpt_msg_pro.msg_len == 0) && (save_char == UPPER_COMPUTER_DATA_LOADER) )
	{
		pmsg->state_loader = save_char;
		grecv_cmpt_msg_pro.msg_len = 1;
	}
	else if( grecv_cmpt_msg_pro.msg_len == 1 )
	{
		pmsg->deal_type = save_char;
		grecv_cmpt_msg_pro.msg_len = 2;
	}
	else if( grecv_cmpt_msg_pro.msg_len == 2 )
	{
		pmsg->command = save_char;
		grecv_cmpt_msg_pro.msg_len = 3;
	}
	else if( grecv_cmpt_msg_pro.msg_len == 3 )
	{
		pmsg->data_len = save_char;
		grecv_cmpt_msg_pro.msg_len = 4;
	}
	else if( grecv_cmpt_msg_pro.msg_len == 4 )
	{
		pmsg->data_len |= (save_char << 8);
		if( pmsg->data_len <= UPPER_RECV_BUF_MAX )
		{
			grecv_cmpt_msg_pro.msg_len = 5;
			grecv_cmpt_msg_pro.data_len = 0;
		}
		else
		{
			grecv_cmpt_msg_pro.msg_len = 0;
		}
	}
	else if( grecv_cmpt_msg_pro.msg_len >= HOST_UPPER_COMPUTER_COMMON_LEN )
	{
		if( grecv_cmpt_msg_pro.data_len < pmsg->data_len )
		{
			pmsg->data_payload[grecv_cmpt_msg_pro.data_len] = save_char;
			grecv_cmpt_msg_pro.msg_len++;
		}
		else if( grecv_cmpt_msg_pro.data_len == pmsg->data_len )
		{
			uint8_t *p = (uint8_t*)pmsg;
			uint8_t count_chk = 0, recv_chk = save_char;
			uint16_t msg_len = grecv_cmpt_msg_pro.msg_len;
			if( msg_len == (HOST_UPPER_COMPUTER_COMMON_LEN + pmsg->data_len) )
			{
				int i = 0;
				for( i = 0; i < msg_len; i++ )
				{
					count_chk += *(p++);
				}

				if( recv_chk == count_chk )
				{
					// 处理接收的上位机发送过来的数据包
					int rx_status = -1;
					proccess_udp_client_msg_recv((uint8_t*)pmsg, msg_len, &rx_status );
					if( rx_status && is_wait_messsage_active_state() )
					{
						set_wait_message_status( 0 );
						sem_post( &sem_waiting );
					}

					grecv_cmpt_msg_pro.msg_len = 0;
					return true;
				}
			}

			grecv_cmpt_msg_pro.msg_len = 0;
		}

		grecv_cmpt_msg_pro.data_len++;
	}

	return false;
}

void upper_computer_common_init( void )
{
	grecv_ucmpt_buf_pro.empty = true;
	grecv_ucmpt_buf_pro.buf_size = RECV_FROM_UPPTER_BUF_SIZE;
	grecv_ucmpt_buf_pro.head = 0;
	grecv_ucmpt_buf_pro.trail = 0;
	grecv_ucmpt_buf_pro.pring_buf = grecv_from_uppter_buf;

	grecv_cmpt_msg_pro.data_len = 0;
	grecv_cmpt_msg_pro.msg_len = 0;
}

