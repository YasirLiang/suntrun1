/*
**control_matrix_common.h
**17-2-2016
**
**
*/

#ifndef __CONTROL_MATRIX_COMMON_H__
#define __CONTROL_MATRIX_COMMON_H__

#include "ring_buf.h"

#define MATRIX_MSG_SAVE_FILE "matrix.log"

int control_matrix_common_recv_message_save( uint8_t *frame, uint16_t frame_len );
void control_matrix_common_recv_message_pro( void );// ;后期可能修改这个处理函数，或改为使用消息队列2016/2/17
bool control_matrix_recv_pro_byte_msg( uint8_t save_char );
void matrix_control_destroy( void );
void control_matrix_common_init( void );

#endif
