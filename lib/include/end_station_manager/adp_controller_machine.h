#ifndef __ADP_CONTROLLER_MACHINE_H__
#define __ADP_CONTROLLER_MACHINE_H__

#include "host_time.h"
#include "entity.h"
#include "raw.h"
#include "system_packet_tx.h"

void adp_entity_state_avail( solid_pdblist guard,  solid_pdblist exist_node, const struct jdksavdecc_adpdu_common_control_header *adp_hdr );
solid_pdblist adp_proccess_new_entity( solid_pdblist guard, solid_pdblist* new_entity, const struct jdksavdecc_adpdu *src_du,const int list_len );
ssize_t transmit_adp_packet_to_net( uint8_t* frame,  uint32_t frame_len, inflight_plist guard, bool resend, const uint8_t dest_mac[6], bool resp );
inline void adp_entity_post_timeout_msr( solid_pdblist target );
void adp_entity_time_tick( solid_pdblist guard );
void adp_entity_time_start( timetype timeout, solid_pdblist new_adp_node );
void adp_entity_time_stop(solid_pdblist new_adp_node);
void adp_entity_time_update(timetype timeout, solid_pdblist new_adp_node);
void adp_entity_timeout( solid_pdblist adp_node );
void adp_entity_avail( struct jdksavdecc_eui64 discover_entity_id, uint16_t msg_type );

#endif

