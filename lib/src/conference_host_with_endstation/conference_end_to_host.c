#include "conference_end_to_host.h"

//读取校验
inline static void conference_common_end_to_host_crc_read(uint8_t *d, const void *base, size_t offset)
{
	uint8_t *p = ((uint8_t *)base) + offset;
	*d = p[0];
}

//读取数据
inline static void conference_common_end_to_host_data_read(uint8_t *d, const void *base, size_t offset)
{
	uint8_t *p = ( ( uint8_t * )base ) + offset;
	*d = p[0];
}

//读取特殊命令响应的数据格式
inline static void conference_common_end_to_host_spe_data_read(uint8_t *d, const void *base, size_t offset,  ssize_t len_data)
{
	uint8_t *p = ((uint8_t *)base) + offset;
	ssize_t len = len_data;
	int i = 0;

	for( i = 0; i < len; i++)
	{
		d[i] = p[i];
	}
}

// 读取协议类型
inline uint8_t conference_common_header_type_read(void *base, size_t offset)
{
	uint8_t *p = ((uint8_t *)base) + offset;
	
	return p[0];
}

//读取头部信息
inline static void  conference_common_header_read(struct conference_common_header *hdr, const void *base, size_t offset)
{
	uint8_t *p = ((uint8_t *)base) + offset;
	hdr->byte_guide = p[0];
	hdr->command_control = p[1];
	hdr->address = ((p[2] & 0x00ff) << 8) | ((p[3] & 0x00ff) << 0);
}

inline static ssize_t conference_end_to_host_spe_data_len_read(uint8_t *d, const void *base, const size_t offset)
{
	uint8_t *p = ((uint8_t *)base) + offset;
	*d = p[0];

	return (ssize_t)(*d);
}

inline uint8_t get_conference_guide_type( const void *base, ssize_t pos)
{
	uint8_t *p = (( uint8_t * )base) + pos;

	return p[0];
}

//判断主机是否接收特殊命令
static bool isnot_special_recv_command(const void *base,const size_t offset)
{
	uint8_t *p = ((uint8_t *)base) + offset;
	uint8_t cmd = p[1];

	if(( cmd & 0x1f ) == 0x1f )//是特殊命令0x1f
		return false;
	else
		return true;
}

/***
*终端普通上发命令读入,这里可以读入特殊的命令格式
*返回值:
*	1:正常的上发命令格式
*	0:特殊的上发命令格式
*/
int conference_end_to_host_frame_read(const uint8_t *payload, struct endstation_to_host *phost, struct endstation_to_host_special *spephost,size_t pos, size_t buflen)
{
	assert( payload && phost && spephost );
	
	ssize_t datalen = 0;
	ssize_t r = conference_validata_range(pos, END_TO_HOST_CMD_LEN , buflen);
	if((r > 0) &&  ((uint8_t)payload[0] == CONFERENCE_TYPE) && ( isnot_special_recv_command(payload, pos) ) )
	{
		conference_common_header_read( &phost->cchdr, payload, pos );
		conference_common_end_to_host_data_read( &phost->data, payload , pos + CONFERENCE_COMMON_HEADER_LEN);
		conference_common_end_to_host_crc_read( &phost->crc, payload , pos + CONFERENCE_COMMON_HEADER_LEN + 1);

		return 1;
	}
	else if( r > 0 &&  ((uint8_t)payload[0] == CONFERENCE_TYPE) && (!isnot_special_recv_command(payload, pos)))//接收到特殊的命令0x1f
	{
		//获取特殊命令中的数据长度中的大小(即数据位的长度)
		 conference_common_header_read( &spephost->cchdr, payload, pos );
		 datalen = conference_end_to_host_spe_data_len_read(&spephost->data_len, payload, \
		 	pos + CONFERENCE_COMMON_HEADER_LEN );
		 conference_common_end_to_host_spe_data_read( spephost->data, payload,\
		 	pos + CONFERENCE_COMMON_HEADER_LEN + END_TO_HOST_DATA_IN_CMD_LEN, datalen);
		 conference_common_end_to_host_crc_read( &spephost->crc, payload, \
		 	pos + CONFERENCE_COMMON_HEADER_LEN + END_TO_HOST_DATA_IN_CMD_LEN + datalen);

		 return 0;
	}
	else 
		return -1;
}

//读终端地址
inline void conference_end_to_host_endstation_address_read(struct conference_end_address_euint16 *addr,
													struct conference_common_header *hdr)
{
	assert( addr != NULL && hdr != NULL );
	memcpy( addr, &hdr->address,  sizeof(struct conference_end_address_euint16) );
}

//是否是终端上发, 是->终端上发;否->主机下发
bool is_end_to_host_cmd_send( uint8_t cmd ) 
{
	uint8_t c = cmd & 0x80;//bit7

	if( c >> 7 )
		return true;
	else
		return false;
}

//是否主动发出,是->主动发出的报文；否->响应的报文
bool is_initiative_send( uint8_t cmd )
{
	uint8_t c = cmd & 0x40;//bit6
	
	if( c >> 6 )
		return true;
	else
		return false;
}

//是否主席发出, 是->是主席发出的报文;否->普通终端发出的报文
bool is_president_send( uint8_t cmd )
{
	uint8_t c = cmd & 0x20;//bit5

	if( c >> 5 )
		return true;
	else
		return false;
}

//校验数据的正确性,这里只能检查普通数据的
bool conference_end_to_host_crc_is_right(const struct endstation_to_host pend/* size_t pos, size_t buflen*/)
{
	uint8_t check_crc = 0xff & 0x00;
	
	uint8_t byte_guide = 0xff & 0x00;
	uint8_t command_control = 0xff & 0x00;
	uint8_t address_high_eight = 0xff & 0x00;
	uint8_t address_low_eight = 0xff & 0x00;
	uint8_t data = 0xff & 0x00;
	uint8_t crc = 0xff & 0x00;
	
	byte_guide |= ( pend.cchdr.byte_guide & 0xff );
	command_control |= ( pend.cchdr.command_control & 0xff );
	address_high_eight |= (uint8_t)( pend.cchdr.address & 0xff00 ) >> 8;
	address_low_eight |= (uint8_t)( pend.cchdr.address & 0x00ff ) >> 0;
	data |= ( pend.data & 0xff );
	crc |= ( pend.crc & 0xff );

	//计算校验
	check_crc |= ( byte_guide ^ command_control ^ address_high_eight ^ address_low_eight ^ data );
	printf("recv crc = %02x, check_crc = %02x\n", crc , check_crc );
	if( check_crc ==  crc)
		return true;
	else
		return false;
}

// 读取会议数据之前的数据校验
bool check_conferece_deal_data_crc(uint16_t lng, const void *data, int pos)
{
	assert( data );
	
	uint8_t *p = (( uint8_t * )data) + pos; 
	uint8_t recv_crc = p[lng - 1];
	uint8_t calculate_crc = p[0];
	int i = 0;

	// 校验为除了校验位的数据的异或(主机与终端的通信协议)
	for( i = 1; i < lng -1 ; i++ )
	{
		calculate_crc ^= p[i] ;
	}

	if( recv_crc == calculate_crc )
		return true;
	else
		return false;
}



