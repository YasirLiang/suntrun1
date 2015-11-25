/**
*terminal_command.c
*
*conference protocol command proccess
*/

#include "terminal_command.h"
#include "terminal_common.h"

// ��ѯ�ն�, addr��δע��ĵ����ն��ѷ����˵ĵ�ַ(0x1)
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

// �ն˷����ַ(0x2)
void terminal_allot_address( void )
{
	struct host_to_endstation askbuf; 
	uint16_t  asklen = 0;
	uint64_t  target_zero = 0;
	
	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_ALLOCATION;
	askbuf.cchdr.address = 0x8000; // ȫ�㲥
	askbuf.data_len = 0;

	ternminal_send( &askbuf, asklen, target_zero, false );
}

// ���·����ַ������������Ӧ(0x3)
void terminal_reallot_address( void )
{
	struct host_to_endstation askbuf; 
	uint16_t  asklen = 0;
	uint64_t  target_zero = 0;
	
	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_REALLOCATION;
	askbuf.cchdr.address = 0x8000; // ȫ�㲥
	askbuf.data_len = 0;

	ternminal_send( &askbuf, asklen, target_zero, true ); 
}

// �ն�״̬����(noneed_resp)
void terminal_state_set( tmnl_state_set tmnlstate, uint16_t addr,  uint64_t  target_id )
{
	struct host_to_endstation askbuf;
	uint16_t asklen = 0;

	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_SET_END_STATUS;
	askbuf.cchdr.address = addr;
	askbuf.data_len = sizeof( tmnl_state_set );
	memcpy( askbuf.data, &tmnlstate, sizeof( tmnl_state_set)); 

	bool noneed_resp = false;// ����Ҫ��Ӧ
	if( (addr & BRDCST_ALL) && !(addr & BRDCST_NEED_RESPONSE ) )
	{
		noneed_resp = true;
	}

	ternminal_send( &askbuf, asklen, target_id, noneed_resp );
}

// ���û�Ͳ״̬(0x13)
void terminal_set_mic_status( uint8_t data, uint16_t addr,  uint64_t  target_id )
{
	struct host_to_endstation askbuf;
	uint16_t asklen = 0;

	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_SET_MIS_STATUS;
	askbuf.cchdr.address = addr;
	askbuf.data_len = sizeof( uint8_t );
	askbuf.data[0] = data;

	bool noneed_resp = false;// ����Ҫ��Ӧ
	if( (addr & BRDCST_ALL) && !(addr & BRDCST_NEED_RESPONSE ) )
	{
		noneed_resp = true;
	}
	
	ternminal_send( &askbuf, asklen, target_id, noneed_resp );
}

// �����ն�ָʾ��(0x06)
void terminal_set_indicator_lamp( ttmnl_led_lamp data, uint16_t addr, uint64_t target_id )
{
	struct host_to_endstation askbuf;
	uint16_t asklen = 0;

	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_SET_ENDLIGHT;
	askbuf.cchdr.address = addr;
	askbuf.data_len = sizeof( ttmnl_led_lamp ); // data lenght of the conference deal is 2
	askbuf.data[0] = data.data_low;
	askbuf.data[1] = data.data_high;

	bool noneed_resp = false;// ����Ҫ��Ӧ
	if( (addr & BRDCST_ALL) && !(addr & BRDCST_NEED_RESPONSE ) )
	{
		noneed_resp = true;
	}
	
	ternminal_send( &askbuf, asklen, target_id, noneed_resp );
}

// �����ն˷����ַ��0x07����ֻ��ID��Ч������,��ЩID�������ն˵�ַ�б�(�����ն˵�ַ�ļ�)�У����������ն˵�ע���б���
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

// �����ն˵�LCD��ʾ��ʽ(0x09)
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

// �����ն�LED��ʾ��ʽ(0x0B)
void terminal_set_led_play_stype( uint64_t target_id, uint16_t addr, tmnl_led_state_show_set led_stype )
{
	struct host_to_endstation askbuf;
	uint16_t asklen = 0;

	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_SET_END_LED;
	askbuf.cchdr.address = addr;
	askbuf.data_len = sizeof( uint16_t ); // data lenght of the conference deal is 2
	memcpy( askbuf.data, &led_stype, sizeof(uint16_t));

	bool noneed_resp = false;// ����Ҫ��Ӧ
	if( (addr & BRDCST_ALL) && !(addr & BRDCST_NEED_RESPONSE ) )
	{
		noneed_resp = true;
	}
	
	ternminal_send( &askbuf, asklen, target_id, noneed_resp );
}

// ��ϯ�����ƻ��飨0x0D������Ӧ�ն˵���ϯ��������
void terminal_chairman_control_meeting( uint64_t target_id, uint16_t addr, uint8_t data )
{
	struct host_to_endstation askbuf;
	uint16_t asklen = 0;

	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_PRESIDENT_CONTROL|COMMAND_TMN_REPLY;
	askbuf.cchdr.address = addr;
	askbuf.data_len = sizeof( uint8_t );
	askbuf.data[0] = data;	// oo nomal; 11 δǩ��

	ternminal_send( &askbuf, asklen, target_id, true );
}

// ���ͱ��������0x0E��
void terminal_send_vote_result( uint64_t target_id, uint16_t addr, tmnl_vote_result vote_rslt )
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

// ��������ʱ����0x0F��
void terminal_limit_spk_time( uint64_t target_id, uint16_t addr, tmnl_limit_spk_time spk_time )
{
	struct host_to_endstation askbuf;
	uint16_t asklen = 0;

	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_TALKTIME_LEN;
	askbuf.cchdr.address = addr;
	askbuf.data_len = sizeof( uint8_t );
	memcpy(&askbuf.data[0], &spk_time, sizeof( uint8_t));

	bool noneed_resp = false;// ����Ҫ��Ӧ
	if( (addr & BRDCST_ALL) && !(addr & BRDCST_NEED_RESPONSE ) )
	{
		noneed_resp = true;
	}
	
	ternminal_send( &askbuf, asklen, target_id, noneed_resp );
}

// ��������״̬0x10, ����������Ӧ(2015/11/4ע)
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
	askbuf.data[2] = ((uint8_t)(main_send.conference_stype & 0x0f)) |((uint8_t)((main_send.chm_first & 0x01) << 6))\
				| ((uint8_t)((main_send.camera_follow & 0x01) << 7));
	memcpy( &askbuf.data[3], &main_send.limit, askbuf.data_len - 3 );
	
	ternminal_send( &askbuf, asklen, target_id, true );
}

// �����ն�LCD��ʾ���ţ�0x11��
void terminal_send_end_lcd_display( uint64_t target_id, uint16_t addr, tmnl_send_end_lcd_display lcd_dis )
{
	struct host_to_endstation askbuf;
	uint16_t asklen = 0;

	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_SEND_END_LCD_DISPLAY;
	askbuf.cchdr.address = addr;
	askbuf.data_len = sizeof( uint16_t ); // 2
	askbuf.data[0] = lcd_dis.opt;
	askbuf.data[1] = lcd_dis.num;

	bool noneed_resp = false;
	if( (addr & BRDCST_ALL) && !(addr & BRDCST_NEED_RESPONSE ) )
	{
		noneed_resp = true;// ����Ҫ��Ӧ
	}
	
	ternminal_send( &askbuf, asklen, target_id, noneed_resp );
}

// �����ն�
void terminal_option_endpoint( uint64_t target_id, uint16_t addr, eopt_tmnl opt )
{
	struct host_to_endstation askbuf;
	uint16_t asklen = 0;

	DEBUG_INFO( "option end is %d " , (int)opt );
	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_OPTITION_END;
	askbuf.cchdr.address = addr;
	askbuf.data_len = sizeof( uint8_t ); // 1
	askbuf.data[0] = (uint8_t)opt;

	bool noneed_resp = false;
	if( (addr & BRDCST_ALL) && !(addr & BRDCST_NEED_RESPONSE ) )
	{
		noneed_resp = true;// ����Ҫ��Ӧ
	}
	
	ternminal_send( &askbuf, asklen, target_id, noneed_resp );
}

// �ն������¼���0x14����Ӧ(����)
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

// ת����λ������Ϣ��0x1E��
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

// ת���ն˶���Ϣ��0x1F��������Ӧ
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

// ��ѯ�ն�ǩ���������(0x16)
void terminal_query_vote_sign_result( uint64_t target_id, uint16_t addr )
{
	struct host_to_endstation askbuf;
	uint16_t asklen = 0;

	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_CHECK_END_RESULT;
	askbuf.cchdr.address = addr;
	askbuf.data_len = 0;

	ternminal_send( &askbuf, asklen, target_id, false );
}

// �ն˰�������(0x05 ������Ӧ) �˺���������ֻ��һ������ָ�벻Ϊ�գ���������������Ϊ��
void terminal_key_action_host_reply( uint64_t target_id, uint16_t addr, uint8_t data_len, tka_common_reply *common_data, tka_special1_reply *spe1_data, tka_special2_reply *spe2_data )
{
	struct host_to_endstation askbuf;
	uint16_t asklen = 0;
	memset(askbuf.data, 0, sizeof(askbuf.data));
	
	if( common_data != NULL )
	{
		askbuf.data[0] = common_data->recv_data;
	}
	else if( spe1_data != NULL )
	{
		askbuf.data[0] = spe1_data->recv_data;
		askbuf.data[1] = spe1_data->reply_num;
		askbuf.data[2] = spe1_data->mic_state;
	}
	else if( spe2_data != NULL )
	{
		askbuf.data[0] = spe2_data->recv_data;
		askbuf.data[1] = spe2_data->reply_num;
		askbuf.data[2] = spe2_data->key_down;
		askbuf.data[2] |= (uint8_t)((spe2_data->key_up&0x07) << 5);// ��3λλ�ڸ���λ
		askbuf.data[3] |= (uint8_t)((spe2_data->key_up&0x18) << 0);// ����λλ�ڵ���λ
		askbuf.data[3] |= (uint8_t)((spe2_data->key_led&0x3f) << 2);// low six bit 
		askbuf.data[4] |= (uint8_t)((spe2_data->key_led&0x03c0) << 0); // bit 9~bit 6
		askbuf.data[4] |= (uint8_t)((spe2_data->sys&0xf) << 4);
		askbuf.data[5] = spe2_data->lcd_num;
	}

	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_KEYPAD_ACTION |COMMAND_TMN_REPLY;
	askbuf.cchdr.address = addr;
	askbuf.data_len = data_len;
	
	ternminal_send( &askbuf, asklen, target_id, true );
}
