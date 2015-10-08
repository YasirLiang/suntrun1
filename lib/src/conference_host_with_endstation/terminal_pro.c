#include "terminal_pro.h"

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
}

uint16_t ternminal_send( void *buf, uint16_t length )
{
	struct host_to_endstation *send_buf = (struct host_to_endstation*)buf;
	struct jdksavdecc_frame send_frame;

	memcpy( send_buf->deal_backups, buf, length);
	conference_host_to_end_form_msg_cha( &send_frame,send_buf, (ssize_t)send_buf->data_len );
	
}

