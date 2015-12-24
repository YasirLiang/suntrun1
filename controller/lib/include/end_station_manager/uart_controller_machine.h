/*uart_controller_machine.h
**Date:2015/11/25
**Description: the head file for uart_controller_machine.c
**
*/

#ifndef __UART_CONTROLLER_MACHINE_H__
#define __UART_CONTROLLER_MACHINE_H__

#include "jdksavdecc_world.h"
#include "host_controller_debug.h"

extern int uart_fd; // ´®¿ÚÎÄ¼şÃèÊö·û
extern bool uart_set_success;

int transmit_uart_control_packet_uart( uint8_t *frame, uint16_t frame_len, bool isresend, bool isresp, uint32_t *interval_time );


#endif
