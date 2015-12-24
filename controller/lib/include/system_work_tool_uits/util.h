#ifndef __UTIL_H__
#define __UTIL_H__

#include "jdksavdecc_world.h"
#include "host_controller_debug.h"
#include "enum.h"
#include "jdksavdecc_print.h"
#include "jdksavdecc_aecp_vendor.h"
#include "jdksavdecc_aecp_print.h"

#define GET_ARRAY_ELEM_NUM( array, elem_type ) (((sizeof(array)))/((sizeof(elem_type))))// get the num of a array

// 从buf中获取两个字节的数据，并返回
static inline uint16_t get_uint16_data_from_buf( void* base, uint16_t offset )
{
	uint8_t* p = (uint8_t*)base + offset;
	return (((uint16_t)((p[0]&0x00ff) << 0))|((uint16_t)((p[1]&0x00ff) << 8)));
}

// 从base中获取一个字节的数据，并返回
static inline uint8_t get_uint8_data_value_from_buf( void* base, uint16_t offset )
{
	uint8_t* p = (uint8_t*)base + offset;
	return p[0];
}

uint32_t get_udp_client_timeout_table(uint8_t msg_type );
void convert_eui48_to_uint64(const uint8_t value[6], uint64_t *new_value);
void convert_eui64_to_uint64(const uint8_t value [8], uint64_t * new_value);
uint32_t get_acmp_timeout(uint8_t msg_type );
bool is_conference_deal_data_response_type( void *base, uint16_t offset );
const char * acmp_cmd_value_to_name(uint32_t cmd_value);
const char* get_aem_desc_command_string( uint16_t desc_value );
const char * get_host_and_end_conference_string_value( uint8_t cmd );
const char* get_aem_command_string( uint16_t cmd_values );
uint8_t conference_command_type_read( void *base, uint16_t offerset);
void convert_entity_id_to_eui48_mac_address( uint64_t entity_id, uint8_t value[6] );
uint64_t convert_entity_id_to_uit64_mac_address( uint64_t entity_id );
void convert_str_to_eui64(char *p, uint8_t output[8] )	;// p 的格式0x----------------
uint64_t convert_eui64_to_uint64_return(const uint8_t value [8]);
bool is_mac_equal( const uint8_t mac_value1[6], const uint8_t mac_value2[6] );
bool get_adpdu_msg_type_value_with_str(const char* msg_str, uint16_t *msg_code );
const char * acmp_cmd_status_value_to_name( uint32_t acmp_cmd_status_value );
void convert_uint64_to_eui64( uint8_t dest_out[8], const uint64_t in );
const char * aem_cmd_value_to_name(uint16_t cmd_value);
const char *  aem_desc_value_to_name(uint16_t desc_value);
const char * aem_cmd_status_value_to_name(uint32_t aem_cmd_status_value);
const char* aecp_vendor_unique_status_value_to_name( uint32_t vendor_statue_value );
uint16_t conferenc_terminal_read_address_data( void *base, uint16_t offerset );
const char *upper_cmpt_cmd_value_to_string_name( uint8_t cmd_value );
bool get_upper_cmpt_cmd_value_from_string_name(const char* msg_str, uint32_t *msg_code );
bool is_terminal_command( void *base, uint16_t offset );
bool is_terminal_response( void *base, uint16_t offset );
uint32_t get_host_endstation_command_timeout( uint32_t cmd_value );
uint32_t get_send_respons_interval_timeout( uint8_t data_type, uint32_t command );


#endif

