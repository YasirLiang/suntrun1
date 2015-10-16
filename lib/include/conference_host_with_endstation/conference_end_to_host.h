#ifndef __CONFERENCE_END_TO_HOST_H__
#define __CONFERENCE_END_TO_HOST_H__

#include "conference.h"
#include "jdksavdecc_pdu.h"
#include "host_controller_debug.h"

inline uint8_t get_conference_guide_type( const void *base, ssize_t pos);
int conference_end_to_host_frame_read(const void *payload, struct endstation_to_host *phost, struct endstation_to_host_special *spephost,size_t pos, size_t buflen);
inline void conference_end_to_host_endstation_address_read(struct conference_end_address_euint16 *addr,struct conference_common_header *hdr);
bool is_end_to_host_cmd_send( uint8_t cmd );
bool is_initiative_send( uint8_t cmd );
bool is_president_send( uint8_t cmd );
bool check_conferece_deal_data_crc(uint16_t lng, const void *data, int pos);

#endif

