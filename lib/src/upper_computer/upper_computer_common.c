#include "upper_computer_common.h"

struct udp_client upper_udp_client;		    // 上位机的通信信息
bool  is_upper_udp_client_connect = false;

/****************************
*writer:YasirLiang
*first date: 2015/10/15
*Name:	upper_computer_send
*Func:	send upper computer data to system pipe
*param:
*	send_data: data of upper computer deal data except deal check data
*	is_resp:
*		true: data is host response data
*		false:  data isnot host response data
*Return value: real ready send frame data lenght
*	
*/
int upper_computer_send( void* data_send, bool is_resp )
{
	assert( data_send );
	struct host_upper_cmpt *askbuf = (struct host_upper_cmpt*)data_send;
	struct host_upper_cmpt_frame upper_send_frame;
	uint16_t upper_send_frame_len = 0;
	int ret = 0;

	upper_send_frame.payload_len = OTHER_DATA_LENGHT + askbuf->common_header.data_len;
	upper_send_frame_len = upper_send_frame.payload_len;
	ret = conference_host_to_upper_computer_form_msg( &upper_send_frame, askbuf );// need to set check first in this function
	if( ret < 0 )
	{
		DEBUG_INFO( "computer frame form failed!");
		assert( ret >= 0 );
	}

	if( is_upper_udp_client_connect ) // 是否有客户端连接上主机
		system_udp_packet_tx( &upper_udp_client.cltaddr, upper_send_frame.payload, upper_send_frame.payload_len, RUNINFLIGHT, TRANSMIT_TYPE_UDP_CLT );

	return upper_send_frame_len;
}

