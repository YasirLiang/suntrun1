#include "upper_computer_common.h"

/****************************
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
	int send_len = 0;

	send_len = conference_host_to_upper_computer_form_msg( &upper_send_frame, askbuf );
	
}