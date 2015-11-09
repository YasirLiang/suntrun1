#ifndef __AECP_CONTROLLER_MACHINE_H__
#define __AECP_CONTROLLER_MACHINE_H__

#include "host_time.h"
#include "conference.h"
#include "linked_list_unit.h"
#include "system_packet_tx.h"
#include "inflight.h"
#include "raw.h"

void aecp_controller_init( solid_pdblist solid_guard_node, desc_pdblist desc_guard, inflight_plist inflight_guard );
int transmit_aecp_packet_network( uint8_t* frame, uint32_t frame_len, inflight_plist guard, bool resend, const uint8_t dest_mac[6], bool resp );
void aecp_inflight_station_timeouts( inflight_plist aecp_sta, inflight_plist hdr );
int aecp_read_desc_init(uint16_t desc_type, uint16_t desc_index, uint64_t target_entity_id);
int aecp_callback( uint32_t notification_flag, uint8_t *frame);
int aecp_proc_resp( struct jdksavdecc_frame *cmd_frame);
int aecp_state_rcvd_resp( struct jdksavdecc_frame *cmd_frame);
int aecp_update_inflight_for_rcvd_resp( uint32_t msg_type, bool u_field, struct jdksavdecc_frame *cmd_frame);
int aecp_send_read_desc_cmd_with_flag( uint16_t desc_type, uint16_t desc_index, uint64_t entity_id);
int aecp_send_read_desc_cmd( uint16_t desc_type, uint16_t desc_index, uint64_t target_entity_id);
int aecp_read_desc_init(uint16_t desc_type, uint16_t desc_index, uint64_t target_entity_id);
int  aecp_update_inflight_for_vendor_unique_message(uint32_t msg_type, const uint8_t *frame, size_t frame_len, int *status );



#endif

