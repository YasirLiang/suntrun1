/*
**matrix_output.h
**2-15-2016
*/

#ifndef __MATRIX_OUTPUT_H__
#define __MATRIX_OUTPUT_H__

#include "uart.h"
#define MATRIX_OUTPUT_FILE UART2

int matrix_output_transmit_uart_control_packet( uint8_t *frame, uint16_t frame_len, bool isresend, bool isresp, uint32_t *interval_time );
int matrix_output_init( void );

#endif

