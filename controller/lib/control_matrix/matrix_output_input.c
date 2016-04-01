/*
**matrix_output.c
**2-15-2016
*/

#include "matrix_output_input.h"
#include "system_packet_tx.h"

int gmatrix_output_file = -1;	// 矩阵控制输出文件
volatile bool gmatrix_file_set_success = false; // 矩阵控制输出文件初始化成功

uint16_t matrix_input_recieve_message( uint8_t *p_msg_buf, uint16_t recv_len )
{
	uint16_t msg_len = 0;
	if( (gmatrix_file_set_success) && (recv_len > 0) )
	{
		msg_len = read( gmatrix_output_file, p_msg_buf, recv_len );
		if( msg_len < 0 )
		{
			DEBUG_INFO( "recv matrix uart msg Err!" );
			return -1;
		}

		return msg_len;
	}

	return -1;
}

int matrix_output_transmit_uart_control_packet( uint8_t *frame, uint16_t frame_len, bool isresend, bool isresp, uint32_t *interval_time )
{
	int send_len = (int)frame_len;
	int err = -1;
	uint16_t cmd = 0xffff;
	uint32_t timeout = get_send_respons_interval_timeout( TRANSMIT_TYPE_MATRIX_UART_CTRL, cmd );
	
	assert( interval_time );
	if( interval_time == NULL )
		return -1;
	*interval_time = timeout;

	if( gmatrix_file_set_success )
	{
		err = UART_File_Send( gmatrix_output_file, frame, send_len );
		if( err == -1 )
		{
			DEBUG_ERR( "uart send Err!" );
		}
	}
	else
	{
		DEBUG_INFO( "uart (%s)information not initalization success: Please Check!", MATRIX_OUTPUT_FILE );
	}
	
	return err;
}

/***************************
*矩阵控制输出端口初始化*/
int matrix_output_init( void )
{
	int fd = -1;
	int err;
	
	fd = UART_File_Open( fd, MATRIX_OUTPUT_FILE );
	if( fd == -1 )
	{
		DEBUG_ERR( "matrix_output_file ( %s ) open failed!",  MATRIX_OUTPUT_FILE );
		return -1;
	}
	else
	{
		gmatrix_output_file = fd;
		err = UART_File_Init( gmatrix_output_file, 9600, 0, 8, 1, 'N' ); // 设置串口参数
		if( err == -1 )
		{
			DEBUG_ERR( "set port information failed!check the permition or the right port file: %s", MATRIX_OUTPUT_FILE );
			UART_File_Close( gmatrix_output_file );
			assert( err != -1 );
			return -1;
		}
		else
		{
			gmatrix_file_set_success = true;
		}
	}

	return 0;
}

