#include "conference_end_to_host.h"

//��ȡ����
void conference_common_end_to_host_data_msg_read(uint8_t *d, const void *base, size_t offset, ssize_t data_len )
{
	uint8_t *p = ( ( uint8_t * )base ) + offset;

	int i = 0;
	for( i = 0; i < data_len; i++)
	{
		d[i] = p[i];
	}
}


//��ȡ����������Ӧ�����ݸ�ʽ
void conference_common_end_to_host_spe_data_read(uint8_t *d, const void *base, size_t offset,  ssize_t len_data)
{
	uint8_t *p = ((uint8_t *)base) + offset;
	ssize_t len = len_data;
	int i = 0;

	for( i = 0; i < len; i++)
	{
		d[i] = p[i];
	}
}

//�ж������Ƿ������������
static bool isnot_special_recv_command(const void *base,const size_t offset)
{
	uint8_t *p = ((uint8_t *)base) + offset;
	uint8_t cmd = p[1];

	if(( cmd & 0x1f ) == 0x1f )//����������0x1f
		return false;
	else
		return true;
}

/***
*�ն���ͨ�Ϸ��������,������Զ�������������ʽ
*����ֵ:
*	1:�������Ϸ������ʽ
*	0:������Ϸ������ʽ
*/
int conference_end_to_host_frame_read(const void *v_payload, struct endstation_to_host *phost, struct endstation_to_host_special *spephost,size_t pos, size_t buflen)
{
	assert( v_payload && phost && spephost );
	const uint8_t* payload = (uint8_t*)v_payload;
	
	ssize_t datalen = 0;
	if(((uint8_t)payload[0] == CONFERENCE_TYPE) && ( isnot_special_recv_command(payload, pos) ) )
	{
		conference_common_header_read( &phost->cchdr, payload, pos );
		conference_common_end_to_host_data_read( &phost->data, payload , pos + CONFERENCE_COMMON_HEADER_LEN);
		conference_common_end_to_host_crc_read( &phost->crc, payload , pos + CONFERENCE_COMMON_HEADER_LEN + 1);

		return 1;
	}
	else if( ((uint8_t)payload[0] == CONFERENCE_TYPE) && (!isnot_special_recv_command(payload, pos)))//���յ����������0x1f
	{
		//��ȡ���������е����ݳ����еĴ�С(������λ�ĳ���)
		 conference_common_header_read( &spephost->cchdr, payload, pos );
		 datalen = conference_end_to_host_spe_data_len_read(&spephost->data_len, payload, \
		 	pos + CONFERENCE_COMMON_HEADER_LEN );
		 conference_common_end_to_host_spe_data_read( spephost->data, payload,\
		 	pos + CONFERENCE_COMMON_HEADER_LEN + END_TO_HOST_DATA_IN_CMD_LEN, datalen  );
		 conference_common_end_to_host_crc_read( &spephost->crc, payload, \
		 	pos + CONFERENCE_COMMON_HEADER_LEN + END_TO_HOST_DATA_IN_CMD_LEN + datalen);

		 return 0;
	}

	return -1;
}

/********************************
*write:YasirLiang
*Date:2015/10/20
*Func:conference_end_to_host_deal_recv_msg_read
*param:
*		p: ��������ṹָ��
*		base: read frame
*		pos: ƫ��
*		buflen: �������Ĵ�С
*		data_len: ����֡�ĳ���
*return value:
*	-1 or read data lenght
*/
ssize_t conference_end_to_host_deal_recv_msg_read( ttmnl_recv_msg *p_tt, const void *base, uint16_t pos, size_t buflen, size_t data_len )
{
	assert( p_tt && base );
	ssize_t r = jdksavdecc_validate_range( pos, buflen, data_len );
	if( r >= 0 )
	{
		conference_common_header_read( &p_tt->cchdr, base, pos );
		if( isnot_special_recv_command( base, pos ) )
		{
			conference_common_end_to_host_data_read( p_tt->data, base , pos + CONFERENCE_COMMON_HEADER_LEN );
			conference_common_end_to_host_data_len_set( &p_tt->data_len, (data_len -CONFERENCE_COMMON_HEADER_LEN -CONFERENCE_CRC_LEN) );
		}
		else
		{
			conference_end_to_host_spe_data_len_read( &p_tt->data_len, base, pos + CONFERENCE_COMMON_HEADER_LEN );
			conference_common_end_to_host_spe_data_read( p_tt->data, base,\
		 	pos + CONFERENCE_COMMON_HEADER_LEN + END_TO_HOST_DATA_IN_CMD_LEN, p_tt->data_len );
		}
	}

	return r;
}

//�Ƿ����ն��Ϸ�, ��->�ն��Ϸ�;��->�����·�
bool is_end_to_host_cmd_send( uint8_t cmd ) 
{
	uint8_t c = cmd & 0x80;//bit7

	if( c >> 7 )
		return true;
	else
		return false;
}

//�Ƿ���������,��->���������ı��ģ���->��Ӧ�ı���
bool is_initiative_send( uint8_t cmd )
{
	uint8_t c = cmd & 0x40;//bit6
	
	if( c >> 6 )
		return true;
	else
		return false;
}

//�Ƿ���ϯ����, ��->����ϯ�����ı���;��->��ͨ�ն˷����ı���
bool is_president_send( uint8_t cmd )
{
	uint8_t c = cmd & 0x20;//bit5

	if( c >> 5 )
		return true;
	else
		return false;
}

//У�����ݵ���ȷ��,����ֻ�ܼ����ͨ���ݵ�
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

	//����У��
	check_crc |= ( byte_guide ^ command_control ^ address_high_eight ^ address_low_eight ^ data );
	printf("recv crc = %02x, check_crc = %02x\n", crc , check_crc );
	if( check_crc ==  crc)
		return true;
	else
		return false;
}

// ��ȡ��������֮ǰ������У��
bool check_conferece_deal_data_crc(uint16_t lng, const void *data, int pos)
{
	uint8_t crc = 0;
	int i = 0;
	uint8_t *p = NULL; 

	assert( data );
	p = (uint8_t*)data;
	for (i = 0; i < lng; i++)
	{
		crc ^= p[i];
	}

	if (crc == 0)
		return true;

	return false;
}

