#ifndef __TERMINAL_COMMAND_H__
#define __TERMINAL_COMMAND_H__

#include "conference.h"
#include "jdksavdecc_world.h"
#include "terminal_pro.h"
#include "terminal_common.h"

extern terminal_address_list_pro allot_addr_pro;

void terminal_query_endstation( uint16_t addr, uint64_t entity_id );
void terminal_allot_address( void );
void terminal_reallot_address( void );
void terminal_state_set( tmnl_state_set tmnlstate, uint16_t addr,  uint64_t  target_id );
void terminal_set_mic_status( uint8_t data, uint16_t addr,  uint64_t  target_id );
void terminal_set_indicator_lamp( uint16_t data, uint16_t addr, uint64_t target_id );
void terminal_new_endstation_allot_address( uint64_t target_id );
void terminal_set_lcd_play_stype( uint64_t target_id, uint16_t addr, uint8_t lcd_stype );
void terminal_set_led_play_stype( uint64_t target_id, uint16_t addr, tmnl_led_state_show_set lcd_stype );
void terminal_chairman_control_meeting( uint64_t target_id, uint16_t addr, uint8_t data );
void terminal_send_vote_result( uint64_t target_id, uint16_t addr, tmnl_vote_result vote_rslt);
void terminal_limit_spk_time( uint64_t target_id, uint16_t addr, tmnl_limit_spk_time spk_time );
void terminal_host_send_state( uint64_t target_id, tmnl_main_state_send main_send );
void terminal_send_end_lcd_display( uint64_t target_id, uint16_t addr, tmnl_send_end_lcd_display lcd_dis);
void terminal_option_endpoint( uint64_t target_id, uint16_t addr, uint8_t opt );
void terminal_endstation_special_event_reply( uint64_t target_id, uint16_t addr );
void terminal_transmit_upper_cmpt_message( uint64_t target_id, uint16_t addr, uint8_t *msg, uint16_t msg_len);
void terminal_reply_end_message_command( uint64_t target_id, uint16_t addr );
void terminal_query_vote_sign_result( uint64_t target_id, uint16_t addr );


#endif

