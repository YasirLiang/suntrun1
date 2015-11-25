#include "camera_common.h"
#include "system_packet_tx.h"

int uart_fd = -1; // �����ļ�������
bool uart_set_success = false;

int camera_form_can_send( uint8_t camera_address, uint16_t d_cmd, uint8_t speed_lv, uint8_t speed_vertical )
{
	pelco_d_format askbuf;
	
	pelco_d_cammand_set( camera_address, d_cmd, speed_lv, speed_vertical, &askbuf );
	DEBUG_SEND( &askbuf, CAMERA_PELCO_D_DEAL_LEN, "Cam Data");
		
	system_uart_packet_tx( &askbuf, CAMERA_PELCO_D_DEAL_LEN, RUNINFLIGHT, TRANSMIT_TYPE_UART_CTRL, true );

	return 0;
}

int camera_common_control_init( void )
{
	int fd = -1;
	int err;
	
	fd = UART0_Open( fd, CAMERA_UART_FILE );
	if( fd == -1 )
	{
		DEBUG_ERR( "uart file open failed!need to check the uart file( %s ) is whether right!",  CAMERA_UART_FILE );
		return -1;
	}
	else
	{
		uart_fd = fd;
		err = UART0_Init( uart_fd, 9600, 0, 8, 1, 'N' ); // ���ô��ڲ���
		if( err == -1 )
		{
			DEBUG_ERR( "set port information failed!check the permition or the right port file %s", CAMERA_UART_FILE );
			UART0_Close( uart_fd );
			return -1;
		}

		uart_set_success = true;
	}

	return 0;
}

int camera_common_control_destroy( void )
{
	UART0_Close( uart_fd );

	return 0;
}
