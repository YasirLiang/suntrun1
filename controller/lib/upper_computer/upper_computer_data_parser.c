#include "upper_computer_data_parser.h"

// 获取协议数据区数据
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


// 检查上位机与主机协议的校验
bool check_crc( uint8_t *payload, int payload_len )
{
	uint8_t *p = payload;
	uint8_t crc_sum = 0;
	int i = 0;

	//  校验是前面字节的累加, 负载的最后一位是校验位
	for( ; i < payload_len - 1 ; i++ )
	{
		crc_sum += p[i];		
	}

	if( crc_sum == p[payload_len -1] )
		return true;
	else
		return false;
}

// 将接受到的udp客户数据解析为会议系统的协议数据(上位机与会议主机)
void  unpack_payload_from_udp_client( struct host_upper_cmpt*outpack, const void* inpack, int pack_len, size_t pos )
{
	uint16_t data_size = 0;
	data_size = get_host_upper_cmpt_common_header( &outpack->common_header, inpack, pos );
	get_host_upper_cmpt_data( outpack->data_payload, inpack,\
									pos + HOST_UPPER_COMPUTER_COMMON_HEAD_LENGTH, data_size );
	get_host_upper_cmpt_crc( &outpack->deal_crc, inpack,\
							pos + HOST_UPPER_COMPUTER_COMMON_HEAD_LENGTH + data_size );
}

