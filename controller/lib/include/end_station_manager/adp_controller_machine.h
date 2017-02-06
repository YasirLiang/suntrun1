/*
* @file adp_controller_machine.h
* @brief adp protocol
* @ingroup adp protocol
* @cond
******************************************************************************
* Last updated for version 1.0.0
* Last updated on  2016-10-31
*
*                    Moltanisk Liang
*                    ---------------------------
*                    avb auto control system
*
* Copyright (C) Moltanisk Liang, GuangZhou Suntron. All rights reserved.
******************************************************************************
* @endcond
*/
#ifndef __ADP_CONTROLLER_MACHINE_H__
#define __ADP_CONTROLLER_MACHINE_H__
/*Including files-----------------------------------------------------------*/
#include "host_time.h"
#include "entity.h"
#include "raw.h"
#include "system_packet_tx.h"
/*! adp_entity_state_avail()------------------------------------------------*/
void adp_entity_state_avail(solid_pdblist guard, solid_pdblist exist_node,
    const struct jdksavdecc_adpdu_common_control_header *adp_hdr);
/*! adp_proccess_new_entity()-----------------------------------------------*/
solid_pdblist adp_proccess_new_entity(solid_pdblist guard,
    solid_pdblist* new_entity, const struct jdksavdecc_adpdu *src_du,
    const int list_len);
/*! transmit_adp_packet_to_net()--------------------------------------------*/
ssize_t transmit_adp_packet_to_net(uint8_t* frame, uint32_t frame_len,
    inflight_plist resend_node, bool resend, const uint8_t dest_mac[6],
    bool resp, uint32_t *interval_time );
/*! adp_entity_post_timeout_msr()-------------------------------------------*/
inline void adp_entity_post_timeout_msr(solid_pdblist target);
/*! adp_entity_time_tick()--------------------------------------------------*/
void adp_entity_time_tick(solid_pdblist guard);
/*! adp_entity_time_start()-------------------------------------------------*/
void adp_entity_time_start(timetype timeout, solid_pdblist new_adp_node);
/*! adp_entity_time_stop()--------------------------------------------------*/
void adp_entity_time_stop(solid_pdblist new_adp_node);
/*! adp_entity_time_update()------------------------------------------------*/
void adp_entity_time_update(timetype timeout, solid_pdblist new_adp_node);
/*! adp_entity_timeout()----------------------------------------------------*/
void adp_entity_timeout(solid_pdblist adp_node );
/*! adp_entity_avail()------------------------------------------------------*/
void adp_entity_avail(struct jdksavdecc_eui64 discover_entity_id,
    uint16_t msg_type);
/*! default_send_reboot_cmd()-----------------------------------------------*/
extern int default_send_reboot_cmd(uint64_t entity_entity_id,
    uint16_t desc_type, uint16_t desc_index);

#endif

