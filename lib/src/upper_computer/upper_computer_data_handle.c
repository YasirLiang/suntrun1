#include "upper_computer_data_handle.h"
#include "upper_computer_common.h"

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
		// 是否为上位机主动发出, 是则要响应客户机后，清除相应的inflight命令，这里依据数据类型与协议的命令类型
		if( isupper_active_send( sub_type ) )
		{
			//handle_command_from_udp_client( cmd_type, data_len, data_payload );
		}
		else	// 上位机响应，主机暂时不做处理，直接返回
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
	int frame_len = pframe->payload_len;
	memset( &cpy_frame, 0, sizeof( struct host_upper_cmpt_frame ));
	
	cpy_frame.payload_len = frame_len;
	memcpy( cpy_frame.payload, pframe->payload, pframe->payload_len );

	if( !check_crc( cpy_frame.payload, frame_len))
		return -1;
	
	proccess_udp_client_msg_recv( cpy_frame.payload, cpy_frame.payload_len );
	
	return 0;
}

