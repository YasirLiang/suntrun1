#ifndef __UPPER_COMPUTER_DATA_PARSER_H__
#define __UPPER_COMPUTER_DATA_PARSER_H__

#include "upper_computer.h"

#define UPPER_HOST_CONFERENCE_HEADER_TYPE_OFFSET 0
#define UPPER_HOST_CONFERENCE_DATA_PACKET_TYPE_OFFSET 1
#define UPPER_HOST_CONFERENCE_COMMAND_TYPE_OFFSET 2
#define UPPER_HOST_CONFERENCE_DATA_LEN_OFFSET 3

inline bool isupper_active_send( uint8_t subtype );
bool check_crc( uint8_t *payload, int payload_len );
inline uint16_t get_host_upper_cmpt_common_header( struct host_upper_cmpt_common *hdr, const void *base, size_t pos );
inline void get_host_upper_cmpt_data( uint8_t *data, const void *base, size_t pos, uint16_t data_len );
inline void get_host_upper_cmpt_crc( uint8_t *crc, const void *base, size_t pos );
void unpack_payload_from_udp_client( struct host_upper_cmpt*outpack, const void* inpack, int pack_len, size_t pos );
inline uint8_t get_host_upper_cmpt_command_type(void *base, size_t pos);
inline uint8_t get_host_upper_cmpt_guide_type(const void *base, size_t pos);
void  unpack_payload_from_udp_client( struct host_upper_cmpt*outpack, const void* inpack, int pack_len, size_t pos );


#endif

