#ifndef __ACMP_CONTROLLER_MACHINE_H__
#define __ACMP_CONTROLLER_MACHINE_H__

#include "host_time.h"
#include "inflight.h"
#include "linked_list_unit.h"
#include "system_packet_tx.h"
#include "raw.h"
#include "muticast_connector.h" // 广播连接表相关处理定义

void acmp_endstation_init( inflight_plist guard, solid_pdblist head, desc_pdblist desc_guard );
void acmp_frame_init( void );
void acmp_disconnect_avail( uint8_t output_id[8], uint16_t talker_unique_id, uint8_t input_id[8] , uint16_t listener_unique_id, uint16_t connection_count, uint16_t sequence_id);
void acmp_connect_avail(  uint8_t output_id[8], uint16_t talker_unique_id, uint8_t input_id[8] , uint16_t listener_unique_id, uint16_t connection_count, uint16_t sequence_id );
ssize_t transmit_acmp_packet_network( uint8_t* frame, uint16_t frame_len, inflight_plist resend_node, bool resend ,const uint8_t dest_mac[6], bool resp, uint32_t *interval_time );
void acmp_inflight_time_tick( inflight_plist inflight_list_guard );
void acmp_inflight_station_timeouts( inflight_plist acmp_sta, inflight_plist hdr );
int acmp_callback(  uint32_t notification_flag, uint8_t *frame, uint16_t frame_len );
//int acmp_callback(  uint32_t notification_flag, uint8_t *frame);
int acmp_proc_state_resp( struct jdksavdecc_frame *cmd_frame );
void acmp_update_output_stream_descriptor( uint16_t desc_index, struct jdksavdecc_acmpdu *acmpdu_get_rx_state_resp );
void acmp_update_input_stream_descriptor( uint16_t desc_index, struct jdksavdecc_acmpdu *acmpdu_get_rx_state_resp );
int acmp_update_endstation_connections_networks( void );
void acmp_tx_state_avail( uint64_t tarker_entity_id, uint16_t tarker_unique_id );
void acmp_rx_state_avail( uint64_t listener_entity_id, uint16_t listener_unique_id );
int proc_get_tx_state_resp( const uint8_t *frame, size_t frame_len,uint16_t desc_index, int *status);
int proc_get_rx_state_resp( const uint8_t *frame, size_t frame_len, uint16_t desc_index, int *status);
void acmp_binflight_cmd_time_tick( void );
//int acmp_disconnect_connect_table( uint8_t tarker_value[8], uint16_t tarker_index, uint8_t listener_value[8], uint16_t listener_index, uint16_t cnnt_count, uint16_t sequence_id, ttcnn_table_call *discnnt_callback_save , int (*disconnect_callback_func)( connect_tbl_pdblist p_cnnt_node ) );
//int acmp_connect_connect_table( uint8_t tarker_value[8], uint16_t tarker_index, uint8_t listener_value[8], uint16_t listener_index, uint16_t cnnt_count, uint16_t sequence_id, ttcnn_table_call *cnnt_callback_save , int (*connect_callback)( connect_tbl_pdblist p_cnnt_node, uint32_t timeouts, bool is_limit_time, uint64_t utarker_id ) );
int acmp_connect_connect_table( uint8_t tarker_value[8], 
									uint16_t tarker_index, 
									uint8_t listener_value[8], 
									uint16_t listener_index, 
									uint16_t cnnt_count, 
									uint16_t sequence_id, 
									ttcnn_table_call *cnnt_callback_save , 
									int (*connect_callback)( connect_tbl_pdblist p_cnnt_node, uint32_t timeouts, bool is_limit_time, uint64_t utarker_id ),
									tdisconnect_connect_mic_main_set *p_mic_main_set );
int acmp_disconnect_connect_table( uint8_t tarker_value[8], 
									uint16_t tarker_index, 
									uint8_t listener_value[8], 
									uint16_t listener_index, 
									uint16_t cnnt_count, 
									uint16_t sequence_id, 
									ttcnn_table_call *discnnt_callback_save , 
									int (*disconnect_callback_func)( connect_tbl_pdblist p_cnnt_node ),
									tdisconnect_connect_mic_main_set *p_mic_main_set );
void acmp_connect_muticastor_conventioner( uint64_t tarker_id, 
			uint16_t tarker_index,
			uint64_t listener_id, 
			uint16_t listener_index,
			muticast_offline_callback connect_callback, 
			conventioner_cnnt_list_node* connect_node,
			uint16_t sequence_id,
			uint16_t count );
void acmp_update_muticastor_conventioner( uint64_t tarker_id, 
			uint16_t tarker_index,
			uint64_t listener_id, 
			uint16_t listener_index,
			muticast_online_callback proccess_online_callback, 
			conventioner_cnnt_list_node* connect_node,
			uint16_t sequence_id,
			uint16_t count );

#endif

