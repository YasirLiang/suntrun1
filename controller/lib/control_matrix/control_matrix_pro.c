/*
**control_matrix_pro.c
**17-2-2016
**
**
*/

#include "control_matrix_pro.h"
#include "control_matrix_command.h"
#include "system_packet_tx.h"

#define MATRIX_COMMAND_BUF_SIZE 256
uint8_t gmatrix_command_buf[MATRIX_COMMAND_BUF_SIZE]; // 发送矩阵命令缓冲区 
uint8_t gmatrix_comand_system_flags = MATRIX_SET_CREATOR20;

/*设置矩阵ascii命令发送缓冲区的命令数据*/
int control_matrix_command_form( uint16_t matrix_command, uint16_t extra_len, uint8_t* p_extra_data, uint16_t *send_len )
{
	uint16_t ascii_cmd_len = 0;
	const uint8_t *p_cmd = NULL;
	uint8_t *p_command_buf = gmatrix_command_buf;
	int ret = -1;

	if( (send_len != NULL) && ( p_command_buf != NULL))
	{
		switch( matrix_command )
		{
			case MATRIX_QUEUE_TYPE:
			case MATRIX_LOCK_KEYBOARD:
			case MATRIX_UNLOCK_KEYBOARD:
			case MATRIX_CLOSE_BELL:
			case MATRIX_OPEN_BELL:
			case MATRIX_QUEUE_VERSION:
			case MATRIX_SET_EXTRON:
			case MATRIX_SET_CREATOR20:
			case MATRIX_CLOSE_SERIAL_RETURN_MSG:
			case MATRIX_OPEN_SERIAL_RETURN_MSG:
			case MATRIX_SET_OUTIN_CORRESPONDING:
			case MATRIX_CLOSE_ALL_OUTPUT_CHANNAL:
			case MATRIX_QUEUE_OUT_CHANNAL_INPUT_STATUS:
			{
				p_cmd = control_matrix_comand_get( matrix_command, &ascii_cmd_len );
				if( (ascii_cmd_len <= MATRIX_COMMAND_BUF_SIZE) && \
					(p_cmd != NULL) && (ascii_cmd_len != 0) )
				{
					memcpy( p_command_buf, p_cmd, ascii_cmd_len );
					*send_len = ascii_cmd_len;
					ret = 0;
				}

				break;
			}
			case MATRIX_CHANGE_KEY_PASSWORD:
				if( (p_extra_data != NULL) && (extra_len == 8))
				{
					p_command_buf[0] = '/';
					p_command_buf[1] = '+';
					memcpy( &p_command_buf[2], p_extra_data, extra_len );
					p_command_buf[10] = ';';
					*send_len = 11;
					ret = 0;
				}
				break;
			case MATRIX_SET_LCD_BACKLIGHT_TIME:
				if( (p_extra_data != NULL) && (extra_len == 3))
				{
					memcpy( p_command_buf, MATRIX_STRING_CMD_SET_LCD_BACKLIGHT_TIME, \
						MATRIX_STRING_CMD_SET_LCD_BACKLIGHT_TIME_LEN );
					memcpy( &p_command_buf[MATRIX_STRING_CMD_SET_LCD_BACKLIGHT_TIME_LEN], \
						p_extra_data, extra_len);
					p_command_buf[MATRIX_STRING_CMD_SET_LCD_BACKLIGHT_TIME_LEN+3] = ';';
					*send_len = MATRIX_STRING_CMD_SET_LCD_BACKLIGHT_TIME_LEN+4;
					ret = 0;
				}
				break;
			default:
				break;
		}
	}

	return ret;
}

/*==============开始指令系统=====================================================*/
int control_matrix_set_outin_corresponding( void )
{
	uint16_t send_len = 0;

	if( gmatrix_comand_system_flags == MATRIX_SET_CREATOR20 )
	{
		if( 0 == control_matrix_command_form( MATRIX_SET_OUTIN_CORRESPONDING, 0, NULL, &send_len ))// form success?
		{
			system_uart_packet_tx( gmatrix_command_buf, send_len, RUNINFLIGHT, TRANSMIT_TYPE_MATRIX_UART_CTRL, true );
		}
	}

	return 0;
}

int control_matrix_close_all_out_channel( void )
{
	uint16_t send_len = 0;

	if( gmatrix_comand_system_flags == MATRIX_SET_CREATOR20 )
	{
		if( 0 == control_matrix_command_form( MATRIX_CLOSE_ALL_OUTPUT_CHANNAL, 0, NULL, &send_len ))// form success?
		{
			system_uart_packet_tx( gmatrix_command_buf, send_len, RUNINFLIGHT, TRANSMIT_TYPE_MATRIX_UART_CTRL, true );
		}
	}
	
	return 0;
}

int control_matrix_queue_input_status_for_out( void )
{
	uint16_t send_len = 0;

	if( gmatrix_comand_system_flags == MATRIX_SET_CREATOR20 )
	{
		if( 0 == control_matrix_command_form( MATRIX_QUEUE_OUT_CHANNAL_INPUT_STATUS, 0, NULL, &send_len ))// form success?
		{
			system_uart_packet_tx( gmatrix_command_buf, send_len, RUNINFLIGHT, TRANSMIT_TYPE_MATRIX_UART_CTRL, true );
		}
	}

	return 0;
}

// 矩阵的切换
int control_matrix_input_output_switch( void )
{
	uint16_t send_len = 0;

	if( gmatrix_comand_system_flags == MATRIX_SET_CREATOR20 )
	{
		if( 0 == control_matrix_command_form( MATRIX_QUEUE_OUT_CHANNAL_INPUT_STATUS, 0, NULL, &send_len ))// form success?
		{
			system_uart_packet_tx( gmatrix_command_buf, send_len, RUNINFLIGHT, TRANSMIT_TYPE_MATRIX_UART_CTRL, true );
		}
	}
	
	return 0;
}

/*==============结束指令系统=====================================================*/


/*==============开始兼容指令=====================================================*/
/*==============结束兼容指令=====================================================*/

/*==============开始系统指令=====================================================*/
int queue_control_matrix_type( void )
{
	uint16_t send_len = 0;
	
	if( 0 == control_matrix_command_form( MATRIX_QUEUE_TYPE, 0, NULL, &send_len ))// form success?
	{
		system_uart_packet_tx( gmatrix_command_buf, send_len, RUNINFLIGHT, TRANSMIT_TYPE_MATRIX_UART_CTRL, true );
	}
	
	return 0;
}

int control_change_keyboard_lock_password( uint8_t* p_password, uint16_t pass_len )
{
	uint16_t send_len = 0;
	uint8_t pass_buf[8] = {0};
	
	if( pass_len != 8 || p_password == NULL )
		return -1;

	memcpy( pass_buf, p_password, pass_len );
	if( 0 == control_matrix_command_form( MATRIX_CHANGE_KEY_PASSWORD, pass_len, pass_buf, &send_len ))// form success?
	{
		system_uart_packet_tx( gmatrix_command_buf, send_len, RUNINFLIGHT, TRANSMIT_TYPE_MATRIX_UART_CTRL, true );
	}

	return 0;
}

int control_matrix_lock_keyboard( void )
{
	uint16_t send_len = 0;
	
	if( 0 == control_matrix_command_form( MATRIX_LOCK_KEYBOARD, 0, NULL, &send_len ))// form success?
	{
		system_uart_packet_tx( gmatrix_command_buf, send_len, RUNINFLIGHT, TRANSMIT_TYPE_MATRIX_UART_CTRL, true );
	}

	return 0;
}

int control_matrix_unlock_keyboard( void )
{
	uint16_t send_len = 0;
	
	if( 0 == control_matrix_command_form( MATRIX_UNLOCK_KEYBOARD, 0, NULL, &send_len ))// form success?
	{
		system_uart_packet_tx( gmatrix_command_buf, send_len, RUNINFLIGHT, TRANSMIT_TYPE_MATRIX_UART_CTRL, true );
	}

	return 0;
}

int control_matrix_close_bell( void )
{
	uint16_t send_len = 0;
	
	if( 0 == control_matrix_command_form( MATRIX_CLOSE_BELL, 0, NULL, &send_len ))// form success?
	{
		system_uart_packet_tx( gmatrix_command_buf, send_len, RUNINFLIGHT, TRANSMIT_TYPE_MATRIX_UART_CTRL, true );
	}

	return 0;
}

int control_matrix_open_bell( void )
{
	uint16_t send_len = 0;
	
	if( 0 == control_matrix_command_form( MATRIX_OPEN_BELL, 0, NULL, &send_len ))// form success?
	{
		system_uart_packet_tx( gmatrix_command_buf, send_len, RUNINFLIGHT, TRANSMIT_TYPE_MATRIX_UART_CTRL, true );
	}

	return 0;
}

int contro_matrix_queue_program_version( void )
{
	uint16_t send_len = 0;
	
	if( 0 == control_matrix_command_form( MATRIX_QUEUE_VERSION, 0, NULL, &send_len ))// form success?
	{
		system_uart_packet_tx( gmatrix_command_buf, send_len, RUNINFLIGHT, TRANSMIT_TYPE_MATRIX_UART_CTRL, true );
	}

	return 0;
}

int control_matrix_set_extron( void )
{
	uint16_t send_len = 0;
	
	if( 0 == control_matrix_command_form( MATRIX_SET_EXTRON, 0, NULL, &send_len ))// form success?
	{
		system_uart_packet_tx( gmatrix_command_buf, send_len, RUNINFLIGHT, TRANSMIT_TYPE_MATRIX_UART_CTRL, true );
		gmatrix_comand_system_flags = MATRIX_SET_EXTRON;
	}

	return 0;
}

int control_matrix_set_creator20( void )
{
	uint16_t send_len = 0;
	
	if( 0 == control_matrix_command_form( MATRIX_SET_CREATOR20, 0, NULL, &send_len ))// form success?
	{
		system_uart_packet_tx( gmatrix_command_buf, send_len, RUNINFLIGHT, TRANSMIT_TYPE_MATRIX_UART_CTRL, true );
		gmatrix_comand_system_flags = MATRIX_SET_CREATOR20;
	}

	return 0;
}

int control_matrix_close_serial_msg_return( void )
{
	uint16_t send_len = 0;
	
	if( 0 == control_matrix_command_form( MATRIX_CLOSE_SERIAL_RETURN_MSG, 0, NULL, &send_len ))// form success?
	{
		system_uart_packet_tx( gmatrix_command_buf, send_len, RUNINFLIGHT, TRANSMIT_TYPE_MATRIX_UART_CTRL, true );
	}

	return 0;
}

int control_matrix_open_serial_msg_return( void )
{
	uint16_t send_len = 0;
	
	if( 0 == control_matrix_command_form( MATRIX_OPEN_SERIAL_RETURN_MSG, 0, NULL, &send_len ))// form success?
	{
		system_uart_packet_tx( gmatrix_command_buf, send_len, RUNINFLIGHT, TRANSMIT_TYPE_MATRIX_UART_CTRL, true );
	}

	return 0;
}

int control_matrix_set_lcd_backlight_time( uint8_t* p_backlight_time, uint16_t pass_len )
{
	uint16_t send_len = 0;
	uint8_t backlight_time[3] = {0};

	if( pass_len != 3 || p_backlight_time == NULL )
		return -1;

	memcpy( backlight_time, p_backlight_time, pass_len );
	if( 0 == control_matrix_command_form( MATRIX_SET_LCD_BACKLIGHT_TIME, pass_len, backlight_time, &send_len ))// form success?
	{
		system_uart_packet_tx( gmatrix_command_buf, send_len, RUNINFLIGHT, TRANSMIT_TYPE_MATRIX_UART_CTRL, true );
	}

	return 0;
}

/*==============结束系统指令=====================================================*/

