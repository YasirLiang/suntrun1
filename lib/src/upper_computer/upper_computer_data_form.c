#include "upper_computer_data_form.h"

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


/***
*�����Ϸ���λ������д�뷢�͸���,дͷ,���ݳ��ȣ����ݣ�У���
*/
static size_t  conference_host_to_upper_computer_frame_write( void *base, struct host_to_endstation *p, uint16_t data_len, size_t pos, size_t len )
{
	ssize_t r = conference_validata_range( pos, data_len + OTHER_DATA_LENGHT, len );
	if(r > 0)
	{
		conference_common_header_write(p->cchdr, base, pos);
		conference_host_to_end_datalen_write(p->data_len, base, pos + CONFERENCE_COMMON_HEADER_LEN);
		conference_host_to_end_data_write(p->data, base, pos + CONFERENCE_COMMON_HEADER_LEN + 1, data_len);
		conference_host_to_end_crc_write(p->crc, base, pos + CONFERENCE_COMMON_HEADER_LEN + 1 + data_len);
	}

	return (size_t)r;
}

/**
*������У��λδ���ú�phost�������������Ѿ����úõ����
*
*/
int conference_host_to_upper_computer_form_msg( struct host_upper_cmpt_frame *frame,  struct host_upper_cmpt *phost )
{
	int upper_data_len = phost->common_header.data_len; // ���˹���ͷ���ݵĳ��ȣ�����������λ
	size_t cfc_dlgh = 0;

	// ���㲢����У��
	set_upper_cmpt_check( phost );

	// ��Э������д�븺��
	cfc_dlgh = conference_host_to_upper_computer_frame_write( frame->payload, phost, phost->common_header.data_len, UPPER_PAYLOAD_DATA_MAX_LEN );
}




