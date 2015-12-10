/*uart_controller_machine.c
**Date:2015/11/25
**Description: proccess uart data
**
*/

#include "uart_controller_machine.h"
#include "uart.h"

int transmit_uart_control_packet_uart( uint8_t *frame, uint16_t frame_len, bool isresend, bool isresp )
{
	int send_len = (int)frame_len;
	int err = -1;

	if( uart_set_success )
	{
		err = UART0_Send( uart_fd, frame, send_len );
		if( err == -1 )
		{
			DEBUG_ERR( "uart send Err!" );
		}
	}
	else
	{
		DEBUG_INFO( "uart information not initalization success: Please Check!" );
	}
	
	return err;
}

