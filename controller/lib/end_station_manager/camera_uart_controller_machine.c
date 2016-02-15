/*uart_controller_machine.c
**Date:2015/11/25
**Description: proccess uart data
**
*/

#include "camera_uart_controller_machine.h"
#include "uart.h"
#include "pelco_d.h"
#include "util.h"
#include "system_packet_tx.h"
#include "camera_common.h"

int transmit_camera_uart_control_packet( uint8_t *frame, uint16_t frame_len, bool isresend, bool isresp, uint32_t *interval_time )
{
	int send_len = (int)frame_len;
	int err = -1;
	uint16_t cmd = pelco_d_cammand_get( frame, 0 );
	uint32_t timeout = get_send_respons_interval_timeout( TRANSMIT_TYPE_CAMERA_UART_CTRL, cmd );
	assert( interval_time );
	*interval_time = timeout;
	
	if( gcamera_uart_set_success )
	{
		err = UART_File_Send( gcamera_uart_fd, frame, send_len );
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

