/*
**control_matrix_pro.h
**17-2-2016
**
**
*/

#ifndef __CONTROL_MATRIX_PRO_H__
#define __CONTROL_MATRIX_PRO_H__

#include "jdksavdecc_world.h"

int control_matrix_command_form( uint16_t matrix_command, uint16_t extra_len, uint8_t* p_extra_data, uint16_t *send_len );
int queue_control_matrix_type( void );
int control_change_keyboard_lock_password( uint8_t* p_password, uint16_t pass_len );
int control_matrix_lock_keyboard( void );
int control_matrix_unlock_keyboard( void );
int control_matrix_close_bell( void );
int control_matrix_open_bell( void );
int contro_matrix_queue_program_version( void );
int control_matrix_set_extron( void );
int control_matrix_set_creator20( void );
int control_matrix_close_serial_msg_return( void );
int control_matrix_open_serial_msg_return( void );
int control_matrix_set_lcd_backlight_time( uint8_t* p_backlight_time, uint16_t pass_len );
int control_matrix_set_outin_corresponding( void );
int control_matrix_close_all_out_channel( void );
int control_matrix_queue_input_status_for_out( void );


#endif
