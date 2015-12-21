#ifndef __CONFERENCE_END_TO_HOST_H__
#define __CONFERENCE_END_TO_HOST_H__

#include "conference.h"
#include "jdksavdecc_pdu.h"
#include "host_controller_debug.h"

//��ȡУ��
static inline void conference_common_end_to_host_crc_read(uint8_t *d, const void *base, size_t offset)
{
	uint8_t *p = ((uint8_t *)base) + offset;
	*d = p[0];
}

//��ȡ����
static inline void conference_common_end_to_host_data_read(uint8_t *d, const void *base, size_t offset)
{
	uint8_t *p = ( ( uint8_t * )base ) + offset;
	*d = p[0];
}

// ��ȡЭ������
static inline uint8_t conference_common_header_type_read(void *base, size_t offset)
{
	uint8_t *p = ((uint8_t *)base) + offset;
	
	return p[0];
}

//��ȡͷ����Ϣ
static inline void  conference_common_header_read(struct conference_common_header *hdr, const void *base, size_t offset)
{
	uint8_t *p = ((uint8_t *)base) + offset;
	hdr->byte_guide = p[0];
	hdr->command_control = p[1];
	hdr->address = ((p[2] & 0x00ff) << 0) | ((p[3] & 0x00ff) << 8);// ��ַ�ڸ����е��ֽ���ǰ
}

static inline ssize_t conference_end_to_host_spe_data_len_read(uint8_t *d, const void *base, const size_t offset)
{
	uint8_t *p = ((uint8_t *)base) + offset;
	*d = p[0];

	return (ssize_t)(*d);
}

static inline uint8_t get_conference_guide_type( const void *base, ssize_t pos)
{
	uint8_t *p = (( uint8_t * )base) + pos;

	return p[0];
}

static inline void conference_common_end_to_host_data_len_set( uint8_t *out_data_len, uint8_t input_data_len )
{
	*out_data_len = input_data_len;
}

//���ն˵�ַ
static inline void conference_end_to_host_endstation_address_read(struct conference_end_address_euint16 *addr,
													struct conference_common_header *hdr)
{
	assert( addr != NULL && hdr != NULL );
	memcpy( addr, &hdr->address,  sizeof(struct conference_end_address_euint16) );
}

void conference_common_end_to_host_data_msg_read(uint8_t *d, const void *base, size_t offset, ssize_t data_len );
int conference_end_to_host_frame_read(const void *payload, struct endstation_to_host *phost, struct endstation_to_host_special *spephost,size_t pos, size_t buflen);
bool is_end_to_host_cmd_send( uint8_t cmd );
bool is_initiative_send( uint8_t cmd );
bool is_president_send( uint8_t cmd );
bool check_conferece_deal_data_crc(uint16_t lng, const void *data, int pos);
ssize_t conference_end_to_host_deal_recv_msg_read( ttmnl_recv_msg *p_tt, const void *base, uint16_t pos, size_t buflen, size_t data_len );

#endif

