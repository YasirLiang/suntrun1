/*
**matrix_output.h
**2-15-2016
*/

#ifndef __MATRIX_OUTPUT_H__
#define __MATRIX_OUTPUT_H__

#include "uart.h"
#define MATRIX_OUTPUT_FILE UART1

#define MATRIX_INPUT_NUM 4 // 4�������
#define MATRIX_OUTPUT_NUM 4// 4�������

extern int gmatrix_output_file;	// �����������ļ�
extern bool gmatrix_file_set_success; // �����������ļ���ʼ���ɹ�

int matrix_output_transmit_uart_control_packet( uint8_t *frame, uint16_t frame_len, bool isresend, bool isresp, uint32_t *interval_time );
int matrix_output_init( void );

#endif

