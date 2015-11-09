#ifndef __STREAM_DESCRIPTOR_H__
#define __STREAM_DESCRIPTOR_H__

#include "jdksavdecc_world.h"
#include "descriptor.h"

int proc_get_tx_state_resp( const uint8_t *frame, size_t frame_len,uint16_t desc_index, int *status);
int proc_get_rx_state_resp( const uint8_t *frame, size_t frame_len, uint16_t desc_index, int *status);
int proc_connect_rx_resp( const uint8_t *frame, size_t frame_len, int *status);
int proc_disconnect_rx_resp( const uint8_t *frame, size_t frame_len, int *status);
int proc_get_tx_connection_resp( const uint8_t *frame, size_t frame_len, int *status);
int background_read_descriptor_input_output_stream( void );
void store_stream_output_desc( const uint8_t *frame, int pos, size_t frame_len, desc_pdblist  desc_info_node );
void store_stream_input_desc( const uint8_t *frame, int pos, size_t frame_len, desc_pdblist  desc_info_node );
int background_read_descriptor_input_output_stream( void );


#endif
