#include "terminal_command.h"

static bool fill_ask_chk_sum( struct host_to_endstation* pask_buf, uint16_t* pask_len)
{
	uint8_t *p;
	uint8_t Chk;
	uint16_t i = 0;
	
	if((NULL == pask_buf)||(NULL == pask_len))
	{
		return false;
	}
	
	*pask_len = sizeof(struct conference_common_header) + 1 + pask_buf->data_len;// 负载长度，除了校验
	p = (uint8_t *)pask_buf;
	Chk = 0;
	for( i = 0; i < *pask_len; i++)
	{
		Chk ^= (*p);
		p++;
	}
	
	*p = Chk;
	(*pask_len) += 1;
	
	return true;
}

// 查询终端, addr是未注册的但是终端已分配了的地址
void terminal_query_endstation( uint16_t addr, uint64_t entity_id )
{
	struct host_to_endstation askbuf;
	uint16_t asklen = 0;
	
	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_QUERY_END;
	askbuf.cchdr.address = addr;
	askbuf.data_len = 0;

	fill_ask_chk_sum( &askbuf, &asklen);
	ternminal_send( &askbuf, askbuf, entity_id );
}

// 终端分配地址
void terminal_allot_address( void )
{
	struct host_to_endstation askbuf; 
	uint16_t  asklen = 0;
	uint64_t  target_zero = 0;
	
	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_ALLOCATION;
	askbuf.cchdr.address = 0x8000; // 全广播
	askbuf.data_len = 0;

	fill_ask_chk_sum(&askbuf,&asklen);
	ternminal_send( &askbuf, askbuf, target_zero);
}

// 重新分配地址
void terminal_reallot_address( void )
{
	struct host_to_endstation askbuf; 
	uint16_t  asklen = 0;
	uint64_t  target_zero = 0;

	allot_addr_pro.addr_start = 0;
	allot_addr_pro.index = 0;
	allot_addr_pro.renew_flag= 0;
	init_terminal_address_list();
	
	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_REALLOCATION;
	askbuf.cchdr.address = 0x8000; // 全广播
	askbuf.data_len = 0;

	fill_ask_chk_sum( &askbuf, &asklen );
	ternminal_send( &askbuf, askbuf, target_zero );
}

// 终端状态设置
void terminal_state_set( tmnl_state_set tmnlstate, uint16_t addr,  uint64_t  target_id )
{
	struct host_to_endstation askbuf;
	uint16_t asklen = 0;

	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_SET_END_STATUS;
	askbuf.cchdr.address = addr;
	askbuf.data_len = sizeof( tmnl_state_set );
	memcpy( askbuf.data, tmnlstate, sizeof( tmnl_state_set)); 

	fill_ask_chk_sum( &askbuf, &asklen);
	ternminal_send( &askbuf, asklen, target_id );
}

// 设置话筒状态(0x13)
void terminal_set_mic_status( uint8_t data, uint16_t addr,  uint64_t  target_id )
{
	struct host_to_endstation askbuf;
	uint16_t asklen = 0;

	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_SET_MIS_STATUS;
	askbuf.cchdr.address = addr;
	askbuf.data_len = sizeof( uint8_t );
	askbuf.data[0] = data;

	fill_ask_chk_sum( &askbuf, &asklen);
	ternminal_send( &askbuf, asklen, target_id );
}

// 设置终端指示灯
void terminal_set_indicator_lamp( uint16_t data, uint16_t addr, uint64_t target_id )
{
	struct host_to_endstation askbuf;
	uint16_t asklen = 0;

	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_SET_ENDLIGHT;
	askbuf.cchdr.address = addr;
	askbuf.data_len = sizeof( uint16_t );
	askbuf.data[0] = (uint8_t)((data & 0xff00) >> 8);// 高八位在低字节
	askbuf.data[1] = (uint8_t)((data & 0x00ff) >> 0);

	fill_ask_chk_sum( &askbuf, &asklen);
	ternminal_send( &askbuf, asklen, target_id );
}

// 新增终端分配地址（0x07）
void terminal_new_endstation_allot_address( uint64_t target_id )
{
	struct host_to_endstation askbuf;
	uint16_t asklen = 0;

	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_NEW_ALLOCATION;
	askbuf.cchdr.address = 0x8000;
	askbuf.data_len = 0;

	fill_ask_chk_sum( &askbuf, &asklen);
	ternminal_send( &askbuf, asklen, target_id );
}

// 设置终端的LCD显示方式(0x09)
void terminal_set_lcd_play_stype( uint64_t target_id, uint16_t addr, uint8_t lcd_stype)
{
	struct host_to_endstation askbuf;
	uint16_t asklen = 0;

	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_SET_END_LCD;
	askbuf.cchdr.address = addr;
	askbuf.data_len = sizeof( uint8_t );

	fill_ask_chk_sum( &askbuf, &asklen);
	ternminal_send( &askbuf, asklen, target_id );
}


