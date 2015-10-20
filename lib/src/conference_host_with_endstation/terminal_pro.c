/**
*terminal_pro.c
*
*proccess meeting
*/

#include "terminal_pro.h"
#include "linked_list_unit.h"
#include "host_controller_debug.h"
#include "conference_host_to_end.h"
#include "system_packet_tx.h"
#include "inflight.h"
#include "aecp_controller_machine.h"

terminal_address_list tmnl_addr_list[SYSTEM_TMNL_MAX_NUM];	// 终端地址分配列表
terminal_address_list_pro allot_addr_pro;	
tmnl_pdblist dev_terminal_list_guard = NULL; // 终端链表表头结点

void init_terminal_address_list( void )
{
	int i = 0;

	memset( tmnl_addr_list, 0, sizeof(tmnl_addr_list) );
	for( i = 0; i < SYSTEM_TMNL_MAX_NUM; i++ )		// 地址初始化为0xff
	{
		tmnl_addr_list[i].addr = 0xffff;
	}
}

void init_terminal_proccess_system( void )
{
	// init terminal address list
	allot_addr_pro.addr_start = 0;
	allot_addr_pro.index = 0;
	allot_addr_pro.renew_flag= 0;
	init_terminal_address_list();

	// init terminal system double list
	init_terminal_dblist( &dev_terminal_list_guard );
	assert( dev_terminal_list_guard != NULL );
}

// send terminal conference deal message in 1722 frame payload by pipe
uint16_t ternminal_send( void *buf, uint16_t length, uint64_t uint64_target_id, bool is_resp_data )
{
	struct host_to_endstation *data_buf = (struct host_to_endstation*)buf;
	struct host_to_endstation fill_send_buf;
	struct jdksavdecc_frame send_frame;
	struct jdksavdecc_aecpdu_aem aemdu;
	struct jdksavdecc_eui64 target_id;
	int send_len = 0;
	int cnf_data_len = 0;

	memcpy( send_frame.src_address.value, net.m_my_mac, 6 );
	convert_uint64_to_eui64( target_id.value, uint64_target_id );
	cnf_data_len = conference_host_to_end_form_msg( &send_frame, &fill_send_buf, data_buf->cchdr.command_control, data_buf->data_len, data_buf->cchdr.address, data_buf->data );
	send_len = conference_1722_control_form_info( &send_frame, &aemdu, jdksavdecc_multicast_adp_acmp, target_id, cnf_data_len );
	if( send_len < 0 )
	{
		DEBUG_INFO( "send len is bad! send_len = %d", send_len );
		assert( send_len >= 0 );
	}

	system_raw_packet_tx( send_frame.dest_address.value, send_frame.payload, send_len, RUNINFLIGHT, TRANSMIT_TYPE_AECP, is_resp_data );
	aecp_callback( CMD_WITH_NOTIFICATION, send_frame.payload );
	
	return (uint16_t)send_len;
}

// proccess recv conference deal message from raw network
void terminal_recv_message_pro( struct terminal_deal_frame *conference_frame )
{
	assert( NULL != conference_frame );
	uint16_t frame_len = conference_frame->payload_len/2;
	uint8_t *p_right_data = NULL;


	// check the crc of the both data backups,if crc is wrong,return directory
	if( check_conferece_deal_data_crc( frame_len, conference_frame->payload, ZERO_OFFSET_IN_PAYLOAD))
	{	
		p_right_data = conference_frame->payload;
	}
	else
	{
		if( check_conferece_deal_data_crc( frame_len, conference_frame->payload + frame_len, ZERO_OFFSET_IN_PAYLOAD))
			p_right_data = conference_frame->payload + frame_len;
		else	
			return;
	}
	
	DEBUG_RECV( p_right_data, frame_len, "Recv Right Conference Data" );
	ttmnl_recv_msg recv_data;
	ssize_t ret = 0;
	ret = conference_end_to_host_deal_recv_msg_read( &recv_data, p_right_data, ZERO_OFFSET_IN_PAYLOAD, (TERMINAL_MESSAGE_MAX_LEN + HOST_COMMON_TO_END_EXDATA_LEN)*2, frame_len);
	if( ret < 0 )
	{
		DEBUG_INFO( "Err recv conference data read" );
		assert( ret >=0 );
	}

	if( !(recv_data.cchdr.command_control & COMMAND_FROM_TMN) ) // is not terminal command
	{
		return; 
	}

	if( recv_data.cchdr.command_control & COMMAND_TMN_REPLY ) // proccess response data
	{
		if((recv_data.cchdr.command_control & COMMAND_TMN_MASK)== QUERY_END)
		{
			terminal_register( recv_data.cchdr.address, recv_data.data[0] );
		}
		else if((recv_data.cchdr.command_control & COMMAND_TMN_MASK)== SET_END_STATUS )
		{
			terminal_type_save( recv_data.cchdr.address, recv_data.data[0],((recv_data.cchdr.command_control&COMMAND_TMN_CHAIRMAN)?true:false));
		}
		else if( (recv_data.cchdr.command_control & COMMAND_TMN_MASK) == CHECK_END_RESULT )
		{
			query_vote_ask( recv_data.cchdr.address, recv_data.data[0]);
		}
	}

	if( (recv_data.cchdr.command_control & COMMAND_TMN_MASK) == TRANSIT_END_MSG ) // 特殊命令特殊处理
	{
		terminal_trasmint_message( recv_data.cchdr.address, recv_data.data, recv_data.data_len );
	}
	else // 处理其它命令
	{
		
	}
}

void host_reply_terminal()
{
	
}

