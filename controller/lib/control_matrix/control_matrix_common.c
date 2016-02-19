/*
**control_matrix_common.h
**17-2-2016
**
**
*/

#include "control_matrix_common.h"
#include "file_util.h"
#include "time_handle.h"
#include "host_controller_debug.h"

FILE* gmatrix_save_msg_fd = NULL;// 保存矩阵返回信息的文件描述符
tchar_ring_buf gmatrix_recv_buf_pro;// 矩阵环形接受缓冲区 处理参数
#define RECV_FROM_MATRIX_UART_BUF_SIZE 2048 // 矩阵环形接受缓冲区 大小
uint8_t gmatrix_recv_buf[RECV_FROM_MATRIX_UART_BUF_SIZE];//矩阵环形接受缓冲区

/*保存从串口接收到的数据*/
int control_matrix_common_recv_message_save( uint8_t *frame, uint16_t frame_len )
{// ;后期可能改为使用消息队列保存数据2016/2/17
	const uint16_t msg_len = frame_len;
	uint16_t point = 0;
	
	if( frame != NULL )
	{
		while( point < msg_len )
		{// save to ring buf
			char_ring_buf_save( &gmatrix_recv_buf_pro, frame[point++] );
		}
	}

	return 0;
}

/*************************************************
*Writer:	YasirLiang
*Date: 2016/2/17
*Name:control_matrix_common_recv_message_pro
*Func: get matrix common recv message ring buf data and proccess it
*Param:none 
*return value:none
***************************************************/
void control_matrix_common_recv_message_pro( void )// ;后期可能修改这个处理函数，或改为使用消息队列2016/2/17
{
	uint8_t ch_temp;
	bool ret = false;
	while( char_ring_buf_get( &gmatrix_recv_buf_pro, &ch_temp ))
	{
		ret = control_matrix_recv_pro_byte_msg( /*&gmatrix_recv_buf_pro,*/ ch_temp );
		if( !ret )
			DEBUG_INFO( "save msg Error!");
		
		over_time_set( MATRIX_RCV_GAP, 10 );
	}

	if( over_time_listen(MATRIX_RCV_GAP) )
	{
		over_time_stop(MATRIX_RCV_GAP);
	}
}

/*************************************************
*Writer:	YasirLiang
*Date: 2016/2/17
*Name:control_matrix_recv_pro_byte_msg
*Func: proccess byte recv matrix message:write byte data to file as so far(17-2-2017);后期可能修改
*Param:none 
*return value:none
***************************************************/
bool control_matrix_recv_pro_byte_msg( /*thost_upper_cmpt_msg *pmsg, */uint8_t save_char )
{
	bool bret = false;
	uint8_t save_ch = save_char;
	if( gmatrix_save_msg_fd != NULL )
	{
		if( Fwrite( gmatrix_save_msg_fd, &save_ch, 1, 1 ) == 1 );
		{
			Fflush( gmatrix_save_msg_fd );
			bret = true;
		}
	}

	return bret;
}

void control_matrix_common_init( void )
{
	gmatrix_recv_buf_pro.empty = true;
	gmatrix_recv_buf_pro.buf_size = RECV_FROM_MATRIX_UART_BUF_SIZE;
	gmatrix_recv_buf_pro.head = 0;
	gmatrix_recv_buf_pro.trail = 0;
	gmatrix_recv_buf_pro.pring_buf = gmatrix_recv_buf;

	gmatrix_save_msg_fd = Fopen( MATRIX_MSG_SAVE_FILE, "ab+" );
	if( gmatrix_save_msg_fd == NULL )
	{
		DEBUG_INFO( "open matrix save message file( %s ) is wrong!can't save any message", MATRIX_MSG_SAVE_FILE );
	}
}

void matrix_control_destroy( void )
{
	uint8_t ch_temp;
	while( char_ring_buf_get( &gmatrix_recv_buf_pro, &ch_temp ))
	{
		control_matrix_recv_pro_byte_msg( ch_temp );
	}
	
	Fclose( gmatrix_save_msg_fd );
}

