/*uart_controller_machine.h
**Date:2015/11/25
**Description: the head file for uart_controller_machine.c
**
*/

#ifndef __UART_CONTROLLER_MACHINE_H__
#define __UART_CONTROLLER_MACHINE_H__

#include "jdksavdecc_world.h"
#include "host_controller_debug.h"

int transmit_camera_uart_control_packet( uint8_t *frame, uint16_t frame_len, bool isresend, bool isresp, uint32_t *interval_time );

#endif
