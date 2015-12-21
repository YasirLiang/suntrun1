#ifndef __UPPER_COMPUTER_DATA_PARSER_H__
#define __UPPER_COMPUTER_DATA_PARSER_H__

#include "upper_computer.h"

#define UPPER_HOST_CONFERENCE_HEADER_TYPE_OFFSET 0
#define UPPER_HOST_CONFERENCE_DATA_PACKET_TYPE_OFFSET 1
#define UPPER_HOST_CONFERENCE_COMMAND_TYPE_OFFSET 2
#define UPPER_HOST_CONFERENCE_DATA_LEN_OFFSET 3

// ��ȡЭ������
static inline uint8_t get_host_upper_cmpt_guide_type( const void *base, size_t pos)
{
	uint8_t *p = ((uint8_t*)base) + pos;

	return (p[0]&0xff);
}

// ��ȡ��������
static inline uint8_t get_host_upper_cmpt_deal_type( const void *base, size_t pos )
{
	uint8_t *p = ((uint8_t*)base) + pos;

	return (p[1]&0xff);
}

// ��ȡ���ݳ���
static inline uint16_t get_host_upper_cmpt_data_len( const void *base, size_t pos )
{
	uint8_t *p = ((uint8_t*)base) + pos;

	return ((((uint16_t)p[3] & 0x00ff)<< 0) |(((uint16_t)p[4] & 0x00ff)<< 8));
}

// ��ȡЭ������
static inline uint8_t get_host_upper_cmpt_command_type(void *base, size_t pos)
{
	uint8_t *p = ((uint8_t*)base) + pos;

	return (p[2]&0xff);
}

// ����Ƿ�ָ������λ����������
static inline bool isupper_active_send( uint8_t subtype )
{
	if( ((subtype & 0x80) >> 7)  ==  0 )
		return true;
	else
		return false;
}

// ��ȡ����ͷ���������ݸ��صĳ���
static inline uint16_t get_host_upper_cmpt_common_header( struct host_upper_cmpt_common *hdr, const void *base, size_t pos )
{
	uint8_t *p = (( uint8_t * )base) + pos;
	hdr->state_loader = p[0] & 0xff;
	hdr->deal_type = p[1] & 0xff;
	hdr->command = p[2] & 0xff;
	hdr->data_len =  ((uint16_t)( (p[4] & 0x00ff) << 8) | (uint16_t)((p[3] & 0x00ff) << 0));	// ���ֽ�Ϊ�͵�ַ

	return hdr->data_len;
}

// ��ȡУ��
static inline void get_host_upper_cmpt_crc( uint8_t *crc, const void *base, size_t pos )
{
	uint8_t *p = (( uint8_t * )base) + pos;

	*crc = p[0];
}

bool check_crc( uint8_t *payload, int payload_len );
void get_host_upper_cmpt_data( void *data, const void *base, size_t pos, uint16_t data_len );
void unpack_payload_from_udp_client( struct host_upper_cmpt*outpack, const void* inpack, int pack_len, size_t pos );
 
#endif

