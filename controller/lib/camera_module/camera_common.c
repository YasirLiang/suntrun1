#include "camera_common.h"
#include "system_packet_tx.h"

int gcamera_uart_fd = -1; // 串口文件描述符
bool gcamera_uart_set_success = false;

int camera_form_can_send( uint8_t camera_address, uint16_t d_cmd, uint8_t speed_lv, uint8_t speed_vertical )
{
	pelco_d_format askbuf;
	
	pelco_d_cammand_set( camera_address, d_cmd, speed_lv, speed_vertical, &askbuf );
	
#ifdef __CAM_PRINTF__
	DEBUG_SEND( &askbuf, CAMERA_PELCO_D_DEAL_LEN, "Cam Data");
#endif

	system_uart_packet_tx( &askbuf, CAMERA_PELCO_D_DEAL_LEN, RUNINFLIGHT, TRANSMIT_TYPE_CAMERA_UART_CTRL, true );

	return 0;
}

int camera_common_control_init( void )
{
	int fd = -1;
	int err;
	
	fd = UART_File_Open( fd, CAMERA_UART_FILE );
	if( fd == -1 )
	{
		DEBUG_ERR( "uart file open failed!need to check the uart file( %s ) is whether right!",  CAMERA_UART_FILE );
		return -1;
	}
	else
	{
		gcamera_uart_fd = fd;
		err = UART_File_Init( gcamera_uart_fd, 9600, 0, 8, 1, 'N' ); // 设置串口参数
		if( err == -1 )
		{
			DEBUG_ERR( "set port information failed!check the permition or the right port file %s", CAMERA_UART_FILE );
			UART_File_Close( gcamera_uart_fd );
			assert( err != -1 );
		}

		gcamera_uart_set_success = true;
	}

	return 0;
}

int camera_common_control_destroy( void )
{
	gcamera_uart_set_success = false;
	UART_File_Close( gcamera_uart_fd );

	return 0;
}

