#include "conference_end_to_host.h"

//��ȡУ��
inline static void conference_common_end_to_host_crc_read(uint8_t *d, const void *base, size_t offset)
{
	uint8_t *p = ((uint8_t *)base) + offset;
	*d = p[0];
}

//��ȡ����
inline static void conference_common_end_to_host_data_read(uint8_t *d, const void *base, size_t offset)
{
	uint8_t *p = ( ( uint8_t * )base ) + offset;
	*d = p[0];
}

//��ȡ����������Ӧ�����ݸ�ʽ
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

// ��ȡЭ������
inline uint8_t conference_common_header_type_read(void *base, size_t offset)
{
	uint8_t *p = ((uint8_t *)base) + offset;
	
	return p[0];
}

//��ȡͷ����Ϣ
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
	else if( r > 0 &&  ((uint8_t)payload[0] == CONFERENCE_TYPE) && (!isnot_special_recv_command(payload, pos)))//���յ����������0x1f
	{
		//��ȡ���������е����ݳ����еĴ�С(������λ�ĳ���)
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

//���ն˵�ַ
inline void conference_end_to_host_endstation_address_read(struct conference_end_address_euint16 *addr,
													struct conference_common_header *hdr)
{
	assert( addr != NULL && hdr != NULL );
	memcpy( addr, &hdr->address,  sizeof(struct conference_end_address_euint16) );
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
	assert( data );
	
	uint8_t *p = (( uint8_t * )data) + pos; 
	uint8_t recv_crc = p[lng - 1];
	uint8_t calculate_crc = p[0];
	int i = 0;

	// У��Ϊ����У��λ�����ݵ����(�������ն˵�ͨ��Э��)
	for( i = 1; i < lng -1 ; i++ )
	{
		calculate_crc ^= p[i] ;
	}

	if( recv_crc == calculate_crc )
		return true;
	else
		return false;
}



