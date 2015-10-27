/**
*terminal_command.c
*
*conference protocol command proccess
*/

#include "terminal_command.h"

// 查询终端, addr是未注册的但是终端已分配了的地址
void terminal_query_endstation( uint16_t addr, uint64_t entity_id )
{
	struct host_to_endstation askbuf;
	uint16_t asklen = 0;
	
	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_QUERY_END;
	askbuf.cchdr.address = addr;
	askbuf.data_len = 0;
	
	ternminal_send( &askbuf, asklen, entity_id, false );
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

	ternminal_send( &askbuf, asklen, target_zero, false );
}

// 重新分配地址，此命令无响应
void terminal_reallot_address( void )
{
	struct host_to_endstation askbuf; 
	uint16_t  asklen = 0;
	uint64_t  target_zero = 0;
	
	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_REALLOCATION;
	askbuf.cchdr.address = 0x8000; // 全广播
	askbuf.data_len = 0;

	ternminal_send( &askbuf, asklen, target_zero, true ); 
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
	memcpy( askbuf.data, &tmnlstate, sizeof( tmnl_state_set)); 

	ternminal_send( &askbuf, asklen, target_id, false );
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

	ternminal_send( &askbuf, asklen, target_id, false );
}

// 设置终端指示灯
void terminal_set_indicator_lamp( uint16_t data, uint16_t addr, uint64_t target_id )
{
	struct host_to_endstation askbuf;
	uint16_t asklen = 0;

	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_SET_ENDLIGHT;
	askbuf.cchdr.address = addr;
	askbuf.data_len = sizeof( uint16_t ); // data lenght of the conference deal is 2
	askbuf.data[0] = (uint8_t)((data & 0xff00) >> 8);// 高八位在低字节
	askbuf.data[1] = (uint8_t)((data & 0x00ff) >> 0);

	ternminal_send( &askbuf, asklen, target_id, false );
}

// 新增终端分配地址（0x07）
void terminal_new_endstation_allot_address( uint64_t target_id )
{
	struct host_to_endstation askbuf;
	uint16_t asklen = 0;

	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_NEW_ALLOCATION;
	askbuf.cchdr.address = 0x8000;
	askbuf.data_len = 0; // data lenght of the conference deal is 0

	ternminal_send( &askbuf, asklen, target_id, false );
}

// 设置终端的LCD显示方式(0x09)
void terminal_set_lcd_play_stype( uint64_t target_id, uint16_t addr, uint8_t lcd_stype )
{
	struct host_to_endstation askbuf;
	uint16_t asklen = 0;

	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_SET_END_LCD;
	askbuf.cchdr.address = addr;
	askbuf.data_len = sizeof( uint8_t ); // data lenght of the conference deal is 1
	askbuf.data[0] = lcd_stype;

	ternminal_send( &askbuf, asklen, target_id, false );
}

// 设置终端LED显示方式(0x0B)
void terminal_set_led_play_stype( uint64_t target_id, uint16_t addr, tmnl_led_state_show_set led_stype )
{
	struct host_to_endstation askbuf;
	uint16_t asklen = 0;

	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_SET_END_LED;
	askbuf.cchdr.address = addr;
	askbuf.data_len = sizeof( uint16_t ); // data lenght of the conference deal is 2
	askbuf.data[0] = (((uint8_t)led_stype.blink & 0x01)<< 7) |(((uint8_t)led_stype.bright_lv & 0x0f ) << 3) |(((uint8_t)led_stype.page_show_state &0x07) << 0);
	askbuf.data[1] = (((uint8_t)led_stype.speed_roll & 0x0f )<< 4) |(((uint8_t)led_stype.stop_time & 0x0f ) << 0);
	
	ternminal_send( &askbuf, asklen, target_id, false );
}

// 主席机控制会议（0x0D）是响应终端的主席控制命令
void terminal_chairman_control_meeting( uint64_t target_id, uint16_t addr, uint8_t data )
{
	struct host_to_endstation askbuf;
	uint16_t asklen = 0;

	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_PRESIDENT_CONTROL|COMMAND_TMN_REPLY;
	askbuf.cchdr.address = addr;
	askbuf.data_len = sizeof( uint8_t );
	askbuf.data[0] = data;	// oo nomal; 11 未签到

	ternminal_send( &askbuf, asklen, target_id, true );
}

// 发送表决结果（0x0E）
void terminal_send_vote_result( uint64_t target_id, uint16_t addr, tmnl_vote_result vote_rslt)
{
	struct host_to_endstation askbuf;
	uint16_t asklen = 0;

	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_SEND_VOTE_RESULT;
	askbuf.cchdr.address = addr;
	askbuf.data_len = sizeof( tmnl_vote_result ); // data lenght of the conference deal is 8
	memcpy( askbuf.data, &vote_rslt, sizeof( tmnl_vote_result ));// it will copy from low bit of one data
	
	ternminal_send( &askbuf, asklen, target_id, false );
}

// 发言限制时长（0x0F）
void terminal_limit_spk_time( uint64_t target_id, uint16_t addr, tmnl_limit_spk_time spk_time )
{
	struct host_to_endstation askbuf;
	uint16_t asklen = 0;

	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_SEND_VOTE_RESULT;
	askbuf.cchdr.address = addr;
	askbuf.data_len = sizeof( uint8_t );
	memcpy(&askbuf.data[0], &spk_time, sizeof( uint8_t));
	
	ternminal_send( &askbuf, asklen, target_id, false );
}

// 主机发送状态0x10
void terminal_host_send_state( uint64_t target_id, tmnl_main_state_send main_send )
{
	struct host_to_endstation askbuf;
	uint16_t asklen = 0;

	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_HOST_SEND_STATUS;
	askbuf.cchdr.address = 0x8000;
	askbuf.data_len = sizeof( tmnl_main_state_send ) - 1; // data length is 7 based on conference deal
	askbuf.data[0] = ( uint8_t )((main_send.unit & 0x00ff) >> 0);
	askbuf.data[1] = ( uint8_t )((main_send.unit & 0xff00) >> 8);
	askbuf.data[2] = ((uint8_t)(main_send.conference_stype & 0xf)) |((uint8_t)((main_send.chm_first & 0x1) << 6))\
				| ((uint8_t)((main_send.camera_follow & 0x1) << 7));
	memcpy( &askbuf.data[3], &main_send.limit, askbuf.data_len - 3 );
	
	ternminal_send( &askbuf, asklen, target_id, false );
}

// 发送终端LCD显示屏号（0x11）
void terminal_send_end_lcd_display( uint64_t target_id, uint16_t addr, tmnl_send_end_lcd_display lcd_dis)
{
	struct host_to_endstation askbuf;
	uint16_t asklen = 0;

	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_SEND_END_LCD_DISPLAY;
	askbuf.cchdr.address = addr;
	askbuf.data_len = sizeof( uint16_t ); // 2
	askbuf.data[0] = lcd_dis.opt;
	askbuf.data[1] = lcd_dis.num;
	
	ternminal_send( &askbuf, asklen, target_id, false );
}

// 操作终端
void terminal_option_endpoint( uint64_t target_id, uint16_t addr, uint8_t opt )
{
	struct host_to_endstation askbuf;
	uint16_t asklen = 0;

	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_OPTITION_END;
	askbuf.cchdr.address = addr;
	askbuf.data_len = sizeof( uint8_t ); // 1
	askbuf.data[0] = opt;
	
	ternminal_send( &askbuf, asklen, target_id, false );
}

// 终端特殊事件（0x14）响应(主机)
void terminal_endstation_special_event_reply( uint64_t target_id, uint16_t addr )
{
	struct host_to_endstation askbuf;
	uint16_t asklen = 0;

	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_OPTITION_END |COMMAND_TMN_REPLY;
	askbuf.cchdr.address = addr;
	askbuf.data_len = 0;
	
	ternminal_send( &askbuf, asklen, target_id, true );
}

// 转发上位机短消息（0x1E）
void terminal_transmit_upper_cmpt_message( uint64_t target_id, uint16_t addr, uint8_t *msg, uint16_t msg_len)
{
	struct host_to_endstation askbuf;
	uint16_t asklen = 0;

	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_TRANSIT_HOST_MSG;
	askbuf.cchdr.address = addr;
	askbuf.data_len = msg_len;
	memcpy( askbuf.data, msg, msg_len );
	
	ternminal_send( &askbuf, asklen, target_id, false );
}

// 转发终端短消息（0x1F）主机响应
void terminal_reply_end_message_command( uint64_t target_id, uint16_t addr )
{
	struct host_to_endstation askbuf;
	uint16_t asklen = 0;

	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_TRANSIT_END_MSG |COMMAND_TMN_REPLY;
	askbuf.cchdr.address = addr;
	askbuf.data_len = 0;
	
	ternminal_send( &askbuf, asklen, target_id, true );
}

// 查询终端签到表决结果(0x16)
void terminal_query_vote_sign_result( uint64_t target_id, uint16_t addr )
{
	struct host_to_endstation askbuf;
	uint16_t asklen = 0;

	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_CHECK_END_RESULT ;
	askbuf.cchdr.address = addr;
	askbuf.data_len = 0;

	ternminal_send( &askbuf, asklen, target_id, false );
}

