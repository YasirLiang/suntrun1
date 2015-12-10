#ifndef __CAMERA_COMMON_H__
#define __CAMERA_COMMON_H__

#include "jdksavdecc_world.h"
#include "host_controller_debug.h"
#include "pelco_d.h"
#include "uart.h"

#define CAMERA_UART_FILE "/dev/ttyS1" // �����豸�ļ�

int camera_form_can_send( uint8_t camera_address, uint16_t d_cmd, uint8_t speed_lv, uint8_t speed_vertical );
int camera_common_control_init( void );
int camera_common_control_destroy( void );


#endif
