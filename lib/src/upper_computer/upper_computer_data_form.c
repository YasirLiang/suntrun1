#include "upper_computer_data_form.h"
#include "host_controller_debug.h"

// 设置校验
void set_upper_cmpt_check( struct host_upper_cmpt *p )
{
	uint8_t crc = 0;
	uint8_t data_len = p->common_header.data_len;
	uint8_t *p_data = p->data_payload;

	crc = p->common_header.state_loader + p->common_header.command \
		+ p->common_header.deal_type + p->common_header.data_len;
	if( data_len != 0 )
	{
		int i = 0;
		for( i = 0; i < data_len; i++)
			crc += p_data[i];
	}

	p->deal_crc = crc;
}

// 写主机与上位机协议公共头部
void host_upper_cmpt_common_header_write( const struct host_upper_cmpt_common *common_header, void* base, ssize_t pos )
{
	uint8_t *p = ((uint8_t *)base + pos);

	p[0] = common_header->state_loader;
	p[1] = common_header->deal_type;
	p[2] = common_header->command;
	p[3] = (uint8_t)((common_header->data_len & 0x00ff) >> 0);// 低字节在低位
	p[4] = (uint8_t)((common_header->data_len & 0xff00) >> 8);
}

// 写入数据
void host_upper_cmpt_data_write( const uint8_t *pdata, void* base, ssize_t pos, const uint16_t data_len )
{
	uint8_t *p = ((uint8_t *)base + pos);
	int i = 0;

	if( data_len > 0)
	{
		for( i = 0; i < data_len; i++ )
		{	
			p[i] = pdata[i];
		}
	}
}

// 写入校验
void host_upper_cmpt_end_crc_write( const uint8_t crc, void* base, ssize_t pos )
{
	uint8_t *p = ((uint8_t *)base + pos);

	p[0] = crc;
}

/***
*主机上发上位机命令写入发送负载,写头,数据长度，数据，校验和
*/
size_t  conference_host_to_upper_computer_frame_write( void *base, struct host_upper_cmpt *p, const uint16_t data_len, size_t pos, size_t buflen )
{
	ssize_t r = jdksavdecc_validate_range( pos, buflen, data_len + OTHER_DATA_LENGHT );
	if(r >= 0)
	{
		host_upper_cmpt_common_header_write( &p->common_header, base, pos );
		host_upper_cmpt_data_write( p->data_payload, base, pos + HOST_UPPER_COMPUTER_COMMON_LEN, data_len);
		host_upper_cmpt_end_crc_write( p->deal_crc, base, pos + HOST_UPPER_COMPUTER_COMMON_LEN + data_len);
	}

	return ( size_t )r;
}

/**
*适用于校验位未设置和phost的其它参数都已经设置好的情况
*
*/
int conference_host_to_upper_computer_form_msg( struct host_upper_cmpt_frame *frame,  struct host_upper_cmpt *phost )
{
	const uint16_t upper_data_len = phost->common_header.data_len; // 除了公共头数据的长度，不包括检验位

	// 计算并设置校验
	set_upper_cmpt_check( phost );
	// 把协议数据写入负载,并返回
	return (int)conference_host_to_upper_computer_frame_write( frame->payload, phost, upper_data_len, 0, sizeof(frame->payload) );
}

inline void conference_host_upper_computer_set_upper_message_form( tcmpt_data_message *data_msg, const void* base, uint16_t offset, uint16_t len )
{
	memcpy( data_msg, ((uint8_t*)base) + offset, len);
}


