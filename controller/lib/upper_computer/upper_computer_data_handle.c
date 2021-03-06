#include "upper_computer_data_handle.h"
#include "upper_computer_common.h"

int handle_pack_event( struct host_upper_cmpt *cnfrnc_pack )
{
	uint8_t loader_hdr = cnfrnc_pack->common_header.state_loader;
	uint8_t sub_type = cnfrnc_pack->common_header.deal_type;
	uint16_t data_len = cnfrnc_pack->common_header.data_len;
	uint8_t data_payload[DATA_PAYLOAD_LEN_MAX];
	memcpy( data_payload, cnfrnc_pack->data_payload, data_len );

	if( loader_hdr == UPPER_COMPUTER_DATA_LOADER )
	{	
		// 是否为上位机主动发出, 是则要响应客户机后，清除相应的inflight命令，这里依据数据类型与协议的命令类型
		if( isupper_active_send( sub_type ) )
		{
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

int handle_upper_computer_conference_data( struct host_upper_cmpt_frame * pframe, int *status )
{
	assert( pframe && status );
	int frame_len = pframe->payload_len;
	
	if( pframe->payload[0] == UPPER_COMPUTER_DATA_LOADER )
	{	
		if( !check_crc( pframe->payload, frame_len) )
		{
			*status = 0;
			return -1;
		}
		else
		{
			proccess_udp_client_msg_recv( pframe->payload, frame_len, status );
		}	
	}
	
	return 0;
}

