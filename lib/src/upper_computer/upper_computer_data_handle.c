#include "upper_computer_data_handle.h"

int handle_pack_event( struct host_upper_cmpt *cnfrnc_pack )
{
	uint8_t loader_hdr = cnfrnc_pack->common_header.state_loader;
	uint8_t sub_type = cnfrnc_pack->common_header.deal_type;
	uint8_t cmd_type = cnfrnc_pack->common_header.command;
	uint16_t data_len = cnfrnc_pack->common_header.data_len;
	uint8_t data_payload[DATA_PAYLOAD_LEN_MAX];
	memcpy( data_payload, cnfrnc_pack->data_payload, data_len );

	DEBUG_INFO("loader_hdr = %02x sub_type = %02x cmd_type = %02x data_len = %02x", \
					loader_hdr, sub_type,cmd_type,data_len);
	if( loader_hdr == UPPER_COMPUTER_DATA_LOADER )
	{	
		// �Ƿ�Ϊ��λ����������, ����Ҫ��Ӧ�ͻ����������Ӧ��inflight���������������������Э�����������
		if( isupper_active_send( sub_type ) )
		{
			//handle_command_from_udp_client( cmd_type, data_len, data_payload );
		}
		else	// ��λ����Ӧ��������ʱ��������ֱ�ӷ���
		{
			DEBUG_INFO("There is nothing to do for response from upper computer");
		}
	}
	else
	{
		DEBUG_INFO( "wrong UDP state loader\n" );
		return -1;
	}

	return 0;
}

int handle_upper_computer_conference_data( struct host_upper_cmpt_frame * pframe )
{
	struct host_upper_cmpt_frame cpy_frame;
	struct host_upper_cmpt deal_unpack;
	
	memset( &cpy_frame, 0, sizeof( struct host_upper_cmpt_frame ));
	strcpy( (char*)cpy_frame.dest_address, (char*)pframe->dest_address );
	cpy_frame.dest_port = pframe->dest_port;
	memcpy( cpy_frame.payload, pframe->payload, pframe->payload_len );

	// ���У��
	if( check_crc( cpy_frame.payload, cpy_frame.payload_len ) )
	{
		// ���
		unpack_payload_from_udp_client( &deal_unpack, cpy_frame.payload, cpy_frame.payload_len, 0 );

		// ������յ�������
		handle_pack_event( &deal_unpack );
	}
	else	// ����Ӧ�ͻ���
	{
		return -1;
	}
	
	return 0;
}


