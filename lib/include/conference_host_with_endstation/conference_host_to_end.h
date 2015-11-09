#ifndef __CONFERENCE_HOST_TO_END_H__
#define __CONFERENCE_HOST_TO_END_H__

#include "jdksavdecc_pdu.h"
#include "jdksavdecc_aecp_aem.h"
#include "conference.h"

#define CONFERENCE_DATA_MSG "Conference Data:"
#define CONFERENCE_CONTROL_DATA_MSG "Conference Control Header:"
#define READY_SEND_FRAME_DATA "Ready Send Frame:"

void conference_common_header_write(const struct conference_common_header phdr,  void*base, size_t offset);
int conference_host_to_end_form_msg(struct jdksavdecc_frame *frame, struct host_to_endstation *phost,uint8_t msg_type,const ssize_t data_len,uint16_t end_addr,	uint8_t hdata[]);
int conference_host_to_command_set(uint8_t *scmd, uint8_t *high3bit);
bool conference_host_to_end_address_application_set(uint16_t *endaddr, uint16_t *setaddr);
bool conference_host_to_end_address_device_set(uint16_t *endaddr, uint16_t *setaddr);
bool conference_host_to_end_address_allbroadcast_set(uint16_t *endaddr,	uint16_t type_broadcast,bool resp,uint8_t num, uint8_t windflag);
inline void conference_host_to_end_datalen_set(ssize_t* data_len, uint8_t dlgh);
void  conference_host_to_end_data_set(uint8_t *datum, uint8_t tmpdat[]);
int conference_1722_control_form_info( struct jdksavdecc_frame *frame, struct jdksavdecc_aecpdu_aem *aemdu, struct jdksavdecc_eui48 destination_mac, struct jdksavdecc_eui64 target_entity_id,uint16_t cfc_dlgh );

#endif

