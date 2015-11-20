#ifndef __END_STATION_PRCS_H__
#define __END_STATION_PRCS_H__

#include "jdksavdecc_world.h"
#include "stream_descriptor.h"
#include "descriptor.h"

extern desc_pdblist descptor_guard;			// 系统中描述符链表哨兵节点

void proc_aecp_message_type_vendor_unique_command_conference( const uint8_t *frame, size_t frame_len, int *status );
int proc_rcvd_acmp_resp( uint32_t msg, const uint8_t *frame, size_t frame_len, int *status );
bool desc_index_from_frame(uint16_t desc_type, void *frame, ssize_t read_desc_offset, uint16_t *desc_index);
void queue_background_read_request(uint64_t target_id, uint16_t desc_type, uint16_t desc_base_index, uint16_t desc_count );
void background_read_deduce_next( desc_pdblist cd, uint16_t desc_type, void *frame, ssize_t read_desc_offset);
int proc_read_desc_resp( const uint8_t *frame, size_t frame_len, int *status);
void proc_rcvd_aem_resp( const uint8_t *frame, size_t frame_len, int *status );

void proc_rcvd_aecp_aa_resp( const uint8_t *frame, size_t frame_len, int *status );


#endif

