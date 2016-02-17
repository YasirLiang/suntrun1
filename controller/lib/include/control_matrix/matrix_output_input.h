/*
**matrix_output.h
**2-15-2016
*/

#ifndef __MATRIX_OUTPUT_H__
#define __MATRIX_OUTPUT_H__

#include "uart.h"
#define MATRIX_OUTPUT_FILE UART1

#define MATRIX_INPUT_NUM 4 // 4输入矩阵
#define MATRIX_OUTPUT_NUM 4// 4输出矩阵

extern int gmatrix_output_file;	// 矩阵控制输出文件
extern bool gmatrix_file_set_success; // 矩阵控制输出文件初始化成功

int matrix_output_transmit_uart_control_packet( uint8_t *frame, uint16_t frame_len, bool isresend, bool isresp, uint32_t *interval_time );
int matrix_output_init( void );

#endif

