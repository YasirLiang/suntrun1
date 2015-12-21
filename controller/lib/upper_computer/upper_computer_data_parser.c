#include "upper_computer_data_parser.h"

// ��ȡЭ������������
void get_host_upper_cmpt_data( void *data, const void *base, size_t pos, uint16_t data_len )
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

