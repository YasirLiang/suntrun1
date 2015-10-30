#include "upper_computer_data_form.h"
#include "host_controller_debug.h"

// ����У��
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

// д��������λ��Э�鹫��ͷ��
void host_upper_cmpt_common_header_write( const struct host_upper_cmpt_common *common_header, void* base, ssize_t pos )
{
	uint8_t *p = ((uint8_t *)base + pos);

	p[0] = common_header->state_loader;
	p[1] = common_header->deal_type;
	p[2] = common_header->command;
	p[3] = (uint8_t)((common_header->data_len & 0x00ff) >> 0);// ���ֽ��ڵ�λ
	p[4] = (uint8_t)((common_header->data_len & 0xff00) >> 8);
}

// д������
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

// д��У��
void host_upper_cmpt_end_crc_write( const uint8_t crc, void* base, ssize_t pos )
{
	uint8_t *p = ((uint8_t *)base + pos);

	p[0] = crc;
}

/***
*�����Ϸ���λ������д�뷢�͸���,дͷ,���ݳ��ȣ����ݣ�У���
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
*������У��λδ���ú�phost�������������Ѿ����úõ����
*
*/
int conference_host_to_upper_computer_form_msg( struct host_upper_cmpt_frame *frame,  struct host_upper_cmpt *phost )
{
	const uint16_t upper_data_len = phost->common_header.data_len; // ���˹���ͷ���ݵĳ��ȣ�����������λ

	// ���㲢����У��
	set_upper_cmpt_check( phost );
	// ��Э������д�븺��,������
	return (int)conference_host_to_upper_computer_frame_write( frame->payload, phost, upper_data_len, 0, sizeof(frame->payload) );
}

inline void conference_host_upper_computer_set_upper_message_form( tcmpt_data_message *data_msg, const void* base, uint16_t offset, uint16_t len )
{
	memcpy( data_msg, ((uint8_t*)base) + offset, len);
}


