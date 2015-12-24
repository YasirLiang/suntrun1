/*uart_controller_machine.c
**Date:2015/11/25
**Description: proccess uart data
**
*/

#include "uart_controller_machine.h"
#include "uart.h"
#include "pelco_d.h"
#include "util.h"
#include "system_packet_tx.h"

int transmit_uart_control_packet_uart( uint8_t *frame, uint16_t frame_len, bool isresend, bool isresp, uint32_t *interval_time )
{
	int send_len = (int)frame_len;
	int err = -1;
	uint16_t cmd = pelco_d_cammand_get( frame, 0 );
	uint32_t timeout = get_send_respons_interval_timeout( TRANSMIT_TYPE_UART_CTRL, cmd );
	assert( interval_time );
	*interval_time = timeout;
	
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

