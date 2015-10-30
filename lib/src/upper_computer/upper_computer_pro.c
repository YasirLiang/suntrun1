/*	upper_computer_pro.c
** 注: 
**	1. 若处理函数未完成，请注明函数处理流程未完成。
**	2. 若处理函数已经完成，但需要完善，请注明函数处理需完善。
**	3. 若处理函数已经完成，但在后期可能需要修改，请在修改后注明修改的时间与内容，并保留原来的处理过程。
** 	4. 若处理函数已经完成，则不用注明。
**
*/

#include "upper_computer_pro.h"
#include "profile_system.h"
#include "upper_computer_common.h"
#include "upper_computer_command.h"
#include "upper_computer_data_parser.h"
#include "terminal_pro.h"


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
					开始终端命令函数
====================================================*/

int proccess_upper_cmpt_discussion_parameter( uint16_t protocol_type, void *data, uint32_t data_len ) // 函数处理流程未完成
{
	FILE* fd = NULL;
	thost_system_set set_sys; // 系统配置文件的格式
	tcmpt_discuss_parameter qu_dis_para, set_dis_para;
	uint16_t send_data_len = 0; // 协议数据负载的长度
	
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
	// 设置保存系统状态值并设置系统状态
	else if((protocol_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET )
	{
		get_host_upper_cmpt_data( &set_dis_para, data, CMPT_DATA_OFFSET, sizeof(tcmpt_discuss_parameter));
		DEBUG_RECV( &set_dis_para, sizeof(tcmpt_discuss_parameter), "Dis Param ");

		// 保存配置文件
		if( profile_system_file_dis_param_save( fd, &set_dis_para ) != -1 )
		{
			// 设置系统状态
			uint8_t temp_status = set_dis_para.auto_close;
			if( temp_status != set_sys.auto_close )
			{
				// 自动关闭麦克风
				terminal_mic_auto_close( 0, NULL, 0 );
			}

			temp_status = set_dis_para.discuss_mode;
			if( temp_status != set_sys.discuss_mode )
			{
				// 设置系统模式
			}
			
			temp_status = set_dis_para.limit_speak_num;
			if( temp_status != set_sys.speak_limit )
			{
				// 限制发言人数
			}

			temp_status = set_dis_para.limit_apply_num;
			if( temp_status != set_sys.apply_limit )
			{
				// 设置限制申请人数
				
			}
			

			// 向终端发送限时消息
			//terminal_limit_speak_time_set();

			// 设置会议讨论状态
			

			// 发送响应
			send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE |CMPT_MSG_TYPE_SET, \
			DISCUSSION_PARAMETER, NULL, 0 ); // 第三个与第四个参数与协议有些出入，这里是根据黄工代码写的。协议是数据单元仅一个字节0，设置成功；非零设置失败。而黄工的没有数据单元，故这里写NULL
		}
		else
		{
			return -1; // 这里返回0 的目的是不让主机报错，
		}
	}

	Fclose( fd );

	return 0;
}

int proccess_upper_cmpt_microphone_switch( uint16_t protocal_type, void *data, uint32_t data_len )
{
	
	
	return 0;
}

/*麦克风状态*/ 
int proccess_upper_cmpt_miscrophone_status( uint16_t protocal_type, void *data, uint32_t data_len )// 后期可能需要修改10/30。
{
	assert( data );
	tmnl_pdblist tmnl_list_head = dev_terminal_list_guard;
	tmnl_pdblist p_tmnl_tmp = dev_terminal_list_guard->next;
	tdata_addr_and_flag mic_list[SYSTEM_TMNL_MAX_NUM]; // 每个终端的麦克风状态对应一个元素
	uint16_t addr_num = 0;

	if( (protocal_type & CMPT_MSG_TYPE_MARK) != CMPT_MSG_TYPE_QUERY)
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

	send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_QUERY, \
		MISCROPHONE_STATUS, mic_list, addr_num * sizeof(tdata_addr_and_flag));
	
	return 0;
}

int proccess_upper_cmpt_select_proposer( uint16_t protocal_type, void *data, uint32_t data_len )//函数处理流程未完成10/30。
{
	uint16_t addr = get_uint16_data_from_buf( data, CMPT_DATA_OFFSET );
	
	if( (protocal_type & CMPT_MSG_TYPE_MARK) != CMPT_MSG_TYPE_SET)
	{
		return -1;
	}

	/*处理选择申请人*/
	DEBUG_INFO( "select apply = %04x ", addr );
	//terminal_select_apply( addr );
	
	send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, SELECT_PROPOSER, NULL, 0 );
	
	return 0;
}
int proccess_upper_cmpt_examine_application( uint16_t protocal_type, void *data, uint32_t data_len )//函数处理流程未完成10/30。
{
	uint8_t exam_value = get_uint8_data_value_from_buf( data, CMPT_DATA_OFFSET );

	if( (protocal_type & CMPT_MSG_TYPE_MARK) != CMPT_MSG_TYPE_SET)
	{
		return -1;
	}
	
	/*处理审批申请*/
	DEBUG_INFO( "examine application value = %d ", exam_value );
	//terminal_examine_apply( exam_value );

	send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, EXAMINE_APPLICATION, NULL, 0 );
	
	return 0;
}

int proccess_upper_cmpt_conference_permission( uint16_t protocal_type, void *data, uint32_t data_len )//函数处理流程未完成10/30。
{
	uint16_t address = get_uint16_data_from_buf( data, CMPT_DATA_OFFSET );
	uint8_t set_value_id = get_uint8_data_value_from_buf( data, CMPT_DATA_OFFSET + sizeof(uint16_t));

	if( (protocal_type & CMPT_MSG_TYPE_MARK) != CMPT_MSG_TYPE_SET)
	{
		return -1;
	}
	
	/*处理会议权限*/
	DEBUG_INFO( "examine permit  = %d, addr = %d ", set_value_id, address );
	//terminal_examine_apply( set_value_id, address );

	send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, CONFERENCE_PERMISSION, NULL, 0 );
	return 0;
}

int proccess_upper_cmpt_senddown_message( uint16_t protocal_type, void *data, uint32_t data_len )//函数处理流程未完成10/30。
{
	uint16_t address = get_uint16_data_from_buf( data, CMPT_DATA_OFFSET );
	uint16_t lenght = get_uint16_data_from_buf( data, CMPT_DATA_LEN_OFFSET ); // 数据区长度
	tcmpt_data_message data_msg;
	
	if( (protocal_type & CMPT_MSG_TYPE_MARK) != CMPT_MSG_TYPE_SET)
	{
		return -1;
	}

	/*主机发送上位机信息*/
	conference_host_upper_computer_set_upper_message_form( &data_msg, data, CMPT_DATA_OFFSET, lenght );
	//terminal_send_upper_message( data_msg, address, lenght - sizeof(uint16_t) );

	send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, SENDDOWN_MESSAGE, NULL, 0 );
	
	return 0;
}

int proccess_upper_cmpt_tablet_stands_manager( uint16_t protocal_type, void *data, uint32_t data_len ) // 函数处理流程未完成10/30。
{
	assert( data );
	uint16_t address = get_uint16_data_from_buf( data, CMPT_DATA_OFFSET );
	uint16_t lenght = get_uint16_data_from_buf( data, CMPT_DATA_LEN_OFFSET ); // 数据区长度
	tcmpt_table_card table_card;

	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		conference_host_upper_computer_set_upper_message_form( &table_card, data, CMPT_DATA_LEN_OFFSET, lenght );

		/*管理终端的桌牌*/
		//terminal_tablet_stands_manager( table_card, address,  lenght - sizeof(uint16_t) ); // 注:这里不涉及到系统的所有状态的处理
		
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, TABLE_TABLET_STANDS_MANAGER, NULL, 0 );
	}
	else
	{
		return -1;
	}
	
	return 0;
}

int proccess_upper_cmpt__begin_sign( uint16_t protocal_type, void *data, uint32_t data_len ) // 函数处理流程未完成10/30。
{
	assert( data );
	tcmpt_begin_sign sign_flag;
	sign_flag.sign_type = (get_uint8_data_value_from_buf( data, CMPT_DATA_OFFSET )&0x03; // 低两位 00 按键；01插卡，其它保留
	sign_flag.retroactive_timeouts = get_uint8_data_value_from_buf( data, CMPT_DATA_OFFSET + 1);

	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		/*系统开始签到*/
		//terminal_start_sign_in( sign_flag ); 
		
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, BEGIN_SIGN, NULL, 0 );
	}
	else
	{
		return -1;
	}
	
	return 0;
}

/* 系统中终端的签到情况 */
int proccess_upper_cmpt_sign_situation( uint16_t protocal_type, void *data, uint32_t data_len )//后期可能需要修改10/30。
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
				sign_list[sign_num].addr.high_addr = (uint8_t)((p_tmnl_list->tmnl_dev.address.addr &0xff00) >> 8); // low addr
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

int proccess_upper_cmpt_end_of_sign( uint16_t protocal_type, void *data, uint32_t data_len )
{
	assert( data );
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		//terminal_start_sign_in( sign_flag ); 
		
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, END_OF_SIGN, NULL, 0 );
	}
	else
	{
		return -1;
	}
	
	return 0;
}

int proccess_upper_cmpt_endtation_allocation_address( uint16_t protocal_type, void *data, uint32_t data_len )
{
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		//terminal_start_sign_in( sign_flag ); 
		
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, BEGIN_SIGN, NULL, 0 );
	}
	else
	{
		return -1;
	}
	
	return 0;
}

int proccess_upper_cmpt_endstation_register_status( uint16_t protocal_type, void *data, uint32_t data_len )
{
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		//terminal_start_sign_in( sign_flag ); 
		
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, BEGIN_SIGN, NULL, 0 );
	}
	else
	{
		return -1;
	}
	
	return 0;
}

int proccess_upper_cmpt_current_vidicon( uint16_t protocal_type, void *data, uint32_t data_len )
{
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		//terminal_start_sign_in( sign_flag ); 
		
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, BEGIN_SIGN, NULL, 0 );
	}
	else
	{
		return -1;
	}
	
	return 0;
}

int proccess_upper_cmpt_endstation_address_undetermined_allocation( uint16_t protocal_type, void *data, uint32_t data_len )
{
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		//terminal_start_sign_in( sign_flag ); 
		
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, BEGIN_SIGN, NULL, 0 );
	}
	else
	{
		return -1;
	}
	
	return 0;
}

int proccess_upper_cmpt_vidicon_control( uint16_t protocal_type, void *data, uint32_t data_len )
{
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		//terminal_start_sign_in( sign_flag ); 
		
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, BEGIN_SIGN, NULL, 0 );
	}
	else
	{
		return -1;
	}
	
	return 0;
}

int proccess_upper_cmpt_vidicon_preration_set( uint16_t protocal_type, void *data, uint32_t data_len )
{
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		//terminal_start_sign_in( sign_flag ); 
		
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, BEGIN_SIGN, NULL, 0 );
	}
	else
	{
		return -1;
	}
	
	return 0;
}

int proccess_upper_cmpt_vidicon_lock( uint16_t protocal_type, void *data, uint32_t data_len )
{
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		//terminal_start_sign_in( sign_flag ); 
		
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, BEGIN_SIGN, NULL, 0 );
	}
	else
	{
		return -1;
	}
	
	return 0;
}

int proccess_upper_cmpt_vidicon_output( uint16_t protocal_type, void *data, uint32_t data_len )
{
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		//terminal_start_sign_in( sign_flag ); 
		
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, BEGIN_SIGN, NULL, 0 );
	}
	else
	{
		return -1;
	}
	
	return 0;
}

int proccess_upper_cmpt_cmpt_begin_vote( uint16_t protocal_type, void *data, uint32_t data_len )
{
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		//terminal_start_sign_in( sign_flag ); 
		
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, BEGIN_SIGN, NULL, 0 );
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
		//terminal_start_sign_in( sign_flag ); 
		
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, BEGIN_SIGN, NULL, 0 );
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
		//terminal_start_sign_in( sign_flag ); 
		
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, BEGIN_SIGN, NULL, 0 );
	}
	else
	{
		return -1;
	}
	
	return 0;
}

int proccess_upper_cmpt_end_vote( uint16_t protocal_type, void *data, uint32_t data_len )
{
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		//terminal_start_sign_in( sign_flag ); 
		
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, BEGIN_SIGN, NULL, 0 );
	}
	else
	{
		return -1;
	}
	
	return 0;
}

/*表决结果*/ 
int proccess_upper_cmpt_result_vote( uint16_t protocal_type, void *data, uint32_t data_len )//后期可能需要修改10/30。
{
	assert( dev_terminal_list_guard );
	tmnl_pdblist tmnl_list_head = dev_terminal_list_guard;
	tmnl_pdblist p_tmnl_tmp = dev_terminal_list_guard->next;
	tcmp_vote_result vote_result[SYSTEM_TMNL_MAX_NUM]; // 
	uint16_t addr_num = 0;
	uint16_t data_len = 0;
	
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
			vote_result[addr_num].key_value= p_tmnl_tmp->tmnl_dev.tmnl_status.vote_state & CMPT_VOTE_KEY_MARK;
			addr_num++;
		}
	}

	data_len = sizeof(tcmp_vote_result) * addr_num;
	DEBUG_INFO( "vote result data len = %d",  data_len );
	send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE |CMPT_MSG_TYPE_SET, RESULT_VOTE, vote_result, data_len );
	
	return 0;
}

int proccess_upper_cmpt_transmit_to_endstation( uint16_t protocal_type, void *data, uint32_t data_len )
{
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		//terminal_start_sign_in( sign_flag ); 
		
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, BEGIN_SIGN, NULL, 0 );
	}
	else
	{
		return -1;
	}
	
	return 0;
}

int proccess_upper_cmpt_report_endstation_message( uint16_t protocal_type, void *data, uint32_t data_len )
{
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		//terminal_start_sign_in( sign_flag ); 
		
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, BEGIN_SIGN, NULL, 0 );
	}
	else
	{
		return -1;
	}
	
	return 0;
}

int proccess_upper_cmpt_hign_definition_switch_set( uint16_t protocal_type, void *data, uint32_t data_len )
{
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		//terminal_start_sign_in( sign_flag ); 
		
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, BEGIN_SIGN, NULL, 0 );
	}
	else
	{
		return -1;
	}
	
	return 0;
}

/*==================================================
					结束终端命令函数
====================================================*/

