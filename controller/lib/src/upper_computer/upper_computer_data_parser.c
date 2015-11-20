#include "upper_computer_data_parser.h"

// ��ȡЭ������
inline uint8_t get_host_upper_cmpt_guide_type( const void *base, size_t pos)
{
	uint8_t *p = ((uint8_t*)base) + pos;

	return (p[0]&0xff);
}

// ��ȡ��������
inline uint8_t get_host_upper_cmpt_deal_type( const void *base, size_t pos )
{
	uint8_t *p = ((uint8_t*)base) + pos;

	return (p[1]&0xff);
}

// ��ȡ���ݳ���
inline uint16_t get_host_upper_cmpt_data_len( const void *base, size_t pos )
{
	uint8_t *p = ((uint8_t*)base) + pos;

	return ((((uint16_t)p[3] & 0x00ff)<< 0) |(((uint16_t)p[4] & 0x00ff)<< 8));
}

// ��ȡЭ������
inline uint8_t get_host_upper_cmpt_command_type(void *base, size_t pos)
{
	uint8_t *p = ((uint8_t*)base) + pos;

	return (p[2]&0xff);
}

// ����Ƿ�ָ������λ����������
inline bool isupper_active_send( uint8_t subtype )
{
	if( ((subtype & 0x80) >> 7)  ==  0 )
		return true;
	else
		return false;
}
// �����λ��������Э���У��
bool check_crc( uint8_t *payload, int payload_len )
{
	uint8_t *p = payload;
	uint8_t crc_sum = 0;
	int i = 0;

	//  У����ǰ���ֽڵ��ۼ�, ���ص����һλ��У��λ
	for( ; i < payload_len - 1 ; i++ )
	{
		crc_sum += p[i];		
	}

	if( crc_sum == p[payload_len -1] )
		return true;
	else
		return false;
}


// ��ȡ����ͷ���������ݸ��صĳ���
inline uint16_t get_host_upper_cmpt_common_header( struct host_upper_cmpt_common *hdr, const void *base, size_t pos )
{
	uint8_t *p = (( uint8_t * )base) + pos;
	hdr->state_loader = p[0] & 0xff;
	hdr->deal_type = p[1] & 0xff;
	hdr->command = p[2] & 0xff;
	hdr->data_len =  ((uint16_t)( (p[4] & 0x00ff) << 8) | (uint16_t)((p[3] & 0x00ff) << 0));	// ���ֽ�Ϊ�͵�ַ

	return hdr->data_len;
}

// ��ȡЭ������������
inline void get_host_upper_cmpt_data( void *data, const void *base, size_t pos, uint16_t data_len )
{
	assert( data && base );
	uint8_t *p = (( uint8_t * )base) + pos;
	uint8_t *out = (uint8_t*)data;
	int i = 0;

	for( ; i < ( int )data_len; i++ )
	{
		out[i] = p[i];
	}
}

// ��ȡУ��
inline void get_host_upper_cmpt_crc( uint8_t *crc, const void *base, size_t pos )
{
	uint8_t *p = (( uint8_t * )base) + pos;

	*crc = p[0];
}

// �����ܵ���udp�ͻ����ݽ���Ϊ����ϵͳ��Э������(��λ�����������)
void  unpack_payload_from_udp_client( struct host_upper_cmpt*outpack, const void* inpack, int pack_len, size_t pos )
{
	uint16_t data_size = 0;
	data_size = get_host_upper_cmpt_common_header( &outpack->common_header, inpack, pos );
	get_host_upper_cmpt_data( outpack->data_payload, inpack,\
									pos + HOST_UPPER_COMPUTER_COMMON_HEAD_LENGTH, data_size );
	get_host_upper_cmpt_crc( &outpack->deal_crc, inpack,\
							pos + HOST_UPPER_COMPUTER_COMMON_HEAD_LENGTH + data_size );
}


