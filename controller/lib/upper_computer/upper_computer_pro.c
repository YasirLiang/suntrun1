/*	upper_computer_pro.c
** ע: 
**	1. ����������δ��ɣ���ע��������������δ��ɡ�
**	2. �����������Ѿ���ɣ�����Ҫ���ƣ���ע���������������ơ�
**	3. �����������Ѿ���ɣ����ں��ڿ�����Ҫ�޸ģ������޸ĺ�ע���޸ĵ�ʱ�������ݣ�������ԭ���Ĵ������̡�
** 	4. �����������Ѿ���ɣ�����ע����
**
*/

#include "upper_computer_pro.h"
#include "profile_system.h"
#include "upper_computer_common.h"
#include "upper_computer_command.h"
#include "upper_computer_data_parser.h"
#include "terminal_pro.h"
#include "terminal_command.h"
#include "util.h"
#include "upper_computer_data_form.h"
#include "system_packet_tx.h"
#include "terminal_system.h"

int profile_system_file_dis_param_save( FILE* fd, tcmpt_discuss_parameter *set_dis_para )
{
	assert( fd && set_dis_para );
	
	return (( profile_system_file_write( fd, set_dis_para->chairman_first, VAL_CHM_PRIOR_ENUM ) != -1) && \
	 (profile_system_file_write( fd, set_dis_para->chair_music, VAL_CHM_MUSIC ) != -1) && \
	 (profile_system_file_write( fd, set_dis_para->auto_close, VAL_AUTO_CLOSE ) != -1) && \
	 (profile_system_file_write( fd, set_dis_para->discuss_mode, VAL_DSCS_MODE ) != -1) && \
	 (profile_system_file_write( fd, set_dis_para->limit_speak_num, VAL_SPKER_LIMIT ) != -1) && \
	 (profile_system_file_write( fd, set_dis_para->limit_apply_num, VAL_APPLY_LIMIT ) != -1) && \
	 (profile_system_file_write( fd, set_dis_para->limit_chm_time, VAL_CHM_TIMED_ENUM ) != -1) && \
	 (profile_system_file_write( fd, set_dis_para->limit_vip_time, VAL_VIP_TIMED_ENUM ) != -1) && \
	 (profile_system_file_write( fd, set_dis_para->limit_speak_time, VAL_SPK_LIMIT_TIME ) != -1)) ? 0 : -1;
}

/*==================================================
					��ʼ�ն������
====================================================*/

int proccess_upper_cmpt_discussion_parameter( uint16_t protocol_type, void *data, uint32_t data_len ) //���ڿ�����Ҫ�޸�11/10��
{
	FILE* fd = NULL;
	thost_system_set set_sys; // ϵͳ�����ļ��ĸ�ʽ
	tcmpt_discuss_parameter qu_dis_para, set_dis_para;
	uint16_t send_data_len = 0; // Э�����ݸ��صĳ���
	
	fd = Fopen( STSTEM_SET_STUTUS_PROFILE, "rb+" );
	if( NULL == fd )
	{
		DEBUG_INFO( "open files %s Err!",  STSTEM_SET_STUTUS_PROFILE );
		return -1;
	}
	if( profile_system_file_read( fd, &set_sys ) == -1)
	{
		DEBUG_INFO( "Read profile system Err!" );
		return -1;
	}

	if((protocol_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_QUERY )
	{
		qu_dis_para.chairman_first = set_sys.chman_first ? ENABLE_VAL : FORBID_VAL;
		qu_dis_para.chair_music = set_sys.chman_music ? ENABLE_VAL : FORBID_VAL;
		qu_dis_para.auto_close = set_sys.auto_close ? ENABLE_VAL : FORBID_VAL;
		qu_dis_para.discuss_mode = set_sys.discuss_mode & 0x07;
		qu_dis_para.limit_speak_num = set_sys.speak_limit & 0xff;
		qu_dis_para.limit_apply_num = set_sys.apply_limit & 0xff;
		qu_dis_para.limit_chm_time = set_sys.chman_limitime ? ENABLE_VAL : FORBID_VAL;
		qu_dis_para.limit_vip_time = set_sys.vip_limitime ? ENABLE_VAL : FORBID_VAL;
		qu_dis_para.limit_speak_time = set_sys.spk_limtime & 0x3f;
		send_data_len = sizeof(tcmpt_discuss_parameter);
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE |CMPT_MSG_TYPE_QUERY, \
			DISCUSSION_PARAMETER, &qu_dis_para, send_data_len);
	}
	// ���ñ���ϵͳ״ֵ̬������ϵͳ״̬
	else if((protocol_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET )
	{
		// ������Ӧ
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE |CMPT_MSG_TYPE_SET, \
			DISCUSSION_PARAMETER, NULL, 0 ); // ����������ĸ�������Э����Щ���룬�����Ǹ��ݻƹ�����д�ġ�Э�������ݵ�Ԫ��һ���ֽ�0�����óɹ�����������ʧ�ܡ����ƹ���û�����ݵ�Ԫ��������дNULL
			
		get_host_upper_cmpt_data( &set_dis_para, data, CMPT_DATA_OFFSET, sizeof(tcmpt_discuss_parameter));
		DEBUG_RECV( &set_dis_para, sizeof(tcmpt_discuss_parameter), "Dis Param ");

		// ���������ļ�
		if( profile_system_file_dis_param_save( fd, &set_dis_para ) != -1 )
		{
			Fflush( fd ); // ˢ�µ��ļ���
			
			// ����ϵͳ״̬
			uint8_t temp_status = set_dis_para.auto_close;
			if( temp_status != set_sys.auto_close )
			{
				// �Զ��ر���˷�
				//DEBUG_INFO( "auto close = %d", temp_status );
				find_func_command_link( MENU_USE, MENU_AUTO_CLOSE_CMD, 0, NULL, 0 );
			}

			temp_status = set_dis_para.discuss_mode;
			if( temp_status != set_sys.discuss_mode )
			{
				// ����ϵͳ����ģʽ
				//DEBUG_INFO( "discuss_mode = %d", temp_status );
				find_func_command_link( MENU_USE, MENU_DISC_MODE_SET_CMD, 0, &temp_status, 1 );
			}
			
			temp_status = set_dis_para.limit_speak_num;
			if( temp_status != set_sys.speak_limit )
			{
				// �������Ƶķ�������
				//DEBUG_INFO( "limit_speak_num = %d", temp_status );
				find_func_command_link( MENU_USE, MENU_SPK_LIMIT_NUM_SET, 0, &temp_status, 1 );
			}

			temp_status = set_dis_para.limit_apply_num;
			if( temp_status != set_sys.apply_limit )
			{
				// ����������������
				//DEBUG_INFO( "limit_apply_num = %d", temp_status );
				find_func_command_link( MENU_USE, MUNU_APPLY_LIMIT_NUM_SET, 0, &temp_status, 1 );
			}

			// ��ʱ����
			//DEBUG_INFO( " save limit_speak_time = %d", set_dis_para.limit_speak_time );
			terminal_limit_speak_time_set( 0, NULL, 0 );
			
			// ���û�������״̬
			terminal_start_discuss( false );
		}
		else
		{
			return -1; // ���ﷵ��0 ��Ŀ���ǲ�������������
		}
	}

	Fclose( fd );
	
	return 0;
}

int proccess_upper_cmpt_microphone_switch( uint16_t protocal_type, void *data, uint32_t data_len )//���ڿ�����Ҫ�޸�11/10��
{
	tcmpt_data_mic_switch mic_flag;
	uint16_t len_data_get = get_host_upper_cmpt_data_len( data, CMPT_HEAD_OFFSET );
	get_host_upper_cmpt_data( &mic_flag, data, CMPT_DATA_OFFSET, len_data_get );

	if( (protocal_type & CMPT_MSG_TYPE_MARK) != CMPT_MSG_TYPE_SET )
	{
		return -1;
	}

	send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, MISCROPHONE_SWITCH, NULL, 0 );

	DEBUG_INFO( "mic sw addr = 0x%02x%02x, flag = %d ", mic_flag.addr.high_addr, mic_flag.addr.low_addr, mic_flag.switch_flag );
	terminal_upper_computer_speak_proccess( mic_flag );
	
	return 0;
}

/*��˷�״̬*/ 
int proccess_upper_cmpt_miscrophone_status( uint16_t protocal_type, void *data, uint32_t data_len )//���ڿ�����Ҫ�޸�11/10��
{
	assert( data );
	tmnl_pdblist tmnl_list_head = dev_terminal_list_guard;
	tmnl_pdblist p_tmnl_tmp = dev_terminal_list_guard->next;
	tdata_addr_and_flag mic_list[SYSTEM_TMNL_MAX_NUM]; // ÿ���ն˵���˷�״̬��Ӧһ��Ԫ��
	uint16_t addr_num = 0;

	if( (protocal_type & CMPT_MSG_TYPE_MARK) != CMPT_MSG_TYPE_QUERY )
	{
		return -1;
	}

	for( ; p_tmnl_tmp != tmnl_list_head; p_tmnl_tmp = p_tmnl_tmp->next )
	{
		
		if( p_tmnl_tmp->tmnl_dev.address.addr != 0xffff && p_tmnl_tmp->tmnl_dev.tmnl_status.is_rgst )
		{
			uint8_t low_addr = (uint8_t)((p_tmnl_tmp->tmnl_dev.address.addr &0x00ff) >> 0);
			uint8_t high_addr =  (uint8_t)((p_tmnl_tmp->tmnl_dev.address.addr &0xff00) >> 8);
			uint8_t mic_state = p_tmnl_tmp->tmnl_dev.tmnl_status.mic_state;
			mic_list[addr_num].addr_flag[0] = low_addr;
			mic_list[addr_num].addr_flag[1] = high_addr;
			mic_list[addr_num].addr_flag[2] = mic_state;
			addr_num++;
		}
	}

	send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_QUERY, MISCROPHONE_STATUS, mic_list, addr_num * sizeof(tdata_addr_and_flag) );
	
	return 0;
}

int proccess_upper_cmpt_select_proposer( uint16_t protocal_type, void *data, uint32_t data_len )//���ڿ�����Ҫ�޸�11/9��
{
	uint16_t addr = get_uint16_data_from_buf( data, CMPT_DATA_OFFSET );
	
	if( (protocal_type & CMPT_MSG_TYPE_MARK) != CMPT_MSG_TYPE_SET )
	{
		return -1;
	}

	/*����ѡ��������*/
	DEBUG_INFO( "select apply = %04x ", addr );
	terminal_select_apply( addr ); // ʹѡ�������������λ������
	
	send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, SELECT_PROPOSER, NULL, 0 );
	
	return 0;
}
int proccess_upper_cmpt_examine_application( uint16_t protocal_type, void *data, uint32_t data_len )//���ڿ�����Ҫ�޸�11/10��
{
	uint8_t exam_value = get_uint8_data_value_from_buf( data, CMPT_DATA_OFFSET );

	if( (protocal_type & CMPT_MSG_TYPE_MARK) != CMPT_MSG_TYPE_SET)
	{
		return -1;
	}
	
	/*������������*/
	DEBUG_INFO( "examine application value = %d ", exam_value );
	if( exam_value )
	{
		terminal_examine_apply( APPROVE_APPLY );
	}
	else
	{
		terminal_examine_apply( REFUSE_APPLY );
	}

	send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, EXAMINE_APPLICATION, NULL, 0 );
	
	return 0;
}

int proccess_upper_cmpt_conference_permission( uint16_t protocal_type, void *data, uint32_t data_len )//���ڿ�����Ҫ�޸�11/10��
{
	tcmpt_data_meeting_authority tmnl_type;
	uint16_t len_data_get = get_host_upper_cmpt_data_len( data, CMPT_HEAD_OFFSET );
	get_host_upper_cmpt_data( &tmnl_type, data, CMPT_DATA_OFFSET, len_data_get );
	
	if( (protocal_type & CMPT_MSG_TYPE_MARK) != CMPT_MSG_TYPE_SET)
	{
		return -1;
	}
	
	/*��������Ȩ��*/
	DEBUG_INFO( "examine permit  = %d, addr = 0x%02x%02x ", tmnl_type.identity, tmnl_type.addr.high_addr, tmnl_type.addr.low_addr );
	terminal_type_set( tmnl_type );

	send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, CONFERENCE_PERMISSION, NULL, 0 );
	return 0;
}

int proccess_upper_cmpt_senddown_message( uint16_t protocal_type, void *data, uint32_t data_len )//���ڿ�����Ҫ�޸�11/10��
{
	uint16_t address = get_uint16_data_from_buf( data, CMPT_DATA_OFFSET );
	uint16_t lenght = get_uint16_data_from_buf( data, CMPT_DATA_LEN_OFFSET ); // ����������
	tcmpt_data_message data_msg;
	
	if( (protocal_type & CMPT_MSG_TYPE_MARK) != CMPT_MSG_TYPE_SET)
	{
		return -1;
	}

	/*����������λ����Ϣ*/
	conference_host_upper_computer_set_upper_message_form( &data_msg, data, CMPT_DATA_OFFSET, lenght );
	terminal_send_upper_message( ((uint8_t*)&data_msg) + sizeof(uint16_t), address, lenght - sizeof(uint16_t) );

	send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, SENDDOWN_MESSAGE, NULL, 0 );
	
	return 0;
}

int proccess_upper_cmpt_tablet_stands_manager( uint16_t protocal_type, void *data, uint32_t data_len )//���ڿ�����Ҫ�޸�11/10��
{
	assert( data );
	uint16_t address = get_uint16_data_from_buf( data, CMPT_DATA_OFFSET );
	uint16_t lenght = get_uint16_data_from_buf( data, CMPT_DATA_LEN_OFFSET ); // ����������
	tcmpt_table_card table_card;

	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
	
		conference_host_upper_computer_set_upper_message_form( &table_card, data, CMPT_DATA_LEN_OFFSET, lenght );

		/*�����ն˵�����*/
		terminal_tablet_stands_manager( &table_card, address, lenght - sizeof(uint16_t)); // ע:���ﲻ�漰��ϵͳ������״̬�Ĵ���
		
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, TABLE_TABLET_STANDS_MANAGER, NULL, 0 );
	}
	else
	{
		return -1;
	}
	
	return 0;
}

int proccess_upper_cmpt__begin_sign( uint16_t protocal_type, void *data, uint32_t data_len ) //���ڿ�����Ҫ�޸�11/10��
{
	assert( data );
	tcmpt_begin_sign sign_flag;
	sign_flag.sign_type = (get_uint8_data_value_from_buf( data, CMPT_DATA_OFFSET )&0x03); // ����λ 00 ������01�忨����������
	sign_flag.retroactive_timeouts = get_uint8_data_value_from_buf( data, CMPT_DATA_OFFSET + 1);

	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		/*ϵͳ��ʼǩ��*/
		terminal_start_sign_in( sign_flag ); 
		
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, BEGIN_SIGN, NULL, 0 );
	}
	else
	{
		return -1;
	}
	
	return 0;
}

/* ϵͳ���ն˵�ǩ����� */
int proccess_upper_cmpt_sign_situation( uint16_t protocal_type, void *data, uint32_t data_len )//���ڿ�����Ҫ�޸�10/30��
{
	assert( data && dev_terminal_list_guard );
	tmnl_pdblist tmnl_list_head = dev_terminal_list_guard;
	tmnl_pdblist p_tmnl_list = dev_terminal_list_guard->next;
	tcmpt_sign_situation sign_list[SYSTEM_TMNL_MAX_NUM];
	uint16_t sign_num = 0;
	
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_QUERY )
	{
		for( ; p_tmnl_list != tmnl_list_head; p_tmnl_list = p_tmnl_list->next )
		{
			if( p_tmnl_list->tmnl_dev.address.addr != 0xffff && p_tmnl_list->tmnl_dev.tmnl_status.is_rgst ) 
			{
				sign_list[sign_num].addr.low_addr = (uint8_t)((p_tmnl_list->tmnl_dev.address.addr &0x00ff) >> 0); // low addr
				sign_list[sign_num].addr.high_addr = (uint8_t)((p_tmnl_list->tmnl_dev.address.addr &0xff00) >> 8); // ligh addr
				sign_list[sign_num].sign_situation = p_tmnl_list->tmnl_dev.tmnl_status.sign_state;
				sign_num++;
			}
		}

		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_QUERY, SIGN_SITUATION,\
			sign_list, sign_num* sizeof(tcmpt_sign_situation));
	}
	else
	{
		return -1;
	}
	
	return 0;
}

int proccess_upper_cmpt_end_of_sign( uint16_t protocal_type, void *data, uint32_t data_len )//���ڿ�����Ҫ�޸�11/10��
{
	assert( data );
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		/*��������ǩ��*/
		terminal_end_sign( 0, NULL, 0 ); 
		
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, END_OF_SIGN, NULL, 0 );
	}
	else
	{
		return -1;
	}
	
	return 0;
}

int proccess_upper_cmpt_endtation_allocation_address( uint16_t protocal_type, void *data, uint32_t data_len )//���ڿ�����Ҫ�޸�11/2��
{
	tcmpt_end_allot_addr allot_flag;
	allot_flag.allot_type = get_uint8_data_value_from_buf( data, CMPT_DATA_OFFSET );
	
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		switch( allot_flag.allot_type )
		{
			case REALLOT:
				terminal_reallot_address();
				break;
			case NEW_ALLOT:
				terminal_allot_address();
				break;
			case REMOVE_UNREGISTER:
				terminal_remove_unregitster();
				break;
			default:
				break;
			
		}
		
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, ENDSTATION_ALLOCATION_APPLICATION_ADDRESS, NULL, 0 );
	}
	else
	{
		return -1;
	}
	
	return 0;
}

int proccess_upper_cmpt_endstation_register_status( uint16_t protocal_type, void *data, uint32_t data_len )//���ڿ�����Ҫ�޸�11/2��
{
	assert( dev_terminal_list_guard );
	tmnl_pdblist tmnl_list_head = dev_terminal_list_guard;
	tmnl_pdblist p_tmnl_list = dev_terminal_list_guard->next;
	tcmpt_end_register regist_list[SYSTEM_TMNL_MAX_NUM];
	uint16_t register_num = 0;
	
	if( (protocal_type & CMPT_MSG_TYPE_MARK) != CMPT_MSG_TYPE_QUERY )
	{
		return -1;
	}

	for( ; p_tmnl_list != tmnl_list_head; p_tmnl_list = p_tmnl_list->next )
	{
		if( p_tmnl_list->tmnl_dev.address.addr != 0xffff && p_tmnl_list->tmnl_dev.tmnl_status.is_rgst )
		{
			uint8_t low_addr = (uint8_t)((p_tmnl_list->tmnl_dev.address.addr &0x00ff) >> 0);
			uint8_t high_addr =  (uint8_t)((p_tmnl_list->tmnl_dev.address.addr &0xff00) >> 8);
			uint8_t dev_type = p_tmnl_list->tmnl_dev.tmnl_status.device_type;
			uint8_t tmnl_type = p_tmnl_list->tmnl_dev.address.tmn_type;
			regist_list[register_num].addr.low_addr = low_addr;
			regist_list[register_num].addr.high_addr = high_addr;
			regist_list[register_num].device_type = dev_type;
			regist_list[register_num].end_type = tmnl_type;
			regist_list[register_num].register_flags = (p_tmnl_list->tmnl_dev.tmnl_status.is_rgst ? 1:0);

			register_num++;
		}
	}

	if( register_num <= SYSTEM_TMNL_MAX_NUM)
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_QUERY, ENDSTATION_REGISTER_STATUS, regist_list, register_num * sizeof(tcmpt_end_register));
	
	return 0;
}

int proccess_upper_cmpt_current_vidicon( uint16_t protocal_type, void *data, uint32_t data_len )// ������������δ���11/2��
{
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		//terminal_start_sign_in( sign_flag ); 
		
		//send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, BEGIN_SIGN, NULL, 0 );
	}
	else
	{
		return -1;
	}
	
	return 0;
}

int proccess_upper_cmpt_endstation_address_undetermined_allocation( uint16_t protocal_type, void *data, uint32_t data_len )// ������������δ���11/2��
{
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		//terminal_start_sign_in( sign_flag ); 
		
		//send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, BEGIN_SIGN, NULL, 0 );
	}
	else
	{
		return -1;
	}
	
	return 0;
}

int proccess_upper_cmpt_vidicon_control( uint16_t protocal_type, void *data, uint32_t data_len )// ������������δ���11/2��
{
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		//terminal_start_sign_in( sign_flag ); 
		
		//send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, BEGIN_SIGN, NULL, 0 );
	}
	else
	{
		return -1;
	}
	
	return 0;
}

int proccess_upper_cmpt_vidicon_preration_set( uint16_t protocal_type, void *data, uint32_t data_len )// ������������δ���11/2��
{
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		//terminal_start_sign_in( sign_flag ); 
		
		//send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, BEGIN_SIGN, NULL, 0 );
	}
	else
	{
		return -1;
	}
	
	return 0;
}

int proccess_upper_cmpt_vidicon_lock( uint16_t protocal_type, void *data, uint32_t data_len )// ������������δ���11/2��
{
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		//terminal_start_sign_in( sign_flag ); 
		
		//send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, BEGIN_SIGN, NULL, 0 );
	}
	else
	{
		return -1;
	}
	
	return 0;
}

int proccess_upper_cmpt_vidicon_output( uint16_t protocal_type, void *data, uint32_t data_len )// ������������δ���11/2��
{
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		//terminal_start_sign_in( sign_flag ); 
		
		//send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, BEGIN_SIGN, NULL, 0 );
	}
	else
	{
		return -1;
	}
	
	return 0;
}

int proccess_upper_cmpt_cmpt_begin_vote( uint16_t protocal_type, void *data, uint32_t data_len )//���ڿ�����Ҫ�޸�11/10��
{
	tcmp_vote_start vote_start_flag;
	uint8_t sign_flag = 0; // ��Ӧ��ǩ����־
	vote_start_flag.vote_type = get_uint8_data_value_from_buf( data, CMPT_DATA_OFFSET ) & 0x0f;
	vote_start_flag.key_effective = get_uint8_data_value_from_buf( data, CMPT_DATA_OFFSET ) & 0x10;
	
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		terminal_begin_vote( vote_start_flag,  &sign_flag); 
		
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, BEGIN_VOTE, &sign_flag, sizeof(uint8_t));
	}
	else
	{
		return -1;
	}
	
	return 0;
}

int proccess_upper_cmpt_pause_vote( uint16_t protocal_type, void *data, uint32_t data_len )
{
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		terminal_pause_vote( 0, NULL, 0 );
		
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, PAUSE_VOTE, NULL, 0 );
	}
	else
	{
		return -1;
	}
	
	return 0;
}

int proccess_upper_cmpt_regain_vote( uint16_t protocal_type, void *data, uint32_t data_len )
{
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		terminal_regain_vote( 0, NULL, 0 );
		
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, REGAIN_VOTE, NULL, 0 );
	}
	else
	{
		return -1;
	}
	
	return 0;
}

int proccess_upper_cmpt_end_vote( uint16_t protocal_type, void *data, uint32_t data_len )//���ڿ�����Ҫ�޸�11/10��
{
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		/*���ý���ͶƱ��־*/
		terminal_end_vote( 0, NULL, 0); 
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, END_VOTE, NULL, 0 );

		/*��������ģʽ�Ŀ�ʼ��־*/
		set_terminal_system_state( DISCUSS_STATE, true );
		terminal_start_discuss(false);
	}
	else
	{
		return -1;
	}
	
	return 0;
}

/*�������*/ 
int proccess_upper_cmpt_result_vote( uint16_t protocal_type, void *data, uint32_t data_len )//���ڿ�����Ҫ�޸�11/10��
{
	assert( dev_terminal_list_guard );
	tmnl_pdblist tmnl_list_head = dev_terminal_list_guard;
	tmnl_pdblist p_tmnl_tmp = dev_terminal_list_guard->next;
	tcmp_vote_result vote_result[SYSTEM_TMNL_MAX_NUM]; // 
	uint16_t addr_num = 0;
	uint16_t send_data_len = 0;
	
	if( (protocal_type & CMPT_MSG_TYPE_MARK) != CMPT_MSG_TYPE_QUERY )
	{
		return -1;
	}
	
	for( ; p_tmnl_tmp != tmnl_list_head; p_tmnl_tmp = p_tmnl_tmp->next )
	{
		if((p_tmnl_tmp->tmnl_dev.address.addr != 0xffff) && (p_tmnl_tmp->tmnl_dev.tmnl_status.is_rgst))
		{
			vote_result[addr_num].addr.low_addr = (uint8_t)((p_tmnl_tmp->tmnl_dev.address.addr &0x00ff) >> 0); // low addr
			vote_result[addr_num].addr.high_addr = (uint8_t)((p_tmnl_tmp->tmnl_dev.address.addr &0xff00) >> 8); // hight addr
			vote_result[addr_num].key_value= p_tmnl_tmp->tmnl_dev.tmnl_status.vote_state & TVOTE_KEY_MARK;
			addr_num++;
		}
	}

	send_data_len = sizeof(tcmp_vote_result) * addr_num;
	DEBUG_INFO( "vote result data len = %d",  data_len );
	send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE |CMPT_MSG_TYPE_QUERY, RESULT_VOTE, vote_result, send_data_len );
	
	return 0;
}

int proccess_upper_cmpt_transmit_to_endstation( uint16_t protocal_type, void *data, uint32_t data_len )// ������������δ���11/10��
{
	assert( data );
	tcmpt_message msg;
	uint16_t recv_data_len = get_host_upper_cmpt_data_len( data, CMPT_HEAD_OFFSET );
	get_host_upper_cmpt_data(msg.msg_buf, data, CMPT_DATA_LEN_OFFSET, recv_data_len );

	DEBUG_RECV(msg.msg_buf, data_len, "transmit to end msg:");
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET )
	{
		//terminal_start_sign_in( sign_flag ); 
		
		//send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, BEGIN_SIGN, NULL, 0 );
	}
	else
	{
		return -1;
	}
	
	return 0;
}

/*ע:�˺��������ϱ��ն˶���Ϣ�����ǶԽ����������ݽ��д����ĺ���,data ���������ݵ���������ָ�룬data_len �����������ݵĳ���*/
int proccess_upper_cmpt_report_endstation_message( uint16_t protocal_type, void *data, uint32_t data_len )//���ڿ�����Ҫ�޸�11/10��
{
	assert( data );
	tcmp_report_terminal_message tmnl_msg;
	memcpy( &tmnl_msg, data, data_len );
	
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_REPORT)
	{	
		send_upper_computer_command(  CMPT_MSG_TYPE_REPORT, REPORT_ENDSTATION_MESSAGE, &tmnl_msg, data_len );
	}
	else
	{
		return -1;
	}
	
	return 0;
}

// ��������ͷ��������
int proccess_upper_cmpt_hign_definition_switch_set( uint16_t protocal_type, void *data, uint32_t data_len )// ������������δ���11/2��
{
	assert( data );
	
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		//terminal_start_sign_in( sign_flag ); 
		
		//send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, BEGIN_SIGN, NULL, 0 );
	}
	else
	{
		return -1;
	}
	
	return 0;
}

/*==================================================
					�����ն������
====================================================*/

/*==================================================
					��λ��������������
====================================================*/
/*�ϱ�������˷�״̬*/
int upper_cmpt_report_mic_state( uint8_t mic_status, uint16_t addr )
{
	tcmpt_data_mic_status mic_endpoint;
	
	if( (mic_status == MIC_COLSE_STATUS) || (mic_status == MIC_OPEN_STATUS) ||\
		(mic_status == MIC_FIRST_APPLY_STATUS) ||(mic_status == MIC_OTHER_APPLY_STATUS) ||\
		(mic_status == MIC_PRESET_BIT_STATUS) ||(mic_status == MIC_CHM_INTERPOSE_STATUS))
	{
		mic_endpoint.addr.low_addr = (uint8_t)((addr &0x00ff) >> 0);
		mic_endpoint.addr.high_addr = (uint8_t)((addr &0xff00) >> 8);
		mic_endpoint.switch_flag = mic_status;

		send_upper_computer_command( CMPT_MSG_TYPE_REPORT, \
		MISCROPHONE_STATUS, &mic_endpoint, sizeof(tcmpt_data_mic_status));
	}
	else
	{
		DEBUG_INFO( "not valid rang mic_status flag!" );
		return -1;
	}

	return 0;
}

int cmpt_miscrophone_status_list( void )
{
	tmnl_pdblist tmnl_list_head = dev_terminal_list_guard;
	tmnl_pdblist p_tmnl_tmp = dev_terminal_list_guard->next;
	tdata_addr_and_flag mic_list[SYSTEM_TMNL_MAX_NUM]; // ÿ���ն˵���˷�״̬��Ӧһ��Ԫ��
	uint16_t addr_num = 0;

	for( ; p_tmnl_tmp != tmnl_list_head; p_tmnl_tmp = p_tmnl_tmp->next )
	{
		
		if( p_tmnl_tmp->tmnl_dev.address.addr != 0xffff && p_tmnl_tmp->tmnl_dev.tmnl_status.is_rgst )
		{
			uint8_t low_addr = (uint8_t)((p_tmnl_tmp->tmnl_dev.address.addr &0x00ff) >> 0);
			uint8_t high_addr =  (uint8_t)((p_tmnl_tmp->tmnl_dev.address.addr &0xff00) >> 8);
			uint8_t mic_state = p_tmnl_tmp->tmnl_dev.tmnl_status.mic_state;
			mic_list[addr_num].addr_flag[0] = low_addr;
			mic_list[addr_num].addr_flag[1] = high_addr;
			mic_list[addr_num].addr_flag[2] = mic_state;
			addr_num++;
		}
	}

	send_upper_computer_command( CMPT_MSG_TYPE_REPORT, MISCROPHONE_STATUS, mic_list, addr_num * sizeof(tdata_addr_and_flag) );
	
	return 0;
}

int cmpt_miscrophone_status_list_from_set( tcmpt_data_mic_status *p_mic_list, uint16_t mic_num )
{
	assert( p_mic_list );
	send_upper_computer_command( CMPT_MSG_TYPE_REPORT, MISCROPHONE_STATUS, p_mic_list, mic_num*sizeof(tcmpt_data_mic_status) );
	return 0;
}

/*�ϱ��ն�ǩ�����*/
int upper_cmpt_report_sign_in_state( uint8_t sign_status, uint16_t addr )
{
	tcmpt_sign_situation sign_flag_station;

	if( TMNL_SIGN_BE_LATE == sign_status ||TMNL_SIGN_BE_LATE == sign_status || TMNL_SIGN_BE_LATE == sign_status )
	{
		sign_flag_station.addr.low_addr = (uint8_t)((addr &0x00ff) >> 0);
		sign_flag_station.addr.high_addr = (uint8_t)((addr &0xff00) >> 8);
		sign_flag_station.sign_situation = sign_status;
		send_upper_computer_command( CMPT_MSG_TYPE_REPORT, \
			SIGN_SITUATION, &sign_flag_station, sizeof(tcmpt_sign_situation));
	}
	else
	{
		DEBUG_INFO( "not valid rang sign flag!" );
		return -1;
	}

	return 0;
}

int upper_cmpt_sign_situation_report( uint8_t vote_rlst, uint16_t addr )
{
	tcmp_vote_result vote_data;

	if( addr != 0xffff )
	{
		vote_data.addr.low_addr = (uint8_t)((addr &0x00ff) >> 0); // low addr
		vote_data.addr.high_addr = (uint8_t)(( addr &0xff00) >> 8); // hight addr
		vote_data.key_value= vote_rlst & TVOTE_KEY_MARK;
	}
	
	send_upper_computer_command( CMPT_MSG_TYPE_REPORT, RESULT_VOTE, &vote_data, sizeof(tcmp_vote_result));

	return 0;
}

int upper_cmpt_terminal_message_report( void* p_tmnl_msg, uint16_t msg_len, uint16_t addr )
{
 	assert( p_tmnl_msg );
	tcmp_report_terminal_message tmnl_msg;
	memcpy( &tmnl_msg, p_tmnl_msg, msg_len );
		
	send_upper_computer_command(  CMPT_MSG_TYPE_REPORT, REPORT_ENDSTATION_MESSAGE, &tmnl_msg, msg_len );
	
	return 0;
}

/*==================================================
					������λ����������
====================================================*/
