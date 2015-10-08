#include "terminal_pro.h"
#include "linked_list_uint.h"
#include "host_controller_debug.h"

terminal_address_list tmnl_addr_list[SYSTEM_TMNL_MAX_NUM];	// 终端地址分配列表
terminal_address_list_pro allot_addr_pro;	
tmnl_pdblist dev_terminal_list_guard; // 终端链表表头结点

void init_terminal_address_list( void )
{
	int i = 0;

	memset( tmnl_addr_list, 0, sizeof(tmnl_addr_list));
	for( i = 0; i< SYSTEM_TMNL_MAX_NUM; i++)		// 地址初始化为0xff
	{
		tmnl_addr_list[i].addr = 0xff;
	}

	init_terminal_dblist( &dev_terminal_list_guard );
	assert( dev_terminal_list_guard != NULL);
}

uint16_t ternminal_send( void *buf, uint16_t length, uint64_t uint64_target_id )
{
	struct host_to_endstation *send_buf = (struct host_to_endstation*)buf;
	struct jdksavdecc_frame send_frame;
	struct jdksavdecc_aecpdu_aem aemdu;
	struct jdksavdecc_eui64 target_id;
	convert_uint64_to_eui64( target_id.value, uint64_target_id);
	uint16_t all_length = length * 2; // 协议数据的总长度(包括备份的)
	int send_len = 0;

	memcpy( send_buf->deal_backups, buf, length);
	conference_host_to_end_form_msg_cha( &send_frame, send_buf, (ssize_t)send_buf->data_len );
	send_len = conference_1722_control_form_info( &send_frame, &aemdu, jdksavdecc_multicast_adp_acmp, target_id, all_length );
	if( send_len < 0 )
	{
		DEBUG_INFO( "send len is bad! send_len = %d", send_len );
		assert( send_len >= 0 );
	}

	system_raw_packet_tx( send_frame.dest_address.value, send_frame.payload, send_len, RUNINFLIGHT, TRANSMIT_TYPE_AECP, false );

	return (uint16_t)send_len;
}

void host_reply_terminal()
{
	
}

