#include "conference.h"
#include "conference_host_to_end.h"

void test_conf_printf(const void *pri_load, size_t load_len, char *msg)
{
	uint8_t *p = ( uint8_t * )pri_load;
	int i = 0;

	fprintf( stdout, "%s \t ",  msg);
	for( ; i < ( int )load_len; i++ )
		fprintf( stdout, "%02x  ", *(p + i) );
	fprintf( stdout, "\n " );
}

//���÷�������
static ssize_t set_data( uint8_t frame_data[], uint8_t rdata[], uint8_t ndata )
{
	int i = 0;
	ssize_t len = ( ssize_t )ndata;

	if( len == 0 )
	    return 0;
	else if( len > 0 )
	{
	    	for( ; i < ndata; i++ )
	    	{
	    	    frame_data[i] = rdata[i];
	    	}
	}

	return len;
}

//���������·���������֡��У��
static int set_frame_crc(struct host_to_endstation *alldata, ssize_t data_len)
{
	int i = 0;
		
	uint8_t crc = 0xff & 0x00;
	uint8_t byte_guide = 0xff & 0x00;
	uint8_t command_control = 0xff & 0x00;
	uint8_t address_high_eight = 0xff & 0x00;
	uint8_t address_low_eight = 0xff & 0x00;
	uint8_t data_bit_len = 0xff & 0x00;
	uint8_t data[DATAMAXLENGTH] ;

	byte_guide |= ( alldata->cchdr.byte_guide & 0xff );
	command_control |= ( alldata->cchdr.command_control & 0xff );
	address_high_eight |= ( alldata->cchdr.address & 0xff00 ) >> 8;
	address_low_eight |= ( alldata->cchdr.address & 0x00ff ) >> 0;
	data_bit_len |= ( alldata->data_len & 0xff );

	if(data_len == 0)
	{
		crc |= ( byte_guide^command_control^address_high_eight ^ address_low_eight ^data_bit_len );
		alldata->crc = crc;
	}
	else if(data_len > 0 && data_len <= DATAMAXLENGTH)
	{
		//��������λǰ���У��
		crc |= ( byte_guide^command_control^address_high_eight ^ address_low_eight^data_bit_len );
		
		for( i = 0; i < data_len; i++)
		{
			data[i] = 0xff & 0x00;
			data[i] |=  ( alldata->data[i] & 0xff );
			crc ^= data[i];
		}

		alldata->crc = crc;
	}
	else 
		return -1;
	
	return 0;
}

//д�����Э��Ĺ���ͷ��
void conference_common_header_write(const struct conference_common_header phdr,  void*base, size_t offset)
{
	uint8_t *p = ((uint8_t *)base) + offset;
	p[0] = phdr.byte_guide;
	p[1] = phdr.command_control;
	p[2] = (0xff00 & phdr.address) >> 8;//��ַ�߰�λ
	p[3] = (0x00ff & phdr.address) >> 0;//��ַ�Ͱ�λ
}

//д�����Э������ݳ���
static inline void conference_host_to_end_datalen_write(const uint8_t lendata, void*base, ssize_t offset)
{
	uint8_t *p = ((uint8_t *)base) + offset;
	p[0] = lendata;
}

//д�����Э������ݲ���
static inline void conference_host_to_end_data_write(const uint8_t data[], void*base, ssize_t offset, ssize_t len)
{
	uint8_t *p = ((uint8_t *)base) + offset;
	int i = 0;
	
	if( len == 0 )
		return;
	else if( len > 0 && len <= DATAMAXLENGTH )
	{
		for( i = 0; i < len; i++ )
			p[i] = data[i];	
	}
}

//д�����Э���У��
static inline void conference_host_to_end_crc_write(const uint8_t crc, void*base, ssize_t offset)
{
	uint8_t *p = ((uint8_t *)base) + offset;
	p[0] = crc; 
}

/***
*�����·�����д�뷢�͸���,дͷ,���ݳ��ȣ����ݣ�У��ͱ��ݵ�Э������
*/
static size_t  conference_host_to_end_frame_write(struct host_to_endstation *p,  
											void*base, 
											size_t pos, 
											ssize_t data_len,
											size_t len)
{
	ssize_t r = conference_validata_range(pos, data_len + HOST_COMMON_TO_END_EXDATA_LEN, len);
	if(r > 0)
	{
		conference_common_header_write(p->cchdr, base, pos);
		conference_host_to_end_datalen_write(p->data_len, base, pos + CONFERENCE_COMMON_HEADER_LEN);
		conference_host_to_end_data_write(p->data, base, pos + CONFERENCE_COMMON_HEADER_LEN + 1, data_len);
		conference_host_to_end_crc_write(p->crc, base, pos + CONFERENCE_COMMON_HEADER_LEN + 1 + data_len);
	}

	return (size_t)r;
}

/***
*��������:�����·����������ʽ������
*��������:
*	msg_type:���������
*	data_len:�������������ݲ��ֵĳ���
*	end_addr:�ն˵�ַ
*	hdata:���͵�����
*����ֵ: 
*	cfc_dlgh:Э���ڷ��͸����ϵĳ��ȣ���˫��Э�鳤��֮��
*/
int conference_host_to_end_form_msg(struct jdksavdecc_frame *frame, struct host_to_endstation *phost, 
										uint8_t msg_type,const ssize_t data_len,
										uint16_t end_addr,
										uint8_t hdata[])
{
	int r = 0;
	ssize_t ndata = -1;
	size_t cfc_dlgh = 0;

	//���÷������ݵ�Ŀ���ַ
	frame->dest_address = jdksavdecc_multicast_adp_acmp; 
	//���÷�����̫��Э������
	 frame->ethertype = JDKSAVDECC_AVTP_ETHERTYPE;
	
	//��������ͷ����ʽ
	phost->cchdr.byte_guide = CONFERENCE_TYPE;
	phost->cchdr.command_control = msg_type;
	phost->cchdr.address = end_addr;
	//�������ݳ���
	phost->data_len = data_len;
	//��������
	ndata = set_data(phost->data, hdata, phost->data_len);
	//����У��crc
	if(set_frame_crc(phost, ndata) == -1)
		DEBUG_INFO( "bad set crc " );		

	//�����úõ������ʽ���Ƶ����͸�����,���÷��͸��صĳ���
	cfc_dlgh = conference_host_to_end_frame_write(phost, frame->payload + \
							CONFERENCE_DATA_IN_CONTROLDATA_OFFSET, 0, data_len , sizeof(frame->payload));

	// ���ÿ���Э������ݣ������õ�������
	memcpy( phost->deal_backups, frame->payload + CONFERENCE_DATA_IN_CONTROLDATA_OFFSET, cfc_dlgh );
	memcpy( frame->payload + CONFERENCE_DATA_IN_CONTROLDATA_OFFSET + cfc_dlgh, phost->deal_backups, cfc_dlgh);
	cfc_dlgh += cfc_dlgh;
	frame->length = ( uint16_t )cfc_dlgh + 24;

	test_conf_printf( frame->payload + CONFERENCE_DATA_IN_CONTROLDATA_OFFSET, cfc_dlgh,  CONFERENCE_DATA_MSG);

	 return ( int )cfc_dlgh;
}

/***
*��������:�����·����������ʽ������
*��������:
*	msg_type:���������
*	data_len:�������������ݲ��ֵĳ���
*	end_addr:�ն˵�ַ
*	hdata:���͵�����
*����ֵ: 
*	cfc_dlgh:Э���ڷ��͸����ϵĳ��ȣ���˫��Э�鳤��֮��
*/
int conference_host_to_end_form_msg_cha(struct jdksavdecc_frame *frame, struct host_to_endstation *phost, ssize_t data_len )
{
	ssize_t ndata = -1;
	size_t cfc_dlgh = 0;

	//���÷������ݵ�Ŀ���ַ
	frame->dest_address = jdksavdecc_multicast_adp_acmp; 
	//���÷�����̫��Э������
	 frame->ethertype = JDKSAVDECC_AVTP_ETHERTYPE;	

	//�����úõ������ʽ���Ƶ����͸�����,���÷��͸��صĳ���
	cfc_dlgh = conference_host_to_end_frame_write(phost, frame->payload + \
							CONFERENCE_DATA_IN_CONTROLDATA_OFFSET, 0, data_len , sizeof(frame->payload));
	// ���ÿ���Э������ݣ������õ�������
	memcpy( frame->payload + CONFERENCE_DATA_IN_CONTROLDATA_OFFSET + cfc_dlgh, phost->deal_backups, cfc_dlgh);
	cfc_dlgh += cfc_dlgh;
	frame->length = ( uint16_t )cfc_dlgh + 24;

	test_conf_printf( frame->payload + CONFERENCE_DATA_IN_CONTROLDATA_OFFSET, cfc_dlgh,  CONFERENCE_DATA_MSG);

	 return ( int )cfc_dlgh;
}

//����1722Э�����ͷ��������������Ļ���ϵͳ��Э���ʽ�����һ�������Ƿ��͵�Э�����ݵĳ���
int conference_1722_control_form_info( struct jdksavdecc_frame *frame,
							                   struct jdksavdecc_aecpdu_aem *aemdu,
							                   struct jdksavdecc_eui48 destination_mac,
							                   struct jdksavdecc_eui64 target_entity_id,
							                   int cfc_dlgh )
{
	//����1722����ͷ
	frame->dest_address = destination_mac;
	frame->ethertype = JDKSAVDECC_AVTP_ETHERTYPE;

	aemdu->aecpdu_header.header.cd = 1;
	aemdu->aecpdu_header.header.subtype = JDKSAVDECC_SUBTYPE_AECP;
	aemdu->aecpdu_header.header.version = 0;
	aemdu->aecpdu_header.header.status = 0;
	aemdu->aecpdu_header.header.sv = 0;
	aemdu->aecpdu_header.header.control_data_length = JDKSAVDECC_AECPDU_AEM_LEN - JDKSAVDECC_COMMON_CONTROL_HEADER_LEN
	                                                      + cfc_dlgh;
	aemdu->aecpdu_header.header.message_type = JDKSAVDECC_AECP_MESSAGE_TYPE_VENDOR_UNIQUE_COMMAND;

	aemdu->aecpdu_header.controller_entity_id.value[0] = frame->src_address.value[0];
	aemdu->aecpdu_header.controller_entity_id.value[1] = frame->src_address.value[1];
	aemdu->aecpdu_header.controller_entity_id.value[2] = frame->src_address.value[2];
	aemdu->aecpdu_header.controller_entity_id.value[3] = 0xff;
	aemdu->aecpdu_header.controller_entity_id.value[4] = 0xfe;
	aemdu->aecpdu_header.controller_entity_id.value[5] = frame->src_address.value[3];
	aemdu->aecpdu_header.controller_entity_id.value[6] = frame->src_address.value[4];
	aemdu->aecpdu_header.controller_entity_id.value[7] = frame->src_address.value[5];

	aemdu->command_type = cfc_dlgh;
	aemdu->aecpdu_header.header.target_entity_id = target_entity_id;

	frame->length = jdksavdecc_aecpdu_aem_write( aemdu, frame->payload, 0, sizeof( frame->payload ) ) + cfc_dlgh;

	test_conf_printf( frame->payload , frame->length - cfc_dlgh,  CONFERENCE_CONTROL_DATA_MSG);
	test_conf_printf( frame->payload , frame->length ,  READY_SEND_FRAME_DATA);

	return ( int )frame->length;
}

//��������,��������ʹ�ö���ĺ꣬��������λhigh3bit���5λscmd
int conference_host_to_command_set(uint8_t *scmd, uint8_t *high3bit)
{
	if( (*scmd < 0x20) && (*high3bit < 0x8) )//����λ�����λ
	{
		*scmd = ((*high3bit & 0x7) << 5) |( (*scmd & 0x1f) << 0);
		return 0;
	}
	else 
		return -1;
}

//����Ӧ�õ�ַ
bool conference_host_to_end_address_application_set(uint16_t *endaddr, uint16_t *setaddr)
{
	if( ( *setaddr >> 12 ) == 0)
	{
		*endaddr &= 0x0000;
		*endaddr = ( *setaddr & 0x0fff) | ( CONFERENCE_ENDSTATION_ADDRESS_APPLICATION );
		
		return true;
	}
	else
		return false;
}

//�����豸��ַ
bool conference_host_to_end_address_device_set(uint16_t *endaddr, uint16_t *setaddr)
{
	if( ( *setaddr >> 12 ) == 0)
	{
		*endaddr &= 0x0000;
		*endaddr = ( *setaddr & 0x0fff) | ( CONFERENCE_ENDSTATION_ADDRESS_DEVICE );
		
		return true;
	}
	else
		return false;
}

/*******************
*����:���ù㲥��ַ,�����㲥�����͡��Ƿ��Ӧ����Ӧʱ�����ز���
*����:
*	endaddr:����ն˵�ַ
*	type_broadcast:�㲥�����ͣ�����ʹ�ú�
*CONFERENCE_ENDSTATION_ADDRESS_BROADCAST_ALL ( 0x40 & 0x7f )��
*CONFERENCE_ENDSTATION_ADDRESS_BROADCAST_COMMON ( 0x41 & 0x7f )��
*CONFERENCE_ENDSTATION_ADDRESS_BROADCAST_VIP ( 0x42 & 0x7f )  ��
*CONFERENCE_ENDSTATION_ADDRESS_BROADCAST_COMMON_PRESIDENT ( 0x44 & 0x7f )��
*CONFERENCE_ENDSTATION_ADDRESS_BROADCAST_EXECUTE_PRESIDENT (0x48 & 0x7f ) ,��Ϊ����
*	resp:0(����Ӧ)��1(��Ӧ)
*	num:Ϊ��ַbit5-bit0��ֵ,��Χ1-64
*	windflag:������Ӧʱ�������һ������,ռ��ַ��bit7��bit6:
			0��ӦwindΪ0x7f��0��ӦwindΪ0xff��0��ӦwindΪ0x1ff��0��ӦwindΪ0x3ff.
*	��Ӧ����Ӧʱ��ΪTime=((Addr&Wind)%Num)*12 ��ms��
*/
bool conference_host_to_end_address_allbroadcast_set(uint16_t *endaddr,
														uint16_t type_broadcast,
														bool resp, 
														uint8_t num, 
														uint8_t windflag)
{
	*endaddr &= 0x0000;
	
	if( num <= 64 )//ռ��λ
	{
		if( (((resp & 0xff) == 0 ) || ((resp & 0xff) == 1 )) && ( windflag < 4 ) &&
			((type_broadcast == 0x40) ||(type_broadcast == 0x41)||(type_broadcast == 0x42)||\
			(type_broadcast == 0x44) || (type_broadcast == 0x44)))
		{
			*endaddr |= ( (type_broadcast << 9) |( (resp & 0x1) << 8 ) | ( ( windflag & 0x3 ) << 6 ) | ((num & 0x3f) << 0));
			return true;
		}
		else
		{
			fprintf(stdout, "invalid resp or windflag or type_broadcast:%02x,%02x,%02x\n", resp, \
				windflag, type_broadcast);
			return false;
		}
	}

	fprintf(stdout, "%s():invalid num->%d\n", __func__, num);
	return false;
}

//�������ݳ���λ��ֵ
inline void conference_host_to_end_datalen_set(ssize_t* data_len, uint8_t dlgh)
{
	*data_len = (ssize_t)dlgh;
}

//��������λ������
void  conference_host_to_end_data_set(uint8_t *datum, uint8_t tmpdat[])
{
	int i = 0;
	int len= sizeof( tmpdat ) / ( sizeof(uint8_t) );

	for( i = 0; i < len; i++ )
	{
		datum[i] = tmpdat[i];
	}

	return;
}

