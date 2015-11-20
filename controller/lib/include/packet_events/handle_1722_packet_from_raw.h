#ifndef __HANDLE_1722_PACKET_FROM_RAW_H__
#define __HANDLE_1722_PACKET_FROM_RAW_H__

#include "conference_end_to_host.h"
#include "jdksavdecc_aecp_aem.h"
#include "jdksavdecc_util.h"
#include "conference.h"
#include "avdecc_net.h"
#include "entity.h"
#include "linked_list_unit.h"
#include "host_controller_debug.h"

int send_controller_avail_response( const uint8_t *frame, size_t frame_len, const uint8_t dst_mac[6], const uint8_t src_mac[6] );
int rx_raw_packet_event( const uint8_t dst_mac[6], const uint8_t src_mac[6], bool *is_notification_id_valid, solid_pdblist list_hdr, const uint8_t *frame, size_t frame_len, int *status, uint16_t operation_id, bool is_operation_id_valid );

#endif
