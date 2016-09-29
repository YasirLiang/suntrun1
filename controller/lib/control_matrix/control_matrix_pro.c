/*
**control_matrix_pro.c
**17-2-2016
**
**
*/

#include "control_matrix_pro.h"
#include "system_packet_tx.h"
#include "system_database.h"

#define MATRIX_COMMAND_BUF_SIZE 256
uint8_t gmatrix_command_buf[MATRIX_COMMAND_BUF_SIZE]; // 发送矩阵命令缓冲区 
uint8_t gmatrix_comand_system_flags = MATRIX_SET_CREATOR20;

/*设置矩阵ascii命令发送缓冲区的命令数据*/
int control_matrix_command_form( uint16_t matrix_command, uint16_t extra_len, uint8_t* p_extra_data, uint16_t *send_len, const uint8_t input_chn )
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
			case MATRIX_AV_SWITCH:
			case MATRIX_VIDEO_SWITCH:
			case MATRIX_AUDIO_SWITCH:
				if( (p_extra_data != NULL) && (extra_len != 0) && (extra_len <= MATRIX_INPUT_NUM))
				{
					int i = 0, j = 0;
					uint16_t total_len = 0;
					p_command_buf[0] = input_chn + 48;// make num to char ,like 1 to ''1''
					if( matrix_command == MATRIX_AV_SWITCH )
					{
						p_command_buf[1] = MATRIX_AV_SWICH_CMD;
					}
					else if( matrix_command == MATRIX_VIDEO_SWITCH )
					{
						p_command_buf[1] = MATRIX_V_SWICH_CMD;
					}
					else if( matrix_command == MATRIX_AUDIO_SWITCH )
					{
						p_command_buf[1] = MATRIX_A_SWICH_CMD;
					}
					else 
						break;

					for( i = 0; i < extra_len; i++ )
					{
						uint8_t out = p_extra_data[i];
						if( (out == 0) || ( out > MATRIX_OUTPUT_NUM))
						{
							DEBUG_INFO( "output switch out of range" );
							ret = -1;
							break;
						}
						
						p_command_buf[i+2+j] = out + 48;// make num to char ,like 1 to ''1''
						p_command_buf[i+3+j] = ',';
						j++;
						ret = 0;
					}

					if( ret != -1)
					{
						total_len = (extra_len*2+2);
						p_command_buf[total_len-1] = '.';// 改变最后一个为"."
						*send_len = total_len;
						ret = 0;
					}
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
		if( 0 == control_matrix_command_form( MATRIX_SET_OUTIN_CORRESPONDING, 0, NULL, &send_len, 0 ))// form success?
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
		if( 0 == control_matrix_command_form( MATRIX_CLOSE_ALL_OUTPUT_CHANNAL, 0, NULL, &send_len, 0 ))// form success?
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
		if( 0 == control_matrix_command_form( MATRIX_QUEUE_OUT_CHANNAL_INPUT_STATUS, 0, NULL, &send_len, 0 ))// form success?
		{
			system_uart_packet_tx( gmatrix_command_buf, send_len, RUNINFLIGHT, TRANSMIT_TYPE_MATRIX_UART_CTRL, true );
		}
	}

	return 0;
}

// 矩阵的av\video\audio切换
int control_matrix_input_output_switch( enum_matrix_command matrix_cmd, uint8_t in, uint8_t out[MATRIX_OUTPUT_NUM], uint8_t out_num )
{
	uint16_t send_len = 0;
	int ret = -1;

	assert( out );
	if( (NULL == out) || ((matrix_cmd != MATRIX_AV_SWITCH) \
		&& (matrix_cmd != MATRIX_VIDEO_SWITCH)\
		&& (matrix_cmd != MATRIX_AUDIO_SWITCH)))
	{
		return -1;
	}

	if( (in == 0) || (in > MATRIX_INPUT_NUM) \
		|| (out_num == 0) || (out_num > MATRIX_OUTPUT_NUM)\
		|| ((out_num != 0) &&(out[0] > MATRIX_OUTPUT_NUM)))
	{
		return -1;
	}
	
	if( gmatrix_comand_system_flags == MATRIX_SET_CREATOR20 )
	{
		
		if( 0 == control_matrix_command_form( matrix_cmd, out_num, out, &send_len, in ))// form success?
		{
			DEBUG_INFO( "form output input swich Success, send len = %d", send_len );
			system_uart_packet_tx( gmatrix_command_buf, send_len, RUNINFLIGHT, TRANSMIT_TYPE_MATRIX_UART_CTRL, true );
			ret = 0;
		}
		else 
			DEBUG_INFO( "form switch Error!" );
			
	}
	else if( gmatrix_comand_system_flags == MATRIX_SET_EXTRON )
	{
		if( (matrix_cmd != MATRIX_AV_SWITCH) && \
			(matrix_cmd != MATRIX_VIDEO_SWITCH) &&\
			(matrix_cmd != MATRIX_AUDIO_SWITCH))
		{
			DEBUG_INFO( "Error Matrix Switch command!" );
			ret = -1;
		}

		if( ret != -1 )
		{
			gmatrix_command_buf[0] = in+48;
			gmatrix_command_buf[1] = '*';
			gmatrix_command_buf[2] = out[0]+48;
			if( matrix_cmd == MATRIX_AV_SWITCH )
				gmatrix_command_buf[3] = '!';
			else if( matrix_cmd == MATRIX_VIDEO_SWITCH )
				gmatrix_command_buf[3] = '%';// or "&" 标志
			else if( matrix_cmd == MATRIX_AUDIO_SWITCH )
				gmatrix_command_buf[3] = '$';

			send_len = MATRIX_EXTRON_CMD_LEN;
			system_uart_packet_tx( gmatrix_command_buf, send_len, RUNINFLIGHT, TRANSMIT_TYPE_MATRIX_UART_CTRL, true );
		}
	}
	
	return ret;
}

// 已测试(22-2-2016)
bool paser_matrix_switch_cmd_string( char *p_str, uint8_t string_len, uint8_t *p_input, uint8_t* p_out, int *p_out_num )
{
	int i = 0;
	bool input_put = false;
	
	if( p_str == NULL ||  p_input == NULL || p_out == NULL || p_out_num == NULL || (string_len >MATRIX_COMMAND_BUF_SIZE))
		return false;

	for( i = 0; i < string_len; i++ )
	{
		if( !input_put )
		{
			if( ((p_str[0] > 48) && (p_str[0] < 58)) && ((p_str[1] == 'B') || (p_str[1] == 'V') || (p_str[1] == 'A')) )
			{
				*p_input = p_str[0] - 48;
				i = 2;
			}
			else if( ((p_str[1] > 48) && (p_str[1] < 58)) && ((p_str[0] -48) == 1) && \
				((p_str[2] == 'B') || (p_str[2] == 'V') || (p_str[2] == 'A')) )
			{
				*p_input = (p_str[0] - 48)*10 + (p_str[1] -48);
				i = 3;
			}
			else 
				return false;

			input_put = true;
		}

		if( (i >= 2) && ((p_str[i] > 48) && (p_str[i] < 58)) && ((p_str[i+1] == '.') || (p_str[i+1] == ',')) )
		{// parser out // 一位?
			if( *p_out_num < MATRIX_INPUT_NUM )
			{
				p_out[(*p_out_num)++] = p_str[i] - 48;
				i += 1;
			}
		}
		else if( (i >= 2) && ((p_str[i] > 48) && (p_str[i] < 58)) && ((p_str[i] -48) == 1) &&\
			((p_str[i+1] > 48) && (p_str[i+1] < 58)) && ((p_str[i+2] == ',') || (p_str[i+2] == '.')))// 两位?
		{
			if( *p_out_num < MATRIX_INPUT_NUM )
			{
				p_out[(*p_out_num)++] = (p_str[i] - 48)*10 + (p_str[i+1] -48);
				i += 2;
			}
		}
		else 
		{
			DEBUG_INFO( "inval form data switch comand!" );
			return false;
		}
	}

	return true;
}

// 矩阵的切换 暂时只支持MATRIX_COMMAND_BUF_SIZE
int control_matrix_switch( char *p_string, uint8_t string_len )
{
	uint16_t send_len = 0;
	int ret = -1, out_num = 0;
	uint8_t out[MATRIX_INPUT_NUM] = {0}, input = 0;
	char str[MATRIX_COMMAND_BUF_SIZE] = {0};
	enum matrix_switch_cmd sw_cmd;

	assert( p_string );
	if( NULL == p_string || string_len > MATRIX_COMMAND_BUF_SIZE )
	{
		return -1;
	}

	memset( gmatrix_command_buf, 0, MATRIX_COMMAND_BUF_SIZE );
	memcpy( gmatrix_command_buf, p_string, string_len );
	send_len = string_len;
	memcpy( str, p_string, string_len );
	
	if( str[1] == 'B' || str[2] == 'B' )
		sw_cmd = AV_SWITCH;
	else if( str[1] == 'V'  || str[2] == 'V')
		sw_cmd = V_SWITCH;
	else if( str[1] == 'A'  || str[2] == 'A')
		sw_cmd = A_SWITCH;
	else 
	{
		MSGINFO( "Err switch cmd = %c, check cmd is A B V\n", str[2] );
		return -1;
	}
	
	if( paser_matrix_switch_cmd_string( str, string_len, &input, out, &out_num ) )
	{
		int i = 0;
#ifdef __DEBUG__
		printf( " input = %d --out(num = %d):\t", input, out_num );
		for( i = 0 ; i < out_num; i++ )
		{
			printf( " %d", out[i] );
		}
		printf( " \n" );
#endif
		// 填充数据库
		gmatrix_io_swich_pro.input = input;
		if( -1 == system_database_matrix_io_queue(sw_cmd))
		{
			memset( &gmatrix_io_swich_pro, 0, sizeof(gmatrix_io_swich_pro) );
			for( i = 0 ; i < out_num; i++ )
			{
				if( out[i] <= MATRIX_OUTPUT_NUM )
					gmatrix_io_swich_pro.output_num[out[i]-1] = out[i]?1:0;
			}
			
			gmatrix_io_swich_pro.input = input;
			system_database_matrix_io_insert(sw_cmd);
		}
		else
		{
			memset( &gmatrix_io_swich_pro, 0, sizeof(gmatrix_io_swich_pro) );
			gmatrix_io_swich_pro.input = input;
			for( i = 0 ; i < out_num; i++ )
			{
				if( out[i] <= MATRIX_OUTPUT_NUM )
					gmatrix_io_swich_pro.output_num[out[i]-1] = out[i]?1:0;
			}
			
			if( -1 == system_database_matrix_io_update( sw_cmd ))
				DEBUG_INFO( "system_database_matrix_io_update Failed!" );
		}

		if( gmatrix_comand_system_flags == MATRIX_SET_CREATOR20 )
		{
			system_uart_packet_tx( gmatrix_command_buf, send_len, RUNINFLIGHT, TRANSMIT_TYPE_MATRIX_UART_CTRL, true );
			ret = 0;
		}
	}
	
	return ret;
}


/*==============结束指令系统=====================================================*/


/*==============开始兼容指令=====================================================*/
/*==============结束兼容指令=====================================================*/

/*==============开始系统指令=====================================================*/
int queue_control_matrix_type( void )
{
	uint16_t send_len = 0;
	
	if( 0 == control_matrix_command_form( MATRIX_QUEUE_TYPE, 0, NULL, &send_len, 0 ))// form success?
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
	if( 0 == control_matrix_command_form( MATRIX_CHANGE_KEY_PASSWORD, pass_len, pass_buf, &send_len, 0 ))// form success?
	{
		system_uart_packet_tx( gmatrix_command_buf, send_len, RUNINFLIGHT, TRANSMIT_TYPE_MATRIX_UART_CTRL, true );
	}

	return 0;
}

int control_matrix_lock_keyboard( void )
{
	uint16_t send_len = 0;
	
	if( 0 == control_matrix_command_form( MATRIX_LOCK_KEYBOARD, 0, NULL, &send_len, 0 ))// form success?
	{
		system_uart_packet_tx( gmatrix_command_buf, send_len, RUNINFLIGHT, TRANSMIT_TYPE_MATRIX_UART_CTRL, true );
	}

	return 0;
}

int control_matrix_unlock_keyboard( void )
{
	uint16_t send_len = 0;
	
	if( 0 == control_matrix_command_form( MATRIX_UNLOCK_KEYBOARD, 0, NULL, &send_len, 0 ))// form success?
	{
		system_uart_packet_tx( gmatrix_command_buf, send_len, RUNINFLIGHT, TRANSMIT_TYPE_MATRIX_UART_CTRL, true );
	}

	return 0;
}

int control_matrix_close_bell( void )
{
	uint16_t send_len = 0;
	
	if( 0 == control_matrix_command_form( MATRIX_CLOSE_BELL, 0, NULL, &send_len, 0 ))// form success?
	{
		system_uart_packet_tx( gmatrix_command_buf, send_len, RUNINFLIGHT, TRANSMIT_TYPE_MATRIX_UART_CTRL, true );
	}

	return 0;
}

int control_matrix_open_bell( void )
{
	uint16_t send_len = 0;
	
	if( 0 == control_matrix_command_form( MATRIX_OPEN_BELL, 0, NULL, &send_len, 0 ))// form success?
	{
		system_uart_packet_tx( gmatrix_command_buf, send_len, RUNINFLIGHT, TRANSMIT_TYPE_MATRIX_UART_CTRL, true );
	}

	return 0;
}

int contro_matrix_queue_program_version( void )
{
	uint16_t send_len = 0;
	
	if( 0 == control_matrix_command_form( MATRIX_QUEUE_VERSION, 0, NULL, &send_len, 0 ))// form success?
	{
		system_uart_packet_tx( gmatrix_command_buf, send_len, RUNINFLIGHT, TRANSMIT_TYPE_MATRIX_UART_CTRL, true );
	}

	return 0;
}

int control_matrix_set_extron( void )
{
	uint16_t send_len = 0;
	
	if( 0 == control_matrix_command_form( MATRIX_SET_EXTRON, 0, NULL, &send_len, 0 ))// form success?
	{
		system_uart_packet_tx( gmatrix_command_buf, send_len, RUNINFLIGHT, TRANSMIT_TYPE_MATRIX_UART_CTRL, true );
		gmatrix_comand_system_flags = MATRIX_SET_EXTRON;
	}

	return 0;
}

int control_matrix_set_creator20( void )
{
	uint16_t send_len = 0;
	
	if( 0 == control_matrix_command_form( MATRIX_SET_CREATOR20, 0, NULL, &send_len, 0 ))// form success?
	{
		system_uart_packet_tx( gmatrix_command_buf, send_len, RUNINFLIGHT, TRANSMIT_TYPE_MATRIX_UART_CTRL, true );
		gmatrix_comand_system_flags = MATRIX_SET_CREATOR20;
	}

	return 0;
}

int control_matrix_close_serial_msg_return( void )
{
	uint16_t send_len = 0;
	
	if( 0 == control_matrix_command_form( MATRIX_CLOSE_SERIAL_RETURN_MSG, 0, NULL, &send_len, 0 ))// form success?
	{
		system_uart_packet_tx( gmatrix_command_buf, send_len, RUNINFLIGHT, TRANSMIT_TYPE_MATRIX_UART_CTRL, true );
	}

	return 0;
}

int control_matrix_open_serial_msg_return( void )
{
	uint16_t send_len = 0;
	
	if( 0 == control_matrix_command_form( MATRIX_OPEN_SERIAL_RETURN_MSG, 0, NULL, &send_len,0 ))// form success?
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
	if( 0 == control_matrix_command_form( MATRIX_SET_LCD_BACKLIGHT_TIME, pass_len, backlight_time, &send_len, 0 ))// form success?
	{
		system_uart_packet_tx( gmatrix_command_buf, send_len, RUNINFLIGHT, TRANSMIT_TYPE_MATRIX_UART_CTRL, true );
	}

	return 0;
}

/*==============结束系统指令=====================================================*/

