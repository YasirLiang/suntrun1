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
#include "terminal_command.h"
#include "util.h"
#include "upper_computer_data_form.h"
#include "system_packet_tx.h"
#include "terminal_system.h"
#include "camera_pro.h"
#include "camera_output.h"

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

int profile_dis_param_save_to_ram( thost_system_set *set_sys, tcmpt_discuss_parameter *set_dis_para )
{
	assert( set_sys && set_dis_para );
	
	set_sys->chman_first = set_dis_para->chairman_first;
	set_sys->chman_music = set_dis_para->chair_music;
	set_sys->auto_close = set_dis_para->auto_close;
	set_sys->discuss_mode = set_dis_para->discuss_mode;
	set_sys->speak_limit = set_dis_para->limit_speak_num;
	set_sys->apply_limit = set_dis_para->limit_apply_num;
	set_sys->chman_limitime = set_dis_para->limit_chm_time;
	set_sys->vip_limitime = set_dis_para->limit_vip_time;
	set_sys->spk_limtime = set_dis_para->limit_speak_time;

	return 0;
}

/*==================================================
					开始终端命令函数
====================================================*/

int proccess_upper_cmpt_discussion_parameter( uint16_t protocol_type, void *data, uint32_t data_len ) //后期可能需要修改11/10。
{
	tcmpt_discuss_parameter qu_dis_para, set_dis_para;
	uint16_t send_data_len = 0; // 协议数据负载的长度
	thost_system_set set_sys; // 系统配置文件的格式
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));

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
			DISCUSSION_PARAMETER, &qu_dis_para, send_data_len );
	}
	else if((protocol_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET )// 设置保存系统状态值并设置系统状态
	{	
		get_host_upper_cmpt_data( &set_dis_para, data, CMPT_DATA_OFFSET, sizeof(tcmpt_discuss_parameter));
		DEBUG_RECV( &set_dis_para, sizeof(tcmpt_discuss_parameter), "Dis Param ");

		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE |CMPT_MSG_TYPE_SET, \
				DISCUSSION_PARAMETER, NULL, 0 ); // 第三个与第四个参数与协议有些出入，这里是根据黄工代码写的。协议是数据单元仅一个字节0，设置成功；非零设置失败。而黄工的没有数据单元，故这里写NULL
				
		// 保存配置文件
		if( (profile_dis_param_save_to_ram( &gset_sys, &set_dis_para ) != -1) && (-1 != profile_system_file_write_gb_param( profile_file_fd, &gset_sys )) )
		{
			Fflush( profile_file_fd );
			// 设置系统状态
			uint8_t temp_status = set_dis_para.auto_close;
			if( temp_status != set_sys.auto_close )
			{
				// 自动关闭麦克风
				find_func_command_link( MENU_USE, MENU_AUTO_CLOSE_CMD, 0, NULL, 0 );
			}

			temp_status = set_dis_para.discuss_mode;
			if( temp_status != set_sys.discuss_mode )
			{
				// 设置系统讨论模式
				find_func_command_link( MENU_USE, MENU_DISC_MODE_SET_CMD, 0, &temp_status, 1 );
			}
			
			temp_status = set_dis_para.limit_speak_num;
			if( temp_status != set_sys.speak_limit )
			{
				// 设置限制的发言人数
				find_func_command_link( MENU_USE, MENU_SPK_LIMIT_NUM_SET, 0, &temp_status, 1 );
			}

			temp_status = set_dis_para.limit_apply_num;
			if( temp_status != set_sys.apply_limit )
			{
				// 设置限制申请人数
				find_func_command_link( MENU_USE, MUNU_APPLY_LIMIT_NUM_SET, 0, &temp_status, 1 );
			}

			// 限时设置
			terminal_limit_speak_time_set( 0, NULL, 0 );
			
			// 设置会议讨论状态
			terminal_start_discuss( false );
		}
	}
	
	return 0;
}

int proccess_upper_cmpt_microphone_switch( uint16_t protocal_type, void *data, uint32_t data_len )//后期可能需要修改11/10。
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

/*麦克风状态*/ 
int proccess_upper_cmpt_miscrophone_status( uint16_t protocal_type, void *data, uint32_t data_len )//后期可能需要修改11/10。
{
	assert( data );
	tmnl_pdblist tmnl_list_head = dev_terminal_list_guard;
	tmnl_pdblist p_tmnl_tmp = dev_terminal_list_guard->next;
	tdata_addr_and_flag mic_list[SYSTEM_TMNL_MAX_NUM]; // 每个终端的麦克风状态对应一个元素
	uint16_t addr_num = 0;

	if( (protocal_type & CMPT_MSG_TYPE_MARK) != CMPT_MSG_TYPE_QUERY )
	{
		return -1;
	}

	for( ; p_tmnl_tmp != tmnl_list_head; p_tmnl_tmp = p_tmnl_tmp->next )
	{
		
		if( p_tmnl_tmp->tmnl_dev.address.addr != 0xffff && p_tmnl_tmp->tmnl_dev.tmnl_status.is_rgst )
		{
			if( addr_num < SYSTEM_TMNL_MAX_NUM )// not = SYSTEM_TMNL_MAX_NUM
			{
				uint8_t low_addr = (uint8_t)((p_tmnl_tmp->tmnl_dev.address.addr &0x00ff) >> 0);
				uint8_t high_addr =  (uint8_t)((p_tmnl_tmp->tmnl_dev.address.addr &0xff00) >> 8);
				uint8_t mic_state = p_tmnl_tmp->tmnl_dev.tmnl_status.mic_state;
				mic_list[addr_num].addr_flag[0] = low_addr;
				mic_list[addr_num].addr_flag[1] = high_addr;
				mic_list[addr_num].addr_flag[2] = mic_state;
				addr_num++;
			}
			else
			{
				DEBUG_INFO( "mic num bigger system terminal num!" );\
				return -1;
			}
		}
	}
	
	DEBUG_INFO( "sizeof tdata_addr_and_flag = %d, addr_num = %d", sizeof(tdata_addr_and_flag), addr_num );
	send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_QUERY, MISCROPHONE_STATUS, addr_num>0?mic_list:NULL, addr_num*sizeof(tdata_addr_and_flag) );
	
	return 0;
}

int proccess_upper_cmpt_select_proposer( uint16_t protocal_type, void *data, uint32_t data_len )//后期可能需要修改11/9。
{
	uint16_t addr = get_uint16_data_from_buf( data, CMPT_DATA_OFFSET );
	
	if( (protocal_type & CMPT_MSG_TYPE_MARK) != CMPT_MSG_TYPE_SET )
	{
		return -1;
	}

	/*处理选择申请人*/
	DEBUG_INFO( "select apply = %04x ", addr );
	terminal_select_apply( addr ); // 使选择的申请人是首位申请人
	
	send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, SELECT_PROPOSER, NULL, 0 );
	
	return 0;
}
int proccess_upper_cmpt_examine_application( uint16_t protocal_type, void *data, uint32_t data_len )//后期可能需要修改11/10。
{
	uint8_t exam_value = get_uint8_data_value_from_buf( data, CMPT_DATA_OFFSET );

	if( (protocal_type & CMPT_MSG_TYPE_MARK) != CMPT_MSG_TYPE_SET)
	{
		return -1;
	}
	
	/*处理审批申请*/
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

int proccess_upper_cmpt_conference_permission( uint16_t protocal_type, void *data, uint32_t data_len )//后期可能需要修改11/10。
{
	tcmpt_data_meeting_authority tmnl_type;
	uint16_t len_data_get = get_host_upper_cmpt_data_len( data, CMPT_HEAD_OFFSET );
	get_host_upper_cmpt_data( &tmnl_type, data, CMPT_DATA_OFFSET, len_data_get );
	
	if( (protocal_type & CMPT_MSG_TYPE_MARK) != CMPT_MSG_TYPE_SET )
	{
		return -1;
	}
	
	/*处理会议权限*/
	DEBUG_INFO( "examine permit  = %d, addr = 0x%02x%02x ", tmnl_type.identity, tmnl_type.addr.high_addr, tmnl_type.addr.low_addr );
	terminal_type_set( tmnl_type );

	send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, CONFERENCE_PERMISSION, NULL, 0 );
	
	return 0;
}

int proccess_upper_cmpt_senddown_message( uint16_t protocal_type, void *data, uint32_t data_len )//后期可能需要修改11/10。
{
	uint16_t address = get_uint16_data_from_buf( data, CMPT_DATA_OFFSET );
	uint16_t lenght = get_uint16_data_from_buf( data, CMPT_DATA_LEN_OFFSET ); // 数据区长度
	tcmpt_data_message data_msg;
	
	if( (protocal_type & CMPT_MSG_TYPE_MARK) != CMPT_MSG_TYPE_SET)
	{
		return -1;
	}

	send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, SENDDOWN_MESSAGE, NULL, 0 );

	/*主机发送上位机信息*/
	conference_host_upper_computer_set_upper_message_form( &data_msg, data, CMPT_DATA_OFFSET, lenght );
	terminal_send_upper_message( ((uint8_t*)&data_msg) + sizeof(uint16_t), address, lenght - sizeof(uint16_t) );
	
	return 0;
}

int proccess_upper_cmpt_tablet_stands_manager( uint16_t protocal_type, void *data, uint32_t data_len )//后期可能需要修改11/10。
{
	assert( data );
	uint16_t address = get_uint16_data_from_buf( data, CMPT_DATA_OFFSET );
	uint16_t lenght = get_uint16_data_from_buf( data, CMPT_DATA_LEN_OFFSET ); // 数据区长度
	tcmpt_table_card table_card;

	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, TABLE_TABLET_STANDS_MANAGER, NULL, 0 );
		
		/*管理终端的桌牌*/
		upper_computer_table_card_message_form( &table_card, data, CMPT_DATA_OFFSET, lenght );
		terminal_tablet_stands_manager( &table_card, address, lenght ); // 注:这里不涉及到系统的所有状态的处理
	}

	return 0;
}

int proccess_upper_cmpt__begin_sign( uint16_t protocal_type, void *data, uint32_t data_len ) //后期可能需要修改11/10。
{
	assert( data );
	tcmpt_begin_sign sign_flag;
	sign_flag.sign_type = (get_uint8_data_value_from_buf( data, CMPT_DATA_OFFSET )&0x03); // 低两位 00 按键；01插卡，其它保留
	sign_flag.retroactive_timeouts = get_uint8_data_value_from_buf( data, CMPT_DATA_OFFSET + 1);

	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, BEGIN_SIGN, NULL, 0 );
		
		/*系统开始签到*/
		terminal_start_sign_in( sign_flag ); 
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
				sign_list[sign_num].addr.high_addr = (uint8_t)((p_tmnl_list->tmnl_dev.address.addr &0xff00) >> 8); // ligh addr
				sign_list[sign_num].sign_situation = p_tmnl_list->tmnl_dev.tmnl_status.sign_state;
				sign_num++;
			}
		}

		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_QUERY, SIGN_SITUATION,\
			sign_list, sign_num* sizeof(tcmpt_sign_situation));
	}
	
	return 0;
}

int proccess_upper_cmpt_end_of_sign( uint16_t protocal_type, void *data, uint32_t data_len )//后期可能需要修改11/10。
{
	assert( data );
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, END_OF_SIGN, NULL, 0 );
		
		/*处理结束签到*/
		terminal_end_sign( 0, NULL, 0 ); 
	}
	
	return 0;
}

int proccess_upper_cmpt_endtation_allocation_address( uint16_t protocal_type, void *data, uint32_t data_len )//后期可能需要修改11/2。
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

int proccess_upper_cmpt_endstation_register_status( uint16_t protocal_type, void *data, uint32_t data_len )//后期可能需要修改11/2。
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

	if( register_num <= SYSTEM_TMNL_MAX_NUM )
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_QUERY, ENDSTATION_REGISTER_STATUS, regist_list, register_num * sizeof(tcmpt_end_register));
	
	return 0;
}

int proccess_upper_cmpt_current_vidicon( uint16_t protocal_type, void *data, uint32_t data_len )// 后期可能需要修改12-8
{
	tcmp_current_camara curcmr;
	
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET )
	{
		upper_cmpt_current_cmrnum_get( data, &curcmr, CMPT_DATA_OFFSET, 0 );
		curcmr.camara_num += 1; // change;2016-1-21
		camera_select_num( 0, &curcmr, sizeof(uint8_t) );
		
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, CURRENT_VIDICON, NULL, 0 );
	}
	else
	{
		curcmr.camara_num = gcurpresetcmr.camera_num -1;
		
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, CURRENT_VIDICON, &curcmr, sizeof(uint8_t));
	}
	
	return 0;
}

int proccess_upper_cmpt_endstation_address_undetermined_allocation( uint16_t protocal_type, void *data, uint32_t data_len ) // 后期可能需要修改12-8
{
	tcmp_wait_allocation wait_allot;
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		upper_cmpt_camera_wait_allot_get( data, &wait_allot, CMPT_DATA_OFFSET, 0 );

		// 处理等待定位的地址
		find_func_command_link( SYSTEM_USE, SYS_PRESET_ADDR, 0, (uint8_t*)&wait_allot.addr, sizeof(uint16_t) );

		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, \
			ENDSTATION_ADDRESS_UNDETERMINED_ALLOCATION, NULL, 0 );
	}
	
	return 0;
}

int proccess_upper_cmpt_vidicon_control( uint16_t protocal_type, void *data, uint32_t data_len )// 后期可能需要修改12-8
{
	tcmp_camara_ctl cmr_data;
	uint8_t camera_address = 0;
	uint16_t d_cmd = 0;
	uint8_t speed_lv = 0;
	uint8_t speed_vertical = 0;
	
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET )
	{
		upper_cmpt_camera_controller_get( data, &cmr_data, CMPT_DATA_OFFSET, 0 );
		camera_address = gcurpresetcmr.camera_num; 
		upper_computer_set_camera_d_command( &d_cmd , &cmr_data );
		speed_lv = cmr_data.level_speed;
		speed_vertical = cmr_data.vertical_speed;
		camera_pro_control( camera_address, d_cmd, speed_lv, speed_vertical );
		
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, \
			VIDICON_CONTROL, NULL, 0 );
	}
	
	return 0;
}

int proccess_upper_cmpt_vidicon_preration_set( uint16_t protocal_type, void *data, uint32_t data_len )// 函数处理流程未完成12/8。
{
	tcmp_camara_bit_preset prs;
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET,\
			VIDICON_PRERATION_SET, NULL, 0 );
		
		upper_cmpt_camera_preset_flag_get( data, &prs, CMPT_DATA_OFFSET, 0 );
		switch(prs.camara_preset_flags)
		{
			case SAVE_PRESET_BIT:
				if( CAMERA_PRESET == get_sys_state())
				{
					camera_preset_save( 0, NULL, 0 );
				}
				break;
			case ENTER_PRESET_BIT:
				camera_pro_enter_preset();
				break;
			case EXIT_PRESET_BIT:
				camera_pro_esc_preset();
				break;
			case SAVE_CAMARA_ALLOCATION_ADJUST:
				camera_save_Cmrpreset_direct();
				break;
			default:
				break;
		}
	}
	
	return 0;
}

int proccess_upper_cmpt_vidicon_lock( uint16_t protocal_type, void *data, uint32_t data_len ) // 后期可能需要修改12-8
{
	tcmp_camara_lock cmr_lock;
	
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		upper_cmpt_camera_lock_flag_get( data, &cmr_lock, CMPT_DATA_OFFSET, 0 );
		camera_pro_lock_flags( cmr_lock.camara_lock_flags );
		
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET,\
			VIDICON_LOCK, NULL, 0 );
	}
	
	return 0;
}

int proccess_upper_cmpt_vidicon_output( uint16_t protocal_type, void *data, uint32_t data_len )// 函数处理流程未完成12/8。
{
	tcmp_camara_output out_chn;
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		upper_cmpt_camera_output_flag_get( data, &out_chn, CMPT_DATA_OFFSET, 0 );
		if( out_chn.camara_output_1 )
		{
			camera_output_switch( out_chn.camara_output_1, 1, true );
		}

		if( out_chn.camara_output_2 )
		{
			camera_output_switch( out_chn.camara_output_2, 2, true );
		}
		
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, \
			VIDICON_OUTPUT, NULL, 0 );
	}
	else if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_QUERY )
	{
		out_chn.camara_output_1 = camera_output_get_input(CAMERA_OUT_FULL_VIEW);
		out_chn.camara_output_2 = camera_output_get_input(CAMERA_OUT_TRACK_VIEW);
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_QUERY, \
			VIDICON_OUTPUT, (uint8_t*)&out_chn, sizeof(uint16_t) );
	}
	
	return 0;
}

int proccess_upper_cmpt_cmpt_begin_vote( uint16_t protocal_type, void *data, uint32_t data_len )//后期可能需要修改11/10。
{
	tcmp_vote_start vote_start_flag;
	uint8_t sign_flag = 0, recv_data; // 响应的签到标志
	recv_data = get_uint8_data_value_from_buf( data, CMPT_DATA_OFFSET );
	vote_start_flag.vote_type = recv_data & 0x0f;
	vote_start_flag.key_effective = (recv_data & 0x10 )?1:0;
	DEBUG_INFO( " recv vote data = 0x%02x, key effective = %d, vote type = %d", \
		recv_data, vote_start_flag.key_effective, vote_start_flag.vote_type );
	
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET )
	{ 	
		terminal_begin_vote( vote_start_flag,  &sign_flag );
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, \
			BEGIN_VOTE, &sign_flag, sizeof(uint8_t));
	}
	
	return 0;
}

int proccess_upper_cmpt_pause_vote( uint16_t protocal_type, void *data, uint32_t data_len )
{
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		terminal_pause_vote( 0, NULL, 0 );
		
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET,\
			PAUSE_VOTE, NULL, 0 );
	}
	
	return 0;
}

int proccess_upper_cmpt_regain_vote( uint16_t protocal_type, void *data, uint32_t data_len )
{
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		terminal_regain_vote( 0, NULL, 0 );
		
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET,\
			REGAIN_VOTE, NULL, 0 );
	}
	
	return 0;
}

int proccess_upper_cmpt_end_vote( uint16_t protocal_type, void *data, uint32_t data_len )//后期可能需要修改11/10。
{
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		/*设置结束投票标志*/
		terminal_end_vote( 0, NULL, 0); 
		send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET,\
			END_VOTE, NULL, 0 );

		/*设置讨论模式的开始标志*/
		set_terminal_system_state( DISCUSS_STATE, true );
		terminal_start_discuss(false);
	}
	
	return 0;
}

/*表决结果*/ 
int proccess_upper_cmpt_result_vote( uint16_t protocal_type, void *data, uint32_t data_len )//后期可能需要修改11/10。
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
		if( addr_num < SYSTEM_TMNL_MAX_NUM )
		{
			if((p_tmnl_tmp->tmnl_dev.address.addr != 0xffff) && (p_tmnl_tmp->tmnl_dev.tmnl_status.is_rgst))
			{
				vote_result[addr_num].addr.low_addr = (uint8_t)((p_tmnl_tmp->tmnl_dev.address.addr &0x00ff) >> 0); // low addr
				vote_result[addr_num].addr.high_addr = (uint8_t)((p_tmnl_tmp->tmnl_dev.address.addr &0xff00) >> 8); // hight addr
				vote_result[addr_num].key_value= p_tmnl_tmp->tmnl_dev.tmnl_status.vote_state & TVOTE_KEY_MARK;
				addr_num++;
			}
		}
	}

	send_data_len = sizeof(tcmp_vote_result) * addr_num;
	DEBUG_INFO( "vote result data len = %d",  data_len );
	send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE |CMPT_MSG_TYPE_QUERY,\
		RESULT_VOTE, vote_result, send_data_len );
	
	return 0;
}

int proccess_upper_cmpt_transmit_to_endstation( uint16_t protocal_type, void *data, uint32_t data_len )// 函数处理流程未完成11/10。
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
	
	return 0;
}

/*注:此函数用于上报终端短信息，不是对接受命令数据进行处理的函数,data 是命令数据的数据区的指针，data_len 是数据区数据的长度*/
int proccess_upper_cmpt_report_endstation_message( uint16_t protocal_type, void *data, uint32_t data_len )//后期可能需要修改11/10。
{
	assert( data );
	tcmp_report_terminal_message tmnl_msg;
	memcpy( &tmnl_msg, data, data_len );
	
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_REPORT)
	{	
		send_upper_computer_command(  CMPT_MSG_TYPE_REPORT, REPORT_ENDSTATION_MESSAGE,\
			&tmnl_msg, data_len );
	}
	
	return 0;
}

// 高清摄像头开关设置
int proccess_upper_cmpt_hign_definition_switch_set( uint16_t protocal_type, void *data, uint32_t data_len )// 函数处理流程未完成11/2。
{
	assert( data );
	
	if( (protocal_type & CMPT_MSG_TYPE_MARK) == CMPT_MSG_TYPE_SET)
	{
		//terminal_start_sign_in( sign_flag ); 
		
		//send_upper_computer_command( CMPT_MSG_TYPE_RESPONSE | CMPT_MSG_TYPE_SET, BEGIN_SIGN, NULL, 0 );
	}
	
	return 0;
}

/*==================================================
					结束终端命令函数
====================================================*/

/*==================================================
					上位机处理函数流程
====================================================*/
/*上报单个麦克风状态*/
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
	tdata_addr_and_flag mic_list[SYSTEM_TMNL_MAX_NUM]; // 每个终端的麦克风状态对应一个元素
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

/*上报终端签到情况*/
int upper_cmpt_report_sign_in_state( uint8_t sign_status, uint16_t addr )
{
	tcmpt_sign_situation sign_flag_station;

	if( (TMNL_NO_SIGN_IN == sign_status) ||(TMNL_SIGN_ON_TIME == sign_status) || (TMNL_SIGN_BE_LATE == sign_status) )
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

/*上报终端签到情况(不需要响应 2016/1/29)*/
int upper_cmpt_report_sign_in_state_as_response( uint8_t sign_status, uint16_t addr )
{
	tcmpt_sign_situation sign_flag_station;

	if( (TMNL_NO_SIGN_IN == sign_status) ||(TMNL_SIGN_ON_TIME == sign_status) || (TMNL_SIGN_BE_LATE == sign_status) )
	{
		sign_flag_station.addr.low_addr = (uint8_t)((addr &0x00ff) >> 0);
		sign_flag_station.addr.high_addr = (uint8_t)((addr &0xff00) >> 8);
		sign_flag_station.sign_situation = sign_status;
		send_upper_computer_command( CMPT_MSG_TYPE_REPORT |CMPT_MSG_TYPE_RESPONSE, \
			SIGN_SITUATION, &sign_flag_station, sizeof(tcmpt_sign_situation));
	}
	else
	{
		DEBUG_INFO( "not valid rang sign flag!" );
		return -1;
	}

	return 0;
}

/*上报终端签到情况 2016/1/29*/
void proccess_upper_cmpt_sign_state_list( void )
{
	assert( dev_terminal_list_guard );
	if( dev_terminal_list_guard == NULL )
	{
		return;
	}
	
	tmnl_pdblist tmnl_list_head = dev_terminal_list_guard;
	tmnl_pdblist p_tmnl_list = dev_terminal_list_guard->next;
	tcmpt_sign_situation sign_list[SYSTEM_TMNL_MAX_NUM];
	uint16_t sign_num = 0;
	
	for( ; p_tmnl_list != tmnl_list_head; p_tmnl_list = p_tmnl_list->next )
	{
		if( sign_num < SYSTEM_TMNL_MAX_NUM )
		{
			if( p_tmnl_list->tmnl_dev.address.addr != 0xffff && p_tmnl_list->tmnl_dev.tmnl_status.is_rgst ) 
			{
				sign_list[sign_num].addr.low_addr = (uint8_t)((p_tmnl_list->tmnl_dev.address.addr &0x00ff) >> 0); // low addr
				sign_list[sign_num].addr.high_addr = (uint8_t)((p_tmnl_list->tmnl_dev.address.addr &0xff00) >> 8); // ligh addr
				sign_list[sign_num].sign_situation = p_tmnl_list->tmnl_dev.tmnl_status.sign_state;
				sign_num++;
			}
		}
	}

	send_upper_computer_command( CMPT_MSG_TYPE_REPORT, SIGN_SITUATION,\
		sign_list, sign_num* sizeof(tcmpt_sign_situation));
	
}


// 上报终端投票情况
int upper_cmpt_vote_situation_report( uint8_t vote_rlst, uint16_t addr )
{
	tcmp_vote_result vote_data;

	if( addr != 0xffff )
	{
		vote_data.addr.low_addr = (uint8_t)((addr &0x00ff) >> 0); // low addr
		vote_data.addr.high_addr = (uint8_t)(( addr &0xff00) >> 8); // hight addr
		vote_data.key_value= vote_rlst;
	}
	
	send_upper_computer_command( CMPT_MSG_TYPE_REPORT, RESULT_VOTE, &vote_data, sizeof(tcmp_vote_result));

	return 0;
}


// 上报终端投票情况(不需要响应 2016/1/29)
int upper_cmpt_vote_situation_report_as_response( uint8_t vote_rlst, uint16_t addr )
{
	tcmp_vote_result vote_data;

	if( addr != 0xffff )
	{
		vote_data.addr.low_addr = (uint8_t)((addr &0x00ff) >> 0); // low addr
		vote_data.addr.high_addr = (uint8_t)(( addr &0xff00) >> 8); // hight addr
		vote_data.key_value= vote_rlst;
	}
	
	send_upper_computer_command( CMPT_MSG_TYPE_REPORT|CMPT_MSG_TYPE_RESPONSE, RESULT_VOTE, &vote_data, sizeof(tcmp_vote_result));

	return 0;
}

int upper_cmpt_terminal_message_report( void* p_tmnl_msg, uint16_t msg_len, uint16_t addr )
{
 	assert( p_tmnl_msg );
	tcmp_report_terminal_message tmnl_msg;
	memcpy( &tmnl_msg, p_tmnl_msg, msg_len ); // msg_len需要长度判断
		
	send_upper_computer_command(  CMPT_MSG_TYPE_REPORT, REPORT_ENDSTATION_MESSAGE, &tmnl_msg, msg_len );
	
	return 0;
}

int upper_computer_set_camera_d_command( uint16_t* d_cmd, tcmp_camara_ctl* cmr_data )
{
	assert( d_cmd && cmr_data );
	if( d_cmd == NULL || cmr_data == NULL )
	{
		return -1;
	}

	*d_cmd &= 0x0000;
	*d_cmd |= ((uint16_t)cmr_data->close_focus_flags << 0);
	*d_cmd |= ((uint16_t)cmr_data->aperture_amplification_flags << 1);
	*d_cmd |= ((uint16_t)cmr_data->aperture_shrinks_flags << 2);
	*d_cmd |= ((uint16_t)cmr_data->right_flags << 9);
	*d_cmd |= ((uint16_t)cmr_data->left_flags << 10);
	*d_cmd |= ((uint16_t)cmr_data->up_flags << 11);
	*d_cmd |= ((uint16_t)cmr_data->down_flags << 12);
	*d_cmd |= ((uint16_t)cmr_data->close_object << 13);
	*d_cmd |= ((uint16_t)cmr_data->remote_object << 14);
	*d_cmd |= ((uint16_t)cmr_data->remote_focus_flags << 15);
	
	return 0;
}
/*==================================================
					结束上位机处理流程
====================================================*/

