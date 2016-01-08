#include "upper_computer_common.h"
#include "upper_computer_data_parser.h"
#include "udp_client_controller_machine.h"
#include "upper_computer_data_form.h"

struct udp_client upper_udp_client;		    // 上位机的通信信息
bool  is_upper_udp_client_connect = false;


void test_udp_printf(const void *pri_load, size_t load_len, char *msg)
{
	uint8_t *p = ( uint8_t * )pri_load;
	int i = 0;

	fprintf( stdout, "%s  ",  msg);
	for( ; i < ( int )load_len; i++ )
		fprintf( stdout, "%02x ", *(p + i) );
	fprintf( stdout, "\n" );
}

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
int upper_computer_send( void* data_send )
{
	assert( data_send );
	struct host_upper_cmpt *askbuf = (struct host_upper_cmpt*)data_send;
	struct host_upper_cmpt_frame upper_send_frame;
	int ret = 0;

	memset( &upper_send_frame, 0, sizeof(struct host_upper_cmpt_frame) );
	upper_send_frame.payload_len = OTHER_DATA_LENGHT + askbuf->common_header.data_len;
	ret = conference_host_to_upper_computer_form_msg( &upper_send_frame, askbuf );// need to set check first in this function
	if( ret < 0 )
	{
		DEBUG_INFO( "computer frame form failed!");
		assert( ret >= 0 );
	}

	if( is_upper_udp_client_connect )
	{
#ifdef __DEBUG__
		//DEBUG_INFO( " (ret =%d)?= (pay len = %d)", ret, upper_send_frame.payload_len );
		test_udp_printf( upper_send_frame.payload, upper_send_frame.payload_len, "Udp Client Send Data:" );
#endif
		system_udp_packet_tx( &upper_udp_client.cltaddr, upper_send_frame.payload, upper_send_frame.payload_len, RUNINFLIGHT, TRANSMIT_TYPE_UDP_CLT );
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
*	frame_len: receive data 
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

