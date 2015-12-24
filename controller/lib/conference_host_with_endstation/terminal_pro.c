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
#include "file_util.h"
#include "conference_end_to_host.h"
#include "profile_system.h"
#include "terminal_command.h"
#include "terminal_system.h"
#include "upper_computer_pro.h"
#include "camera_pro.h"

FILE* addr_file_fd = NULL; 								// 终端地址信息读取文件描述符
terminal_address_list tmnl_addr_list[SYSTEM_TMNL_MAX_NUM];	// 终端地址分配列表
terminal_address_list_pro allot_addr_pro;	
tmnl_pdblist dev_terminal_list_guard = NULL; 				// 终端链表表头结点，对其正确地操作，必须先注册完终端
bool reallot_flag = false; 									// 重新分配标志
tmnl_state_set gtmnl_state_opt[TMNL_TYPE_NUM];
tsys_discuss_pro gdisc_flags; 								// 系统讨论参数
tchairman_control_in gchm_int_ctl; 						// 主席插话
ttmnl_register_proccess gregister_tmnl_pro; 					// 终端报到处理
uint8_t speak_limit_time = 0; 								// 发言时长， 0表示无限时；1-63表示限时1-63分钟
uint8_t glcd_num = 0; 									// lcd 显示的屏号
uint8_t gled_buf[2] = {0}; 								// 终端指示灯
enum_signstate gtmnl_signstate;							// 签到的状态，也可为终端的签到状态
uint8_t gsign_latetime; 									// 补签的超时时间
uint8_t gsigned_flag = false;								// 签到标志
evote_state_pro gvote_flag; 								// 投票处理
bool gfirst_key_flag; 									// 真为投票首键有效
tevote_type gvote_mode;									// 投票模式

type_spktrack gspeaker_track;

void init_terminal_proccess_fd( FILE ** fd )
{
	*fd = Fopen( ADDRESS_FILE, "ab+" );
	if( NULL == *fd )
	{
		DEBUG_ERR( "init terminal addr fd Err!" );
		assert( NULL != *fd );
	}
}

/*==============================================
初始化终端地址列表
================================================*/
int init_terminal_address_list_from_file( void )
{
	int i = 0;
	int ret = 0;
	
	// 初始化全局变量tmnl_addr_list
	memset( tmnl_addr_list, 0, sizeof(tmnl_addr_list) );
	for( i = 0; i < SYSTEM_TMNL_MAX_NUM; i++ )
	{
		tmnl_addr_list[i].addr = INIT_ADDRESS;
		tmnl_addr_list[i].tmn_type = TMNL_TYPE_COMMON_RPRST;
	}

	// 读终端地址信息到tmnl_addr_list, 若读取失败，则系统需要重新分配终端地址
	ret = terminal_address_list_read_file( addr_file_fd, tmnl_addr_list );
	if( ret == -1 )
	{
		DEBUG_INFO( "init tmnl_addr_list from address file!need to reallot terminal address");
		reallot_flag = true;
	}

	return ret;
}
void init_terminal_address_list( void )
{
	int i = 0;
	
	// 初始化全局变量tmnl_addr_list
	memset( tmnl_addr_list, 0, sizeof(tmnl_addr_list) );
	for( i = 0; i < SYSTEM_TMNL_MAX_NUM; i++ )
	{
		tmnl_addr_list[i].addr = INIT_ADDRESS;
		tmnl_addr_list[i].tmn_type = TMNL_TYPE_COMMON_RPRST;
	}
}


/*==============================================
结束初始化终端地址列表
================================================*/


inline void init_terminal_allot_address( void )
{
	allot_addr_pro.addr_start = 0;
	allot_addr_pro.index = 0;
	allot_addr_pro.renew_flag= 0;
}

inline void init_terminal_device_double_list( void )
{
	// init terminal system double list
	init_terminal_dblist( &dev_terminal_list_guard );
	assert( dev_terminal_list_guard != NULL );
}

/*
*date:2015-11-6
*/
int init_terminal_discuss_param( void ) 
{
	thost_system_set set_sys; // 系统配置文件的格式
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));

	gdisc_flags.apply_limit = set_sys.apply_limit;
	gdisc_flags.limit_num = set_sys.speak_limit;
	gdisc_flags.currect_first_index = set_sys.apply_limit;
	gdisc_flags.apply_num = 0;
	gdisc_flags.speak_limit_num = 0; // 发言人数
	gdisc_flags.edis_mode = (ttmnl_discuss_mode)set_sys.discuss_mode;
	memset( gdisc_flags.speak_addr_list, 0xffff, MAX_LIMIT_SPK_NUM );
	memset( gdisc_flags.apply_addr_list, 0xffff, MAX_LIMIT_APPLY_NUM );

	return 0;
}

void terminal_speak_track_pro_init( void )
{
	int i = 0;

	gspeaker_track.spk_num = 0;
	for( i = 0; i < MAX_SPK_NUM;i++ )
	{
		gspeaker_track.spk_addrlist[i] = 0xffff;
	}
}

#ifdef __DEBUG__  // 模拟终端信息
#define WRITE_ADDR_NUM 10
// 测试接口的格式:test_interface_ + 实际会用到的接口
void 	test_interface_terminal_address_list_write_file( FILE** fd )
{
	Fclose( *fd );
	*fd = Fopen( ADDRESS_FILE, "wb+" );
	if( *fd == NULL )
	{
		DEBUG_ERR( "init terminal addr fd Err!" );
		assert( NULL != *fd );
	}

	// write info
	int i = 0;
	for( ; i < WRITE_ADDR_NUM; i++ )
	{
		terminal_address_list tmp_addr;
		tmp_addr.addr = i;
		tmp_addr.tmn_type = 0;
		terminal_address_list_write_file( *fd, &tmp_addr, 1 );
	}
}

void print_out_terminal_addr_infomation( terminal_address_list* p, int num )
{
	int i = 0;

	printf( "Addr Info:\n" );
	for( ; i < num; i++ )
	{
		printf( "[ (addr-type)-> (%d -%d) ]\n", p[i].addr, p[i].tmn_type );
	}
}
#endif

void init_terminal_proccess_system( void )
{
	int tmnl_count = 0;
	init_terminal_proccess_fd(  &addr_file_fd );
	if( NULL == addr_file_fd )
		return;
#if 0	
#ifdef __DEBUG__ // 模拟终端信息数据
	test_interface_terminal_address_list_write_file( &addr_file_fd );
#endif
#endif

	if( NULL != addr_file_fd )
	{
		tmnl_count = init_terminal_address_list_from_file();
		if( tmnl_count != -1)
			DEBUG_INFO( "terminal count num = %d", tmnl_count );
		Fclose( addr_file_fd ); // 关闭文件描述符
	}
#if 1	
#ifdef __DEBUG__ // 输出终端信息的数据
	print_out_terminal_addr_infomation( tmnl_addr_list, tmnl_count );
	if( tmnl_count != -1)
		DEBUG_INFO( "terminal count num = %d", tmnl_count );
#endif
#endif
	init_terminal_allot_address();
	init_terminal_device_double_list();	
	init_terminal_discuss_param();
	terminal_speak_track_pro_init();
}

/*注册*/
bool terminal_register( uint16_t address, uint8_t dev_type, tmnl_pdblist p_tmnl_station )
{
	bool bret = false;
	int i = 0;
	
	if( NULL == p_tmnl_station )
	{
#ifdef __DEBUG__
		assert( p_tmnl_station );
#else
		bret = false;
#endif
	}
	
	if( !p_tmnl_station->tmnl_dev.tmnl_status.is_rgst )
	{
		for( i = 0; i < SYSTEM_TMNL_MAX_NUM; i++ )
		{
			if( (address & TMN_ADDR_MASK) == (tmnl_addr_list[i].addr))
			{
				DEBUG_INFO( "register addr = %04x-%04x, index = %d ", address & TMN_ADDR_MASK, tmnl_addr_list[i].addr, i );
				p_tmnl_station->tmnl_dev.tmnl_status.is_rgst = true;
				p_tmnl_station->tmnl_dev.tmnl_status.device_type = dev_type;
			        p_tmnl_station->tmnl_dev.address.addr = address & TMN_ADDR_MASK;
				p_tmnl_station->tmnl_dev.address.tmn_type = tmnl_addr_list[i].tmn_type;
				bret = true;
				break;
			}
		}
	}

	set_terminal_system_state( DISCUSS_STATE, true );

	return bret;
}

void terminal_type_save( uint16_t address, uint8_t tmnl_type, bool is_chman )
{
	int i = 0;

	for( i = 0; i < SYSTEM_TMNL_MAX_NUM; i++ )
	{
		if( address == (tmnl_addr_list[i].addr))
		{
			DEBUG_INFO( "terminal(--%04x--) save type = %d ", address, tmnl_type );

			if( (is_chman && ((tmnl_type == TMNL_TYPE_CHM_COMMON) ||(tmnl_type == TMNL_TYPE_CHM_EXCUTE)))\
				|| (!is_chman && ((tmnl_type == TMNL_TYPE_COMMON_RPRST) ||(tmnl_type == TMNL_TYPE_VIP))))
			{
				tmnl_pdblist tmnl_node = found_terminal_dblist_node_by_addr( address );
				if( tmnl_node !=NULL )
				{
					tmnl_node->tmnl_dev.address.tmn_type = tmnl_type;
				}

				tmnl_addr_list[i].tmn_type = tmnl_type;
				break;
			}	
		}
	}
}

void terminal_trasmint_message( uint16_t address, uint8_t *p_data, uint16_t msg_len )
{
	assert( p_data );
	
	upper_cmpt_terminal_message_report( p_data, msg_len, address );
}

uint16_t find_new_apply_addr( terminal_address_list_pro* p_gallot, terminal_address_list* p_gaddr_list, uint16_t* new_index)
{
	assert( p_gallot && p_gaddr_list && new_index );
	uint16_t current_index = p_gallot->index;
	uint16_t temp_addr = 0;
	uint16_t i = 0;
	
	if( current_index >= SYSTEM_TMNL_MAX_NUM || NULL == new_index)
		return 0xffff;

	if( p_gaddr_list[current_index].addr == 0xffff )
	{
		temp_addr = p_gallot->addr_start + current_index;
		*new_index = current_index;
	}
	else
	{
		i = current_index + 1;
		temp_addr = p_gaddr_list[i].addr;
		do
		{
			i %= SYSTEM_TMNL_MAX_NUM;
			if( p_gaddr_list[i].addr == 0xffff)
			{
				break;
			}

			i++;
		}
		while( i != current_index);

		if( i != current_index)
		{
			temp_addr = p_gallot->addr_start + i;
			*new_index = i;
		}
	}

	DEBUG_INFO( "new addr = %04x",  temp_addr );
	return temp_addr;
}

/*==================================================
					开始终端命令函数
====================================================*/
/***************************************************
**Writer:YasirLiang
**Date: 2015/10/26
**Name:terminal_func_allot_address
**Garam:
**		cmd: func cmd
**		data: proccess recv data
**		data_len: recv data length
**Func: procces conference allot command data sended by terminal
******************************************************/
int terminal_func_allot_address( uint16_t cmd, void *data, uint32_t data_len )
{
	struct endstation_to_host msg; 
	struct endstation_to_host_special spe_msg;
	terminal_address_list* p_addr_list = tmnl_addr_list;
	terminal_address_list_pro* p_allot = &allot_addr_pro;
	uint16_t new_addr = 0;
	uint16_t new_index = 0;
	uint8_t data_buf[DATAMAXLENGTH] = {0};
	uint16_t send_data_lng = 0;
	
	conference_end_to_host_frame_read( data, &msg, &spe_msg, 0, sizeof(msg) );
	
	// 保存地址数据
	if( msg.cchdr.command_control & COMMAND_TMN_REPLY )
	{
		DEBUG_LINE();
		send_data_lng = 0;
		if( p_addr_list[p_allot->index].addr != 0xffff)
		{
			p_allot->renew_flag = 1;

			DEBUG_INFO( "man type = 0x%02x ", msg.cchdr.command_control & COMMAND_TMN_CHAIRMAN );
			if( msg.cchdr.command_control & COMMAND_TMN_CHAIRMAN )
			{
				p_addr_list[p_allot->index].tmn_type = TMNL_TYPE_CHM_EXCUTE;
				DEBUG_INFO( "tmn type = %d ", p_addr_list[p_allot->index].tmn_type );
			}
			else
			{
				p_addr_list[p_allot->index].tmn_type = TMNL_TYPE_COMMON_RPRST;
				DEBUG_INFO( "tmn type = %d ", p_addr_list[p_allot->index].tmn_type );
			}

			// save new addr to file
			addr_file_fd = Fopen( ADDRESS_FILE, "ab+"); // -->>此流程可优化，改为直接操作内存
			if( addr_file_fd == NULL )
			{
				DEBUG_ERR( "terminal_func_allot_address open fd  Err!" );
				assert( NULL != addr_file_fd );
			}

			terminal_address_list tmp_addr;
			tmp_addr.addr = p_addr_list[p_allot->index].addr;
			tmp_addr.tmn_type = p_addr_list[p_allot->index].tmn_type;
			terminal_address_list_write_file( addr_file_fd, &tmp_addr, 1 );
			Fclose( addr_file_fd );
		}
	}
	else
	{
		if( (msg.data == ADDRESS_ALREADY_ALLOT) && (p_allot->renew_flag) && (p_addr_list[p_allot->index].addr != 0xffff))
		{
			new_addr = p_addr_list[p_allot->index].addr;
		}
		else
		{
			new_addr = find_new_apply_addr( p_allot, p_addr_list, &new_index );
			p_allot->index = new_index;
			p_allot->renew_flag = 0;
			p_addr_list[p_allot->index].addr = new_addr;
		}

		data_buf[0] = (uint8_t)((new_addr & 0x00ff) >> 0);// 低字节在前
		data_buf[1] = (uint8_t )((new_addr & 0xff00) >> 8);
		send_data_lng = sizeof(uint16_t);
	}

	host_reply_terminal( ALLOCATION, msg.cchdr.address, data_buf, send_data_lng );

	return 0;
}

/***************************************************
**Writer:YasirLiang
**Date: 2015/10/29
**Name:terminal_func_key_action
**Garam:
**		cmd: func cmd
**		data: proccess recv data
**		data_len: recv data length
**Func: procces conference key_action command data sended by terminal
******************************************************/
int terminal_func_key_action( uint16_t cmd, void *data, uint32_t data_len )
{
	struct endstation_to_host msg;
	struct endstation_to_host_special spe_msg;
	conference_end_to_host_frame_read( data, &msg, &spe_msg, 0, sizeof(msg) );
	tmnl_pdblist cam_node = NULL;
	uint16_t addr = msg.cchdr.address & TMN_ADDR_MASK;
	uint8_t key_num = KEY_ACTION_KEY_NUM( msg.data );
	uint8_t key_value = KEY_ACTION_KEY_VALUE( msg.data );
	uint8_t tmnl_state = KEY_ACTION_STATE_VALUE( msg.data );
	uint8_t sys_state = get_sys_state();
	DEBUG_INFO( "key_num = %d, key_value = %d, tmnl_state = %d, sys_state = %d", key_num, key_value, tmnl_state, sys_state );

	if( (msg.cchdr.command_control & COMMAND_TMN_REPLY) == COMMAND_TMN_REPLY )
	{
		DEBUG_INFO( "key action command not valid!" );
		return -1;
	}
	
	switch( sys_state )
	{
		case SIGN_STATE:
		case VOTE_STATE:
		case GRADE_STATE:
		case ELECT_STATE:
			terminal_vote( addr, key_num, key_value, tmnl_state, msg.data );
			terminal_key_speak( addr, key_num, key_value, tmnl_state, msg.data );
			terminal_key_action_chman_interpose( addr, key_num, key_value, tmnl_state, msg.data );
			break;
		case DISCUSS_STATE:
			terminal_key_discuccess( addr, key_num, key_value, tmnl_state, msg.data );
			terminal_chairman_apply_reply( msg.cchdr.command_control & COMMAND_TMN_CHAIRMAN,addr, key_num, key_value, tmnl_state, msg.data);
			terminal_key_action_chman_interpose( addr, key_num, key_value, tmnl_state, msg.data );
			break;
		case INTERPOSE_STATE:
			terminal_key_action_chman_interpose( addr, key_num, key_value, tmnl_state, msg.data );
			break;
		case CAMERA_PRESET:
			cam_node = found_terminal_dblist_node_by_addr( addr );
			if( cam_node != NULL )
			{
				terminal_key_action_host_common_reply( msg.data, cam_node );
				terminal_key_preset( 0, addr, tmnl_state, key_num, key_value );
			}
			break;
		default:
			break;
	}
	
	return 0;
}

/***************************************************
**Writer:YasirLiang
**Date: 2015/10/29
**Name:terminal_func_chairman_control
**Garam:
**		cmd: func cmd
**		data: proccess recv data
**		data_len: recv data length
**Func: procces conference chairman control command data sended by terminal
******************************************************/
int terminal_func_chairman_control( uint16_t cmd, void *data, uint32_t data_len )
{
	struct endstation_to_host msg;
	struct endstation_to_host_special spe_msg;
	conference_end_to_host_frame_read( data, &msg, &spe_msg, 0, sizeof(msg) );
	uint16_t addr = msg.cchdr.address & TMN_ADDR_MASK;
	uint8_t sign_value = 0;
	uint8_t chair_opt = msg.data&CHAIRMAN_CONTROL_MEET_MASK;
	uint8_t sign_flag = 0;
	tmnl_pdblist query_tmp = NULL;
	thost_system_set set_sys; // 系统配置文件的格式
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));
	
	tmnl_pdblist tmp = found_terminal_dblist_node_by_addr( addr );
	if( tmp == NULL )
	{
		DEBUG_INFO( "not found chairman conntrol address!" );
		return -1;
	}

	if(  chair_opt != CHM_BEGIN_VOTE )
	{
		terminal_chairman_control_meeting( tmp->tmnl_dev.entity_id, addr, sign_value );
	}

	switch( chair_opt )
	{
		case CHM_BEGIN_SIGN:
			gset_sys.sign_type = KEY_SIGN_IN;
			terminal_chman_control_start_sign_in( KEY_SIGN_IN, 10 );
			break;
		case CHM_END_SIGN:
			terminal_end_sign( 0, NULL, 0 );
			break;
		case CHM_BEGIN_VOTE:
			if( gtmnl_signstate ) // 系统已经签到
			{
				terminal_chairman_control_meeting( tmp->tmnl_dev.entity_id, addr, sign_value );
				terminal_chman_control_begin_vote( VOTE_MODE, false, &sign_flag );// 末次按键有效
			}
			else
			{
				sign_value = 1; // 未签到
				terminal_chairman_control_meeting( tmp->tmnl_dev.entity_id, addr, sign_value );
			}
			break;
		case CHM_END_VOTE:
			terminal_end_vote( 0, NULL, 0 );

			// 设置终端的状态
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0; 
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keyup = 0;
			terminal_state_set_base_type( BRDCST_ALL, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST] );
			terminal_state_all_copy_from_common();

			// 设置lcd
			terminal_lcd_display_num_send( BRDCST_ALL, LCD_OPTION_CLEAR, VOTE_INTERFACE );
			// 设置led灯
			terminal_led_set_save( BRDCST_ALL, TLED_KEY2, TLED_OFF );
			terminal_led_set_save( BRDCST_ALL, TLED_KEY3, TLED_OFF );
			terminal_led_set_save( BRDCST_ALL, TLED_KEY4, TLED_OFF );
			fterminal_led_set_send( BRDCST_ALL );

			// 发送表决结果
			if( msg.data&0x10 )	// 广播表决结果
			{
				terminal_broadcast_end_vote_result( BRDCST_ALL );
			}
			else
			{
				terminal_broadcast_end_vote_result( addr );
			}
			break;
		case CHM_SUSPEND_VOTE:
			terminal_pause_vote( 0, NULL, 0 );
			break;
		case CHM_RECOVER_VOTE:
			terminal_regain_vote( 0, NULL, 0 );
			break;
		case CHM_RETURN_DISCUSS:
			set_terminal_system_state( DISCUSS_STATE, true );
			terminal_start_discuss( false );
			break;
		case CHM_CLOSE_ALL_MIC:// 关闭所有普通代表机
			assert( dev_terminal_list_guard );
			for( query_tmp = dev_terminal_list_guard->next; query_tmp != dev_terminal_list_guard; query_tmp = query_tmp->next )
			{
				if( (query_tmp->tmnl_dev.address.addr != 0xffff) && \
					query_tmp->tmnl_dev.tmnl_status.is_rgst && \
					query_tmp->tmnl_dev.address.tmn_type == TMNL_TYPE_COMMON_RPRST &&\
					(query_tmp->tmnl_dev.tmnl_status.mic_state != MIC_COLSE_STATUS) )
				{
					query_tmp->tmnl_dev.tmnl_status.mic_state = MIC_COLSE_STATUS;
					connect_table_tarker_disconnect( query_tmp->tmnl_dev.entity_id, query_tmp, true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
				}
			}
			
			terminal_mic_state_set( MIC_COLSE_STATUS, BRDCST_MEM,BRDCST_1722_ALL, false, NULL );
			cmpt_miscrophone_status_list();
			gdisc_flags.speak_limit_num = 0;
			gdisc_flags.apply_num = 0;
			gdisc_flags.currect_first_index = gdisc_flags.apply_limit;
			terminal_main_state_send( 0, NULL, 0 );
			break;
		default:
			break;
	}

	return 0;
}

/***************************************************
**Writer:YasirLiang
**Date: 2015/10/29
**Name:terminal_func_send_main_state
**Garam:
**		cmd: func cmd
**		data: proccess recv data
**		data_len: recv data length
**Func: procces conference send main state command data sended by terminal
******************************************************/
int terminal_func_send_main_state( uint16_t cmd, void *data, uint32_t data_len )
{
	terminal_main_state_send( 0, NULL, 0 );
	
	return 0;
}

/***************************************************
**Writer:YasirLiang
**Date: 2015/10/29
**Name:terminal_func_cmd_event
**Garam:
**		cmd: func cmd
**		data: proccess recv data
**		data_len: recv data length
**Func: procces conference special event command data sended by terminal
******************************************************/
int terminal_func_cmd_event( uint16_t cmd, void *data, uint32_t data_len )
{
	struct endstation_to_host msg;
	struct endstation_to_host_special spe_msg;
	conference_end_to_host_frame_read( data, &msg, &spe_msg, 0, sizeof(msg) );
	uint16_t addr = msg.cchdr.address & TMN_ADDR_MASK;
	thost_system_set set_sys; // 系统配置文件的格式
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));
	
	/*reply termianl*/
	if( msg.cchdr.command_control & COMMAND_TMN_REPLY )
	{
		return -1;
	}

	tmnl_pdblist tmp = found_terminal_dblist_node_by_addr( addr );
	if( tmp == NULL )
	{
		return -1;
	}

	terminal_endstation_special_event_reply( tmp->tmnl_dev.entity_id, addr);
	
	uint8_t special_event = msg.data;
	if( special_event == SIGN_IN_SPECIAL_EVENT ) // 终端签到
	{
		terminal_sign_in_special_event( tmp );
		thost_sys_state sys_state = get_terminal_system_state();
		if( DISCUSS_STATE == sys_state.host_state )
		{
			uint8_t dis_mode = set_sys.discuss_mode;
			
			if( APPLY_MODE == dis_mode  && (tmp->tmnl_dev.address.tmn_type == TMNL_TYPE_CHM_EXCUTE))
			{
				terminal_chairman_apply_type_set( addr );
			}
			else
			{
				terminal_state_set_base_type( addr, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);
			}
		}
	}
	
	return 0;
}

/***************************************************
**Writer:YasirLiang
**Date: 2015/10/29
**Name:terminal_mic_auto_close
**Garam:
**		cmd: func cmd
**		data: proccess recv data
**		data_len: recv data length
**Func: procces mic_auto_close system set command
******************************************************/
int terminal_mic_auto_close( uint16_t cmd, void *data, uint32_t data_len )
{
	uint8_t auto_close = 0;
	tmnl_pdblist tmnl_node = dev_terminal_list_guard->next;
	int i = 0;
	thost_system_set set_sys;
	
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));
	
	/* 设置终端的麦克风状态*/
	auto_close = set_sys.auto_close;
	for( i = 0; i < TMNL_TYPE_NUM; i++)
	{
		gtmnl_state_opt[i].auto_close = auto_close?1:0;
		gtmnl_state_opt[i].MicClose = MIC_CLOSE;
	}
	
	/*关闭所有麦克风，这里需要一个机制，即通道分配机制与麦克风设置机制(这时未实现10/29), 使用连接表管理系统的麦克风的连接状态，暂时未考虑同步的问题(11/4)*/
	for( ; tmnl_node != dev_terminal_list_guard; tmnl_node = tmnl_node->next )
	{
		if( tmnl_node->tmnl_dev.tmnl_status.is_rgst && tmnl_node->tmnl_dev.tmnl_status.mic_state != MIC_COLSE_STATUS )
		{
			connect_table_tarker_disconnect( tmnl_node->tmnl_dev.entity_id, tmnl_node, true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
		}
	}

	/*发送主机状态*/
	terminal_main_state_send( 0, NULL, 0 );

	return 0;
}

/*主机发送状态, */
int terminal_main_state_send( uint16_t cmd, void *data, uint32_t data_len )
{
	assert( dev_terminal_list_guard );
	tmnl_pdblist p_tmnl_list = dev_terminal_list_guard->next;	
	tmnl_main_state_send host_main_state;
	uint8_t spk_num = 0;
	thost_system_set set_sys; // 系统配置文件的格式
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));

	host_main_state.unit = gregister_tmnl_pro.tmn_total;
	host_main_state.camera_follow = set_sys.camara_track ? 1 : 0;
	host_main_state.chm_first = set_sys.temp_close ? 1 : 0;
	host_main_state.conference_stype = (set_sys.discuss_mode&0x0f); // low 4bit
	host_main_state.limit = set_sys.speak_limit; 		// 讲话人数上限
	host_main_state.apply_set = set_sys.apply_limit;	// 申请人数上限
	
	for( ;p_tmnl_list != dev_terminal_list_guard; p_tmnl_list = p_tmnl_list->next )
	{
		if( p_tmnl_list->tmnl_dev.address.addr != 0xffff && (p_tmnl_list->tmnl_dev.tmnl_status.mic_state == MIC_OPEN_STATUS))
			spk_num++;
	}
	
	host_main_state.spk_num = spk_num; // 当前讲话人数
	host_main_state.apply = gdisc_flags.apply_num;

	terminal_host_send_state( BRDCST_1722_ALL, host_main_state ); // target id is 0

	return 0;
}

/*终端发送显示屏号，可以继续完善11/2*/
int terminal_lcd_display_num_send( uint16_t addr, uint8_t display_opt, uint8_t display_num )
{
	tmnl_send_end_lcd_display lcd_dis;
	lcd_dis.opt = display_opt;
	lcd_dis.num = display_num;

	if( display_opt == LCD_OPTION_DISPLAY )
	{
		glcd_num = display_num;
	}
	
	terminal_send_end_lcd_display( 0, addr, lcd_dis );
	
	return 0;
}

/*暂定投票*/
int terminal_pause_vote( uint16_t cmd, void *data, uint32_t data_len )
{
	terminal_option_endpoint( BRDCST_1722_ALL, CONFERENCE_BROADCAST_ADDRESS, OPT_TMNL_SUSPEND_VOTE );

	return 0;
}

/*重新投票*/
int terminal_regain_vote( uint16_t cmd, void *data, uint32_t data_len )
{
	terminal_option_endpoint( BRDCST_1722_ALL, CONFERENCE_BROADCAST_ADDRESS, OPT_TMNL_RECOVER_VOTE );

	return 0;
}

int terminal_system_discuss_mode_set( uint16_t cmd, void *data, uint32_t data_len )
{
	assert( data && dev_terminal_list_guard );
	uint8_t dis_mode = *((uint8_t*)data);
	tmnl_pdblist tmnl_node = dev_terminal_list_guard->next;

	terminal_speak_track_pro_init();
	gdisc_flags.edis_mode = (ttmnl_discuss_mode)dis_mode;
	gdisc_flags.currect_first_index = MAX_LIMIT_APPLY_NUM;
	gdisc_flags.apply_num = 0;
	gdisc_flags.speak_limit_num = 0;

	/*关闭所有麦克风*/
	for( ; tmnl_node != dev_terminal_list_guard; tmnl_node = tmnl_node->next )
	{
		// 2.设置麦克风tarker的状态,上报麦克风状态, 设置相应终端的麦克风状态(会议主机与终端协议)
		if( tmnl_node->tmnl_dev.tmnl_status.is_rgst && (tmnl_node->tmnl_dev.tmnl_status.mic_state != MIC_COLSE_STATUS) )
		{
			connect_table_tarker_disconnect( tmnl_node->tmnl_dev.entity_id, tmnl_node, true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
		}
	}

	/*发送主机状态*/
	terminal_main_state_send( 0, NULL, 0 );

	return 0;
}

int terminal_speak_limit_num_set( uint16_t cmd, void *data, uint32_t data_len )// 处理函数有待完善(11/4)
{
	thost_system_set set_sys;
	
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));

	uint8_t spk_limt_num = *((uint8_t*)data);
	uint8_t dis_mode = set_sys.discuss_mode;

	gdisc_flags.limit_num = spk_limt_num;

	return 0;
}

int terminal_apply_limit_num_set( uint16_t cmd, void *data, uint32_t data_len )
{
	thost_system_set set_sys;
	
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));

	uint8_t apply_limt_num = *((uint8_t*)data);
	gdisc_flags.apply_limit = apply_limt_num;

	return 0;
}

/* 终端发言限时的控制,由于终端不能控制麦克风的关闭与打开，
所以系统不仅仅是会议协议数据的发送，还需管理终端的发言时长;而发言时长由系统的超时机制进行管理,
因此只需在终端连接时设置连接表相应的超时时间即可*/
int terminal_limit_speak_time_set( uint16_t cmd, void *data, uint32_t data_len )
{
	tmnl_limit_spk_time spk_time;
	thost_system_set set_sys; // 系统配置文件的格式
	
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));
	spk_time.limit_time = set_sys.spk_limtime;
	speak_limit_time = (uint8_t)spk_time.limit_time;
	
	if( !set_sys.spk_limtime ) // 无限时
	{
		terminal_limit_spk_time( 0, BRDCST_ALL, spk_time );
	}
	else
	{
		uint16_t nolimit_addr = 0;
		uint16_t limit_addr = BRDCST_MEM; // 限时对所有普通代表有效
		tmnl_limit_spk_time nolimit_spk_time;
		
		if( set_sys.vip_limitime ) // vip 限时
		{
			limit_addr |= BRDCST_VIP;
		}
		else
		{
			nolimit_addr |= BRDCST_VIP;
		}
		
		if( set_sys.chman_limitime ) // 主席限时
		{
			
			limit_addr |= BRDCST_CHM |BRDCST_EXE;
		}
		else
		{
			nolimit_addr |= BRDCST_CHM |BRDCST_EXE;
		}

		if( nolimit_addr )
		{
			nolimit_spk_time.limit_time = 0;
			terminal_limit_spk_time( BRDCST_1722_ALL, nolimit_addr, nolimit_spk_time );
		}

		terminal_limit_spk_time( BRDCST_1722_ALL, limit_addr, spk_time );
	}

	return 0;
}

int terminal_end_sign( uint16_t cmd, void *data, uint32_t data_len )
{
	set_terminal_system_state( DISCUSS_STATE, true );
	gtmnl_signstate = SIGN_IN_BE_LATE;

	// 设置讨论的状态
	terminal_start_discuss( false );
	// 开始补签

	return 0;
}

int terminal_end_vote( uint16_t cmd, void *data, uint32_t data_len )
{
	gvote_flag = NO_VOTE;// 结束投票

	return 0;
}

int termianal_music_enable( uint16_t cmd, void *data, uint32_t data_len )
{
	bool music_en = false;
	if( data_len != sizeof(uint8_t));
		return -1;

	music_en = *((uint8_t*)data)?true:false;
	gset_sys.chman_music = music_en;
}

int termianal_chairman_prior_set( uint16_t cmd, void *data, uint32_t data_len )
{
	bool prior_en = false;
	if( data_len != sizeof(uint8_t));
		return -1;

	prior_en = *((uint8_t*)data)?true:false;
	gset_sys.chman_first = prior_en;
}

/*==================================================
					结束终端命令函数
====================================================*/


/*===================================================
{@终端处理流程
=====================================================*/

int terminal_socroll_synch(void )
{
	terminal_option_endpoint( BRDCST_1722_ALL, CONFERENCE_BROADCAST_ADDRESS, OPT_TMNL_LED_DISPLAY_ROLL_SYNC );

	return 0;
}

void terminal_remove_unregitster( void ) // 这里没有清除终端地址文件以及内存终端列表里相应的内容
{
	tmnl_pdblist p_node = dev_terminal_list_guard->next;

	for( ; p_node != dev_terminal_list_guard; p_node = p_node->next )
	{
		if(  p_node->tmnl_dev.address.addr == 0xffff || !p_node->tmnl_dev.tmnl_status.is_rgst )
		{
			delect_terminal_dblist_node( &p_node );
		}
	}
}

// 需上报，且不是主席插话，才保存麦克风状态
void terminal_mic_state_set( uint8_t mic_status, uint16_t addr, uint64_t tarker_id, bool is_report_cmpt, tmnl_pdblist tmnl_node )
{
	assert( tmnl_node );
	DEBUG_INFO( "mic state = %d ",  mic_status );

	if( (tmnl_node == NULL) && !(addr & BROADCAST_FLAG) )
	{
		DEBUG_INFO( "nothing to send to set mic status!");
		return;
	}

	terminal_set_mic_status( mic_status, addr, tarker_id );
	if( is_report_cmpt && (mic_status != MIC_CHM_INTERPOSE_STATUS) && tmnl_node != NULL)
	{
		tmnl_node->tmnl_dev.tmnl_status.mic_state = mic_status;
		upper_cmpt_report_mic_state( mic_status, tmnl_node->tmnl_dev.address.addr );
	}
}

/*********************************************************
*writer:YasirLiang
*Date:2015/11/4
*Name:terminal_mic_speak_limit_time_manager_event
*Param:
*	none
*Retern Value:
*	-1,Err;
*	0,nomal.
*state:管理系统终端的发言时长，到时断开连接，
*	单位是分钟;超时检查从讨论开始; 终端限时发言的系统超时管理机制的接口函数
***********************************************************/ 
int terminal_mic_speak_limit_time_manager_event( void )
{
	assert( dev_terminal_list_guard );
	uint8_t system_state = get_sys_state();
	
	if(  system_state != DISCUSS_STATE/* || p_node == dev_terminal_list_guard */ )
	{
		return -1;
	}
	
	/*查看系统连接表, 并检查超时*/
	connect_table_timeouts_image();

	return 0;
}

/* 设置终端开始讨论的状态*/
int terminal_start_discuss( bool mic_flag )
{
	thost_system_set set_sys; // 系统配置文件的格式
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));
	
	assert( dev_terminal_list_guard );
	tmnl_pdblist  tmnl_node = dev_terminal_list_guard->next;
	if( !mic_flag ) // 关闭所有麦克风
	{	
		/*关闭所有麦克风*/
		for( ; tmnl_node != dev_terminal_list_guard; tmnl_node = tmnl_node->next )
		{
			//DEBUG_INFO( "=======>>>mic node id = %016llx, mic state = %d<<<=========", tmnl_node->tmnl_dev.entity_id, tmnl_node->tmnl_dev.tmnl_status.mic_state );
			if( tmnl_node->tmnl_dev.tmnl_status.is_rgst && (tmnl_node->tmnl_dev.tmnl_status.mic_state != MIC_COLSE_STATUS) ) // 断开注册连接的终端
			{
				connect_table_tarker_disconnect( tmnl_node->tmnl_dev.entity_id, tmnl_node, true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
			}
		}
	}
	
	int i = 0;
	for( i = 0; i < TMNL_TYPE_NUM; i++ )
	{
		gtmnl_state_opt[i].auto_close = set_sys.auto_close?1:0;
		gtmnl_state_opt[i].keydown = 0;
		gtmnl_state_opt[i].keyup = 0;
		gtmnl_state_opt[i].MicClose = mic_flag?1:0;
		gtmnl_state_opt[i].sys = TMNL_SYS_STA_DISC; // 讨论模式
	} 

	/* 设置终端状态*/
	if( APPLY_MODE == set_sys.discuss_mode )
	{
		terminal_state_set_base_type( BRDCST_MEM |BRDCST_VIP|BRDCST_CHM, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);	// 根据终端类型设置终端的状态
		terminal_lcd_display_num_send( BRDCST_MEM |BRDCST_VIP|BRDCST_CHM, LCD_OPTION_CLEAR, glcd_num );// 发送lcd显示屏号
		terminal_chairman_apply_type_set( BRDCST_EXE );
		
	}
	else
	{
		terminal_state_set_base_type( BRDCST_MEM |BRDCST_VIP|BRDCST_CHM|BRDCST_EXE,gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);	// 根据终端类型设置终端的状态
		terminal_lcd_display_num_send( BRDCST_MEM |BRDCST_VIP|BRDCST_CHM|BRDCST_EXE, LCD_OPTION_CLEAR, glcd_num );// 发送lcd显示屏号
	}
	
	/*设置终端指示灯*/
	terminal_led_set_save( BRDCST_ALL, TLED_KEY1, TLED_OFF );
	terminal_led_set_save( BRDCST_ALL, TLED_KEY2, TLED_OFF );
	terminal_led_set_save( BRDCST_ALL, TLED_KEY3, TLED_OFF );
	terminal_led_set_save( BRDCST_ALL, TLED_KEY4, TLED_OFF );
	terminal_led_set_save( BRDCST_ALL, TLED_KEY5, TLED_OFF );
	fterminal_led_set_send( BRDCST_ALL );
	
	terminal_main_state_send( 0, NULL, 0 );
	
	return 0;
}

/*依据类型设置终端的状态*/
void terminal_state_set_base_type( uint16_t addr, tmnl_state_set tmnl_state )
{
	assert( dev_terminal_list_guard );
	tmnl_pdblist tmp = found_terminal_dblist_node_by_addr( addr );
	if( tmp == NULL )
	{
		terminal_state_set( tmnl_state, addr, BRDCST_1722_ALL );
	}
	else
	{
		terminal_state_set( tmnl_state, addr, tmp->tmnl_dev.entity_id );
	}
	
}

/*主席申请类型终端状态设置*/
void terminal_chairman_apply_type_set( uint16_t addr )
{
	gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE].keydown = 0x0e; // 2 3 4键按下有效
	gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE].keyup = 0;
	gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE].sys = TMNL_SYS_STA_DISC;

	terminal_state_set_base_type( addr, gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE] );
	terminal_lcd_display_num_send( addr, LCD_OPTION_DISPLAY, CHM_APPROVE_APPLY_INTERFACE );
}

void terminal_chairman_apply_type_clear( uint16_t addr )
{
	gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE].keydown = 0; 
	gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE].keyup = 0;
	gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE].sys = TMNL_SYS_STA_DISC;

	terminal_state_set_base_type( addr, gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE] );
	terminal_lcd_display_num_send( addr, LCD_OPTION_DISPLAY, CHM_APPROVE_APPLY_INTERFACE );
}

/*保存led指示灯的状态*/
bool terminal_led_set_save( uint16_t addr, uint8_t led_id, uint8_t  led_state )
{
	uint8_t byte_num,shifting;

	if( 0xFFFF == addr )
	{
		return false;
	}
	
	if(led_id>=TLED_MAX_NUM)
	{
		return false;
	}

	byte_num = led_id/4;
	shifting = (led_id%4)*2;
	gled_buf[byte_num]&=(~(0x03<<shifting));
	gled_buf[byte_num]|=(led_state<<shifting);

	return true;
}

/*发送led指示灯的状态*/ 
void fterminal_led_set_send( uint16_t addr )
{
	ttmnl_led_lamp led_lamp;
	led_lamp.data_low = gled_buf[0];
	led_lamp.data_high = gled_buf[1];

	assert( dev_terminal_list_guard );
	tmnl_pdblist tmp = found_terminal_dblist_node_by_addr( addr );
	if( tmp == NULL )
	{
		terminal_set_indicator_lamp( led_lamp, addr, BRDCST_1722_ALL );
	}
	else
	{
		terminal_set_indicator_lamp( led_lamp, addr, tmp->tmnl_dev.entity_id  );
	}
}

/*处理上位机对麦克风的操作命令(打开与关闭)*/
int terminal_upper_computer_speak_proccess( tcmpt_data_mic_switch mic_flag )
{
	uint16_t addr = (((uint16_t)(mic_flag.addr.low_addr << 0)) | ((uint16_t)(mic_flag.addr.high_addr << 0)));
	uint8_t mic_state_set = mic_flag.switch_flag;
	ttmnl_discuss_mode dis_mode = gdisc_flags.edis_mode;
	tmnl_pdblist speak_node = NULL;
	uint8_t limit_time = 0;
	bool found_node = false;
	bool read_success = false;
	thost_system_set set_sys; // 系统配置文件的格式
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));
	
	read_success = true;
	limit_time = set_sys.spk_limtime;
	
	assert( dev_terminal_list_guard );
	DEBUG_INFO( " speak addr = %04x, discuccess mode = %d", addr, dis_mode );
	for( speak_node = dev_terminal_list_guard->next; speak_node != dev_terminal_list_guard; speak_node = speak_node->next )
	{
		if( speak_node->tmnl_dev.address.addr == addr && speak_node->tmnl_dev.tmnl_status.is_rgst )
		{
			found_node = true;
			break;
		}
	}

	if( read_success && found_node )
	{
		if( mic_state_set &&  dis_mode != APPLY_MODE )
		{
			//found_connect_table_available_connect_node( speak_node->tmnl_dev.entity_id );
		}
		
		if( dis_mode == PPT_MODE ||\
			(speak_node->tmnl_dev.address.tmn_type == TMNL_TYPE_VIP) ||\
			(speak_node->tmnl_dev.address.tmn_type == TMNL_TYPE_CHM_COMMON)||\
			(speak_node->tmnl_dev.address.tmn_type == TMNL_TYPE_CHM_EXCUTE))
		{
			if( mic_state_set )
			{
				connect_table_tarker_connect( speak_node->tmnl_dev.entity_id, limit_time, speak_node, true, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send );
			}
			else
			{
				connect_table_tarker_disconnect( speak_node->tmnl_dev.entity_id, speak_node, true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
			}
		}
		else
		{
			switch( dis_mode )
			{
				case FREE_MODE:
				{
					terminal_free_disccuss_mode_cmpt_pro( mic_state_set, limit_time, speak_node );
					break;
				}
				case LIMIT_MODE:
				{
					terminal_limit_disccuss_mode_cmpt_pro( mic_state_set, limit_time, speak_node );
					break;
				}
				case FIFO_MODE:
				{
					terminal_fifo_disccuss_mode_cmpt_pro( mic_state_set, limit_time, speak_node );
					break;
				}
				case APPLY_MODE:
				{
					terminal_apply_disccuss_mode_cmpt_pro( mic_state_set, limit_time, speak_node );
					break;
				}
				default:
				{
					DEBUG_INFO( " out of discuss mode bound!" );
					break;
				}
			}
		}
	}
	else if( !found_node)
	{
		return -1;
	}
	
	return 0;
}

//  file STSTEM_SET_STUTUS_PROFILE must be close,before use this function
bool terminal_read_profile_file( thost_system_set *set_sys )
{
	FILE* fd = NULL;
	
	fd = Fopen( STSTEM_SET_STUTUS_PROFILE, "rb" ); // 只读读出数据
	if( NULL == fd )
	{
		DEBUG_INFO( "mian state send ->open files %s Err!",  STSTEM_SET_STUTUS_PROFILE );
		return false;
	}

	assert( set_sys );
	if( profile_system_file_read( fd, set_sys ) == -1)
	{
		DEBUG_INFO( "Read profile system Err!" );
		Fclose( fd );
		return false;
	}

	Fclose( fd );
	return true;
}

void terminal_free_disccuss_mode_cmpt_pro( uint8_t mic_flag, uint8_t limit_time, tmnl_pdblist speak_node )
{
	if( speak_node == NULL )
	{
		return;
	}
	
	if( mic_flag ) // 打开麦克风
	{
		if( gdisc_flags.speak_limit_num < FREE_MODE_SPEAK_MAX )
		{
			connect_table_tarker_connect( speak_node->tmnl_dev.entity_id, limit_time, speak_node, true, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send );
		}
	}
	else
	{
		connect_table_tarker_disconnect( speak_node->tmnl_dev.entity_id, speak_node, true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
	}

	terminal_main_state_send( 0, NULL, 0 );
}

bool terminal_limit_disccuss_mode_cmpt_pro( uint8_t mic_flag, uint8_t limit_time, tmnl_pdblist speak_node )
{
	bool ret = false;
	uint16_t addr = speak_node->tmnl_dev.address.addr;
	uint16_t current_addr = 0;
	uint8_t cc_state = 0;
	tmnl_pdblist first_apply = NULL; // 首位申请发言
	
	if( speak_node == NULL )
	{
		return ret;
	}

	if( mic_flag ) // 打开麦克风
	{
		if( speak_node->tmnl_dev.tmnl_status.mic_state != MIC_COLSE_STATUS )
		{
			ret = true;
		}
		else if( gdisc_flags.speak_limit_num < gdisc_flags.limit_num ) // 打开麦克风
		{
			connect_table_tarker_connect( speak_node->tmnl_dev.entity_id, limit_time, speak_node, true, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send );
			ret = true;
		}
		else if( gdisc_flags.apply_num < gdisc_flags.apply_limit ) // 申请发言
		{
			uint8_t state = MIC_OTHER_APPLY_STATUS;
			if( 0 == gdisc_flags.apply_num )
			{
				gdisc_flags.currect_first_index = 0;
				state = MIC_FIRST_APPLY_STATUS;
			}
			gdisc_flags.apply_addr_list[gdisc_flags.apply_num] = speak_node->tmnl_dev.address.addr;
			gdisc_flags.apply_num++;

			terminal_mic_state_set( state, speak_node->tmnl_dev.address.addr, speak_node->tmnl_dev.entity_id, true, speak_node );
			terminal_main_state_send( 0, NULL, 0 );
			ret = true;
		}
	}
	else
	{
		connect_table_tarker_disconnect( speak_node->tmnl_dev.entity_id, speak_node, true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
		current_addr = gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index];
		cc_state = speak_node->tmnl_dev.tmnl_status.mic_state;
		
		if( cc_state == MIC_FIRST_APPLY_STATUS || cc_state == MIC_OTHER_APPLY_STATUS )
		{
			addr_queue_delect_by_value( gdisc_flags.apply_addr_list, &gdisc_flags.apply_num, addr );

			if( gdisc_flags.apply_num > 0 && current_addr == addr )// 置下一个申请为首位申请状态
			{
				gdisc_flags.currect_first_index %= gdisc_flags.apply_num;
				first_apply = found_terminal_dblist_node_by_addr( gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index]);
				if( first_apply != NULL )
				{
					terminal_mic_state_set( MIC_FIRST_APPLY_STATUS, speak_node->tmnl_dev.address.addr, speak_node->tmnl_dev.entity_id, true, speak_node );
				}
			}

			terminal_main_state_send( 0, NULL, 0 );
			ret = true;
		}
		else if( cc_state == MIC_OPEN_STATUS )
		{
			if( gdisc_flags.speak_limit_num > 0 )
			{
				gdisc_flags.speak_limit_num--;
			}

			if(gdisc_flags.speak_limit_num < gdisc_flags.limit_num && gdisc_flags.apply_num > 0 )// 结束发言,并开始下一个申请终端的发言
			{
				if( addr_queue_delete_by_index( gdisc_flags.apply_addr_list, &gdisc_flags.apply_num, gdisc_flags.currect_first_index) )// 开启下一个申请话筒
				{
					tmnl_pdblist first_speak = found_terminal_dblist_node_by_addr( current_addr );
					if( first_speak != NULL )
					{
						connect_table_tarker_connect( first_speak->tmnl_dev.entity_id, limit_time, first_speak, true, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send );

						if( gdisc_flags.apply_num > 0 ) // 设置首位申请发言终端
						{
							gdisc_flags.currect_first_index %= gdisc_flags.apply_num;
							first_apply = found_terminal_dblist_node_by_addr( gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index]);
							if( first_apply != NULL )
							{
								terminal_mic_state_set( MIC_FIRST_APPLY_STATUS, speak_node->tmnl_dev.address.addr, speak_node->tmnl_dev.entity_id, true, speak_node );
							}
							else
							{
								DEBUG_INFO( " no such tmnl dblist node!");
							}
						}
						else
						{
							gdisc_flags.currect_first_index = gdisc_flags.apply_num;
						}
					}
					else
					{
						DEBUG_INFO( " no such tmnl dblist node!");
					}
				}
				else
				{
					gdisc_flags.currect_first_index = 0;
				}
			}

			terminal_main_state_send( 0, NULL, 0 );
			ret = true;
		}
	}

	return ret;
}

bool terminal_fifo_disccuss_mode_cmpt_pro( uint8_t mic_flag, uint8_t limit_time, tmnl_pdblist speak_node )
{
	bool ret = false;
	uint16_t addr = speak_node->tmnl_dev.address.addr;
	if( speak_node == NULL )
	{
		DEBUG_INFO( " NULL speak node!" );
		return false;
	}

	uint8_t speak_limit_num = gdisc_flags.speak_limit_num;
	if( mic_flag ) // 打开话筒
	{
		if( addr_queue_find_by_value( gdisc_flags.speak_addr_list, speak_limit_num, addr, NULL))
		{
			connect_table_tarker_connect( speak_node->tmnl_dev.entity_id, limit_time, speak_node, true, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send );
			ret = true;
		}
		else if( speak_limit_num < gdisc_flags.limit_num )
		{
			connect_table_tarker_connect( speak_node->tmnl_dev.entity_id, limit_time, speak_node, true, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send );
			gdisc_flags.speak_addr_list[speak_limit_num] = addr;
			ret = true;
		}
		else // 发言人数大于或等于限制人数
		{
			if( gdisc_flags.speak_addr_list[0] != 0xffff ) // 先进先出
			{
				tmnl_pdblist first_speak = found_terminal_dblist_node_by_addr( gdisc_flags.speak_addr_list[0] );
				if( first_speak != NULL )
				{
					connect_table_tarker_disconnect( first_speak->tmnl_dev.entity_id, first_speak, true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
				}
				else
				{
					DEBUG_INFO( "fifo not found tmnl list node!");
				}
				
				addr_queue_delete_by_index( gdisc_flags.speak_addr_list, &gdisc_flags.speak_limit_num, 0 );// 首位发言删除

				uint8_t speak_limit_num1 = gdisc_flags.speak_limit_num;
				connect_table_tarker_connect( speak_node->tmnl_dev.entity_id, limit_time, speak_node, true, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send );
				gdisc_flags.speak_addr_list[speak_limit_num1] = speak_node->tmnl_dev.address.addr;
				ret = true;
			}
		}
	}
	else
	{
		addr_queue_delect_by_value( gdisc_flags.speak_addr_list, &gdisc_flags.speak_limit_num, speak_node->tmnl_dev.address.addr );
		connect_table_tarker_disconnect( speak_node->tmnl_dev.entity_id, speak_node, true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
		ret = true;
	}

	return ret;
}

bool terminal_apply_disccuss_mode_cmpt_pro( uint8_t mic_flag, uint8_t limit_time, tmnl_pdblist speak_node )
{
	if( speak_node == NULL )
	{
		return false;
	}

	bool ret = false;
	uint16_t addr = speak_node->tmnl_dev.address.addr;
	uint16_t current_addr = 0;
	tmnl_pdblist first_apply = NULL;

	if( mic_flag ) // 申请发言,加地址入申请列表
	{
		if(  gdisc_flags.apply_num < gdisc_flags.apply_limit )
		{
			uint8_t state = MIC_OTHER_APPLY_STATUS;
			if( 0 == gdisc_flags.apply_num )
			{
				gdisc_flags.currect_first_index = 0;
				state = MIC_FIRST_APPLY_STATUS;
			}
			gdisc_flags.apply_addr_list[gdisc_flags.apply_num] = speak_node->tmnl_dev.address.addr;
			gdisc_flags.apply_num++;

			terminal_mic_state_set( state, speak_node->tmnl_dev.address.addr, speak_node->tmnl_dev.entity_id, true, speak_node );
			terminal_main_state_send( 0, NULL, 0 );
			ret = true;
		}
	}
	else // 取消申请发言
	{
		
		current_addr = gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index];
		if(addr_queue_delect_by_value( gdisc_flags.apply_addr_list, &gdisc_flags.apply_num, addr ))
		{
			if( gdisc_flags.apply_num > 0 && current_addr == addr )// 置下一个申请为首位申请状态
			{
				gdisc_flags.currect_first_index %= gdisc_flags.apply_num;
				first_apply = found_terminal_dblist_node_by_addr( gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index] );
				if( first_apply != NULL )
				{
					terminal_mic_state_set( MIC_FIRST_APPLY_STATUS, speak_node->tmnl_dev.address.addr, speak_node->tmnl_dev.entity_id, true, speak_node );
				}
			}
			
			terminal_main_state_send( 0, NULL, 0 );
		}	
		else
		{
			connect_table_tarker_disconnect( speak_node->tmnl_dev.entity_id, speak_node, true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
		}
		
		ret = true;
	}

	return ret;
}

bool addr_queue_delete_by_index( uint16_t *addr_queue, uint8_t *queue_len, uint8_t index )
{
	uint8_t i;
	
	if((NULL==addr_queue)||(NULL==queue_len))
	{
		return false;
	}
	if((index >= *queue_len)||(0==*queue_len))
	{
		return false;
	}

	for(i = index; i<(*queue_len-1); i++)
	{
		*(addr_queue+i)=*(addr_queue+i+1);
	}
	
	*(addr_queue+*queue_len-1)=0xFFFF;
	(*queue_len)--;

	return true;
}

bool addr_queue_delect_by_value( uint16_t *addr_queue, uint8_t *queue_len, uint16_t value)
{
	uint16_t   i;
	uint16_t  index;
	if((NULL==addr_queue)||(NULL==queue_len))
	{
		return false;
	}
	
	if( 0 == *queue_len )
	{
		return false;
	}
	
	for(i=0; i<*queue_len; i++)
	{
		if(*(addr_queue+i)==value)
		{
			break;
		}
	}
	
	index = i;
	if(index<*queue_len)
	{
		for(i=index; i<(*queue_len-1); i++)
		{
			*(addr_queue+i)=*(addr_queue+i+1);
		}
		
		*(addr_queue+*queue_len-1)=0xFFFF;
		(*queue_len)--;
		
		return true;
	}
	
	return false;
}

bool addr_queue_find_by_value( uint16_t *addr_queue, uint8_t queue_len, uint16_t value, uint8_t *index)
{
	uint8_t i;
	
	if((NULL==addr_queue)||(0==queue_len))
	{
		return false;
	}
	
	for(i=0; i<queue_len; i++)
	{
		if(value == *(addr_queue+i))
		{
			if(index != NULL)
			{
				*index = i;
			}
			
			return true;
		}
	}
	
	return false;
}

tmnl_pdblist found_terminal_dblist_node_by_addr( uint16_t addr )
{
	assert( dev_terminal_list_guard );
	tmnl_pdblist tmp = dev_terminal_list_guard->next;
	
	for( ; tmp != dev_terminal_list_guard; tmp = tmp->next )
	{
		if( tmp->tmnl_dev.address.addr == addr )
			return tmp;
	}

	return NULL;
}

void terminal_select_apply( uint16_t addr ) // 使选择的申请人是首位申请人
{
	tmnl_pdblist apply_first = NULL; 
	uint8_t apply_index = MAX_LIMIT_APPLY_NUM;
	int i = 0;
	
	if( apply_first == NULL )
	{
		return;
	}
	
	if( addr != gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index]) // 不是首位申请
	{
		apply_first = found_terminal_dblist_node_by_addr( gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index] );
		terminal_mic_state_set( MIC_OTHER_APPLY_STATUS, apply_first->tmnl_dev.address.addr, apply_first->tmnl_dev.entity_id, true, apply_first );

		for( i = 0; i < gdisc_flags.apply_num; i++ )
		{
			if( addr == gdisc_flags.apply_addr_list[i])
			{
				apply_index = i;
				break;
			}
		}

		if( apply_index < gdisc_flags.apply_num )
		{
			apply_first = found_terminal_dblist_node_by_addr( addr );
			terminal_mic_state_set( MIC_FIRST_APPLY_STATUS, apply_first->tmnl_dev.address.addr, apply_first->tmnl_dev.entity_id, true, apply_first );
		}
	}
}

bool terminal_examine_apply( enum_apply_pro apply_value )
{
	uint16_t addr = 0;
	tmnl_pdblist apply_first = NULL;
	bool ret = false;
	thost_system_set set_sys; // 系统配置文件的格式
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));
	
	if((gdisc_flags.edis_mode != APPLY_MODE) && (gdisc_flags.edis_mode != LIMIT_MODE))
	{
		return false;
	}

	switch( apply_value )
	{
		case REFUSE_APPLY:
			addr = gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index];
			if( addr_queue_delect_by_value(gdisc_flags.apply_addr_list, &gdisc_flags.apply_num, gdisc_flags.currect_first_index) )
			{
				apply_first = found_terminal_dblist_node_by_addr( addr );
				if( apply_first != NULL )
				{
					terminal_mic_state_set( MIC_COLSE_STATUS, apply_first->tmnl_dev.address.addr, apply_first->tmnl_dev.entity_id, true, apply_first );
					if( gdisc_flags.apply_num > 0 )
					{
						gdisc_flags.currect_first_index %= gdisc_flags.apply_num;
						apply_first = found_terminal_dblist_node_by_addr( gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index]);
						if( apply_first != NULL )
						{
							terminal_mic_state_set( MIC_FIRST_APPLY_STATUS, apply_first->tmnl_dev.address.addr, apply_first->tmnl_dev.entity_id, true, apply_first );
						}
					}
					else 
					{
						gdisc_flags.currect_first_index = MAX_LIMIT_APPLY_NUM;
					}
				}

				terminal_main_state_send( 0, NULL, 0 );
				ret = true;
			}
			break;
		case NEXT_APPLY:
			if( gdisc_flags.apply_num > 0 )
			{
				addr = gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index];
				apply_first = found_terminal_dblist_node_by_addr( addr );
				if( apply_first != NULL )
				{
					terminal_mic_state_set( MIC_OTHER_APPLY_STATUS, apply_first->tmnl_dev.address.addr, apply_first->tmnl_dev.entity_id, true, apply_first );
					gdisc_flags.currect_first_index++;
					gdisc_flags.currect_first_index %= gdisc_flags.apply_num;
					
					apply_first = found_terminal_dblist_node_by_addr( addr );
					if( apply_first != NULL )
					{
						terminal_mic_state_set( MIC_FIRST_APPLY_STATUS, apply_first->tmnl_dev.address.addr, apply_first->tmnl_dev.entity_id, true, apply_first );
					}
					else
					{
						DEBUG_INFO( "no found first apply node!" );
					}
				}
				else
				{
					DEBUG_INFO( "no found first apply node!" );
				}

				terminal_main_state_send( 0, NULL, 0 );
				ret = true;
			}
			break;
		case APPROVE_APPLY:
			if( gdisc_flags.currect_first_index < gdisc_flags.apply_num )
			{
				addr = gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index];
				apply_first = found_terminal_dblist_node_by_addr( addr ); // 打开第一个申请的麦克风
				if( apply_first != NULL )
				{
					connect_table_tarker_connect( apply_first->tmnl_dev.entity_id, set_sys.spk_limtime, apply_first, true, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send );
				}

				addr_queue_delete_by_index( gdisc_flags.apply_addr_list, &gdisc_flags.apply_num, gdisc_flags.currect_first_index );
				if( gdisc_flags.apply_num > 0 )
				{
					gdisc_flags.currect_first_index %= gdisc_flags.apply_num;
					apply_first = found_terminal_dblist_node_by_addr(gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index]);
					if( apply_first != NULL )
					{
						terminal_mic_state_set( MIC_FIRST_APPLY_STATUS, apply_first->tmnl_dev.address.addr, apply_first->tmnl_dev.entity_id, true, apply_first );
					}
				}
				else
				{
					gdisc_flags.currect_first_index = gdisc_flags.apply_limit;
				}

				terminal_main_state_send( 0, NULL, 0 );
				ret = true;
			}
			break;
		default:
			break;
	}

	return ret;
}

void terminal_type_set( tcmpt_data_meeting_authority tmnl_type )
{
	uint16_t addr = ((uint16_t)tmnl_type.addr.high_addr << 8)|((uint16_t)tmnl_type.addr.low_addr << 0 );
	tmnl_pdblist tmnl_node = NULL;
	
	DEBUG_INFO( "terminal type set addr = 0x%04x ", addr );
	tmnl_node = found_terminal_dblist_node_by_addr( addr );
	if( tmnl_node == NULL )
	{
		DEBUG_INFO( "no such type addr found!");
		return;
	}
	
	if( (tmnl_node->tmnl_dev.address.tmn_type == TMNL_TYPE_CHM_COMMON ||\
		tmnl_node->tmnl_dev.address.tmn_type == TMNL_TYPE_CHM_EXCUTE) &&\
		(tmnl_type.identity == TMNL_TYPE_CHM_COMMON ||\
		tmnl_type.identity == TMNL_TYPE_CHM_EXCUTE) )
	{
		tmnl_node->tmnl_dev.address.tmn_type = tmnl_type.identity;
		terminal_chairman_excute_set( tmnl_node->tmnl_dev.address.addr,(tmnl_type.identity == TMNL_TYPE_CHM_EXCUTE)?true:false);
		if( tmnl_type.identity == TMNL_TYPE_CHM_EXCUTE )
		{
			terminal_chairman_apply_type_set( addr );
		}
		else
		{
			terminal_chairman_apply_type_clear( addr );
		}
	}
	else if(tmnl_node->tmnl_dev.address.tmn_type == TMNL_TYPE_COMMON_RPRST ||\
		tmnl_node->tmnl_dev.address.tmn_type == TMNL_TYPE_VIP)
	{
		if( tmnl_type.identity == TMNL_TYPE_VIP )
		{
			tmnl_node->tmnl_dev.address.tmn_type = TMNL_TYPE_VIP;
		}
		else
		{
			tmnl_node->tmnl_dev.address.tmn_type = TMNL_TYPE_COMMON_RPRST;
		}

		terminal_vip_type_set( tmnl_node->tmnl_dev.address.addr, (tmnl_type.identity == TMNL_TYPE_VIP)?true:false );
	}

	// 保存到地址文件address.dat
	int i = 0;
	for( i = 0; i < SYSTEM_TMNL_MAX_NUM; i++ )
	{
		if( tmnl_addr_list[i].addr == addr )
		{
			tmnl_addr_list[i].tmn_type = tmnl_type.identity;
			break;
		}
	}
	
	terminal_type_save_to_address_profile( addr, tmnl_node->tmnl_dev.address.tmn_type );
}

void terminal_chairman_excute_set( uint16_t addr, bool is_set_excute )
{
	assert( dev_terminal_list_guard );
	tmnl_pdblist tmp = found_terminal_dblist_node_by_addr( addr );
	if( tmp == NULL )
	{
		terminal_option_endpoint( BRDCST_1722_ALL, addr, \
			is_set_excute ? OPT_TMNL_SET_EXCUTE_CHM : OPT_TMNL_CANCEL_EXCUTE_CHM );
	}
	else
	{
		terminal_option_endpoint( tmp->tmnl_dev.entity_id, addr, \
			is_set_excute ? OPT_TMNL_SET_EXCUTE_CHM : OPT_TMNL_CANCEL_EXCUTE_CHM );
	}
}

void terminal_vip_type_set( uint16_t addr, bool is_set_vip )
{
	assert( dev_terminal_list_guard );
	tmnl_pdblist tmp = found_terminal_dblist_node_by_addr( addr );
	if( tmp == NULL )
	{
		terminal_option_endpoint( BRDCST_1722_ALL, addr,\
			is_set_vip ? OPT_TMNL_SET_VIP : OPT_TMNL_CANCEL_VIP );
	}
	else
	{
		terminal_option_endpoint( tmp->tmnl_dev.entity_id, addr,\
			is_set_vip ? OPT_TMNL_SET_VIP : OPT_TMNL_CANCEL_VIP );
	}
}

int terminal_type_save_to_address_profile( uint16_t addr, uint16_t tmnl_type )
{
	FILE *fd = NULL;
	terminal_address_list addr_list[SYSTEM_TMNL_MAX_NUM];
	int i = 0;
	int index = 0;
	uint16_t type_tmp;

	fd = Fopen( ADDRESS_FILE, "ab+" ); // 
	if( fd == NULL )
	{
		DEBUG_ERR( "addr file open Err: %s", ADDRESS_FILE );
		return -1;
	}

	memset( addr_list, 0xff, sizeof(terminal_address_list)*SYSTEM_TMNL_MAX_NUM);
	if( terminal_address_list_read_file( fd, addr_list ) == -1 ) // 读取失败
	{
		DEBUG_INFO( "type save read file Err!");
		return -1;
	}

	for( i = 0; i < SYSTEM_TMNL_MAX_NUM; i++ )
	{
		if( addr_list[i].addr == addr )
		{
			type_tmp = addr_list[i].tmn_type;
			addr_list[i].tmn_type = tmnl_type;
			index = i;
			break;
		}
	}

	if( i >= SYSTEM_TMNL_MAX_NUM )
	{
		DEBUG_INFO( "not found addr in the address profile !" );
		return -1;
	}

	if( Fseek( fd, 2, SEEK_SET ) == -1 )
	{
		return -1;
	}

	// 读写检验
	uint16_t crc;
	Fread( fd, &crc, sizeof(uint16_t), 1 );
	crc -= type_tmp; // 减原来的类型
	crc += tmnl_type; // 新校验
	
	if( Fseek( fd, 2, SEEK_SET ) == -1 ) // 检验的偏移
	{
		return -1;
	}
	Fwrite( fd, &crc, sizeof(uint16_t), 1 );
	
	// 写数据
	if( Fseek( fd, index*sizeof(terminal_address_list) + 4/*文件头大小*/, SEEK_SET ) == -1 )
	{
		return -1;
	}

	if( Fwrite( fd, &tmnl_type, sizeof( uint16_t), 1 ) != 1)
	{
		DEBUG_INFO( "write terminal type Err!" );
		return -1;
	}

	Fclose( fd );
	return 0;
}

void terminal_send_upper_message( uint8_t *data_msg, uint16_t addr, uint16_t msg_len )
{
	assert( data_msg );

	if( msg_len > MAX_UPPER_MSG_LEN )
	{
		return;
	}

	assert( dev_terminal_list_guard );
	tmnl_pdblist tmp = found_terminal_dblist_node_by_addr( addr );
	if( tmp == NULL )
	{
		terminal_transmit_upper_cmpt_message( BRDCST_1722_ALL, addr, data_msg, msg_len );
	}
	else
	{
		terminal_transmit_upper_cmpt_message( tmp->tmnl_dev.entity_id, addr, data_msg, msg_len );
	}
	
}

void terminal_tablet_stands_manager( tcmpt_table_card *table_card, uint16_t addr, uint16_t contex_len )// 桌牌管理
{
	assert( table_card );
	uint8_t card_flag = table_card->msg_type;
	tmnl_led_state_show_set card_opt;

	if( card_flag == 0 )
	{
		terminal_socroll_synch();
	}
	else if( card_flag == 1 )// 设置led显示方式
	{
		memcpy( &card_opt, table_card->msg_buf, sizeof(uint16_t));

		assert( dev_terminal_list_guard );
		tmnl_pdblist tmp = found_terminal_dblist_node_by_addr( addr );
		if( tmp == NULL )
		{
			terminal_set_led_play_stype( BRDCST_1722_ALL, addr, card_opt );// 设置led显示方式
		}
		else
		{
			terminal_set_led_play_stype( tmp->tmnl_dev.entity_id, addr, card_opt );
		}
		
	}
}

// 开始签到
void terminal_start_sign_in( tcmpt_begin_sign sign_flag )
{
	assert( dev_terminal_list_guard );
	uint8_t sign_type = sign_flag.sign_type;
	uint8_t timeouts = sign_flag.retroactive_timeouts;
	tmnl_pdblist tmp = dev_terminal_list_guard->next;
	int i = 0;

	set_terminal_system_state( SIGN_STATE, true );
	gtmnl_signstate = SIGN_IN_ON_TIME;
	gsign_latetime = timeouts;
	gsigned_flag = true;

	for( ; tmp != dev_terminal_list_guard; tmp = tmp->next )
	{
		if( tmp->tmnl_dev.address.addr != 0xffff && tmp->tmnl_dev.tmnl_status.is_rgst )
		{
			tmp->tmnl_dev.tmnl_status.sign_state = TMNL_NO_SIGN_IN;
		}
	}

	for( i = 0; i < TMNL_TYPE_NUM; i++)
	{
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0;
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keyup = 0;
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].MicClose = 0;
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].sys = TMNL_SYS_STA_SIGN;
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].sign_stype = sign_type? CARD_SIGN_IN : KEY_SIGN_IN; // 1插卡；0按键
	}

	terminal_state_set_base_type( BRDCST_ALL, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);
}

// 主席控制终端签到
void terminal_chman_control_start_sign_in( uint8_t sign_type, uint8_t timeouts )
{
	assert( dev_terminal_list_guard );
	tmnl_pdblist tmp = dev_terminal_list_guard->next;
	int i = 0;

	set_terminal_system_state( SIGN_STATE, true );
	gtmnl_signstate = SIGN_IN_ON_TIME;
	gsign_latetime = timeouts;
	gsigned_flag = true;

	for( ; tmp != dev_terminal_list_guard; tmp = tmp->next )
	{
		if( tmp->tmnl_dev.address.addr != 0xffff && tmp->tmnl_dev.tmnl_status.is_rgst )
		{
			tmp->tmnl_dev.tmnl_status.sign_state = TMNL_NO_SIGN_IN;
		}
	}

	for( i = 0; i < TMNL_TYPE_NUM; i++)
	{
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0;
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keyup = 0;
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].MicClose = 0;
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].sys = TMNL_SYS_STA_SIGN;
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].sign_stype = sign_type? CARD_SIGN_IN : KEY_SIGN_IN; // 1插卡；0按键
	}

	terminal_state_set_base_type( BRDCST_ALL, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);
}

void terminal_begin_vote( tcmp_vote_start vote_start_flag,  uint8_t* sign_flag )
{
	assert( sign_flag );
	gfirst_key_flag = vote_start_flag.key_effective?true:false;
	uint8_t vote_type = vote_start_flag.vote_type;
	*sign_flag = gsigned_flag; 

	assert( dev_terminal_list_guard );
	tmnl_pdblist tmp = dev_terminal_list_guard->next;

	gvote_mode = (tevote_type)vote_type;
	if( vote_type ==  VOTE_MODE )
	{
		set_terminal_system_state( VOTE_STATE, true );
	}
	else if( vote_type ==  GRADE_MODE )
	{
		set_terminal_system_state( GRADE_STATE, true );
	}
	else
	{
		set_terminal_system_state( ELECT_STATE, true );
	}

	gvote_flag = VOTE_SET;
	for( ; tmp != dev_terminal_list_guard; tmp = tmp->next )
	{
		if( tmp->tmnl_dev.tmnl_status.is_rgst || tmp->tmnl_dev.address.addr )
		{
			continue;
		}

		if( tmp->tmnl_dev.tmnl_status.sign_state != TMNL_NO_SIGN_IN )// 已签到
		{
			tmp->tmnl_dev.tmnl_status.vote_state = TWAIT_VOTE_FLAG;
		}
		else
		{
			tmp->tmnl_dev.tmnl_status.vote_state = TVOTE_SET_FLAG; // 未签到不能投票
		}
	}

	terminal_vote_state_set( BRDCST_ALL );
}

void terminal_chman_control_begin_vote(  uint8_t vote_type, bool key_effective, uint8_t* sign_flag )
{
	assert( sign_flag );
	gfirst_key_flag = key_effective; // true = 首次按键有效；
	*sign_flag = gsigned_flag; 

	assert( dev_terminal_list_guard );
	tmnl_pdblist tmp = dev_terminal_list_guard->next;

	gvote_mode = (tevote_type)vote_type;
	if( vote_type ==  VOTE_MODE )
	{
		set_terminal_system_state( VOTE_STATE, true );
	}
	else if( vote_type ==  GRADE_MODE )
	{
		set_terminal_system_state( GRADE_STATE, true );
	}
	else
	{
		set_terminal_system_state( ELECT_STATE, true );
	}

	gvote_flag = VOTE_SET;
	for( ; tmp != dev_terminal_list_guard; tmp = tmp->next )
	{
		if( tmp->tmnl_dev.tmnl_status.is_rgst || tmp->tmnl_dev.address.addr )
		{
			continue;
		}

		if( tmp->tmnl_dev.tmnl_status.sign_state != TMNL_NO_SIGN_IN )// 已签到
		{
			tmp->tmnl_dev.tmnl_status.vote_state = TWAIT_VOTE_FLAG;
		}
		else
		{
			tmp->tmnl_dev.tmnl_status.vote_state = TVOTE_SET_FLAG; // 未签到不能投票
		}
	}

	terminal_vote_state_set( BRDCST_ALL );
}

void terminal_vote_state_set( uint16_t addr )
{
	tevote_type vote_type = gvote_mode;
	if( addr == 0xffff )
		return;

	gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].one_off = gfirst_key_flag ? 1 : 0;
	gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].VoteType = vote_type;
	gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].MicClose = 0;
	switch( vote_type )
	{
		case VOTE_MODE:
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0x0e; // 2 3 4 键
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keyup = 0;
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].sys = TMNL_SYS_STA_VOTE;
			terminal_state_set_base_type( addr, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);
			terminal_lcd_display_num_send( addr, LCD_OPTION_DISPLAY, VOTE_INTERFACE );
			terminal_led_set_save( addr, TLED_KEY1, TLED_OFF );
			terminal_led_set_save( addr, TLED_KEY2, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY3, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY4, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY5, TLED_OFF );
			fterminal_led_set_send( addr );
			break;
		case GRADE_MODE:
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0x1f; // 1 2 3 4 5 键
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keyup = 0;
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].sys = TMNL_SYS_STA_GRADE;
			terminal_state_set_base_type( addr, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);
			terminal_lcd_display_num_send( addr, LCD_OPTION_DISPLAY, GRADE_1_INTERFACE );
			terminal_led_set_save( addr, TLED_KEY1, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY2, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY3, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY4, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY5, TLED_ON );
			fterminal_led_set_send( addr );
			break;
		case SLCT_2_1:
		case	SLCT_2_2:
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0x03;// 1 2 键
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keyup = 0;
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].sys = TMNL_SYS_STA_SELECT;
			terminal_state_set_base_type( addr, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);
			terminal_lcd_display_num_send( addr, LCD_OPTION_DISPLAY, SLCT_LV_2_INTERFACE );
			terminal_led_set_save( addr, TLED_KEY1, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY2, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY3, TLED_OFF );
			terminal_led_set_save( addr, TLED_KEY4, TLED_OFF );
			terminal_led_set_save( addr, TLED_KEY5, TLED_OFF );
			fterminal_led_set_send( addr );
			break;
		case SLCT_3_1:
		case SLCT_3_2:
		case SLCT_3_3:
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0x07;// 1 2 3 键
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keyup = 0;
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].sys = TMNL_SYS_STA_SELECT;
			terminal_state_set_base_type( addr, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);
			terminal_lcd_display_num_send( addr, LCD_OPTION_DISPLAY, SLCT_LV_3_INTERFACE );
			terminal_led_set_save( addr, TLED_KEY1, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY2, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY3, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY4, TLED_OFF );
			terminal_led_set_save( addr, TLED_KEY5, TLED_OFF );
			fterminal_led_set_send( addr );
			break;
		case SLCT_4_1:
		case SLCT_4_2:
		case SLCT_4_3:
		case SLCT_4_4:
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0x0f; // 1 2 3 键
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keyup = 0;
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].sys = TMNL_SYS_STA_SELECT;
			terminal_state_set_base_type( addr, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);
			terminal_lcd_display_num_send( addr, LCD_OPTION_DISPLAY, SLCT_LV_4_INTERFACE );
			terminal_led_set_save( addr,TLED_KEY1, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY2, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY3, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY4, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY5, TLED_OFF );
			fterminal_led_set_send( addr );
			break;
		case SLCT_5_1:
		case SLCT_5_2:
		case SLCT_5_3:
		case SLCT_5_4:
		case SLCT_5_5:
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0x1f; // 1 2 3 4 5 键
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keyup = 0;
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].sys = TMNL_SYS_STA_VOTE;
			terminal_state_set_base_type( addr, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);
			terminal_lcd_display_num_send( addr, LCD_OPTION_DISPLAY, SLCT_LV_5_INTERFACE );
			terminal_led_set_save( addr, TLED_KEY1, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY2, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY3, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY4, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY5, TLED_ON );
			fterminal_led_set_send( addr );
			break;
		default:
			DEBUG_INFO( "out of bround : vote of type!");
			break;
	}

	terminal_state_all_copy_from_common();
}

void terminal_sign_in_special_event( tmnl_pdblist sign_node ) // 终端特殊事件-签到
{
	assert( sign_node );

	if( gtmnl_signstate == SIGN_IN_ON_TIME )// 设置签到标志
	{
		sign_node->tmnl_dev.tmnl_status.sign_state = TMNL_SIGN_ON_TIME;
	}
	else if( gtmnl_signstate == SIGN_IN_BE_LATE && sign_node->tmnl_dev.tmnl_status.sign_state == TMNL_NO_SIGN_IN )
	{
		sign_node->tmnl_dev.tmnl_status.sign_state = SIGN_IN_BE_LATE;
	}
	
	// 设置投票使能
	termianl_vote_enable_func_handle( sign_node );
	// 上报签到情况
	upper_cmpt_report_sign_in_state( sign_node->tmnl_dev.tmnl_status.sign_state, sign_node->tmnl_dev.address.addr );

	assert( dev_terminal_list_guard );
	tmnl_pdblist tmp = dev_terminal_list_guard->next;
	int sign_num = 0;
	for( ; tmp != dev_terminal_list_guard; tmp = tmp->next )
	{
		if( tmp->tmnl_dev.address.addr != 0xffff && \
			tmp->tmnl_dev.tmnl_status.is_rgst && \
			tmp->tmnl_dev.tmnl_status.sign_state == TMNL_NO_SIGN_IN )
		{
			break;
		}
		
		sign_num++;
	}

	DEBUG_INFO( "sign num = %d", sign_num );
	if( sign_num >= SYSTEM_TMNL_MAX_NUM )// all sign in
	{
		gtmnl_signstate = SIGN_IN_OVER;
		terminal_option_endpoint( BRDCST_1722_ALL, BRDCST_EXE, OPT_TMNL_ALL_SIGN );
	}
}

void termianl_vote_enable_func_handle( tmnl_pdblist sign_node )
{
	assert( sign_node );
	sign_node->tmnl_dev.tmnl_status.vote_state |= TVOTE_EN; // TVOTE_SET_FLAG ->TVOTE_EN ->TWAIT_VOTE_FLAG(投票状态流程)
}

void terminal_state_all_copy_from_common( void )
{
	memcpy( &gtmnl_state_opt[TMNL_TYPE_VIP], &gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST], sizeof(tmnl_state_set));
	memcpy( &gtmnl_state_opt[TMNL_TYPE_CHM_COMMON], &gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST], sizeof(tmnl_state_set));
	memcpy( &gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE], &gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST], sizeof(tmnl_state_set));
}

void terminal_broadcast_end_vote_result( uint16_t addr ) // 根据终端的2 3 4键统计结果
{
	assert( dev_terminal_list_guard );
	tmnl_pdblist tmp = NULL, head_list = dev_terminal_list_guard;
	uint16_t vote_total = 0, neg = 0, abs = 0, aff = 0;
	tmnl_vote_result vote_rslt;

	for( tmp = head_list; tmp != head_list; tmp = tmp->next )
	{
		if( (tmp->tmnl_dev.address.addr == 0xffff) || (!tmp->tmnl_dev.tmnl_status.is_rgst) )
		{
			continue;
		}

		if( tmp->tmnl_dev.tmnl_status.sign_state != TMNL_NO_SIGN_IN )
		{
			vote_total++;
		}

		if( (tmp->tmnl_dev.tmnl_status.vote_state & TVOTE_KEY_MARK) == TVOTE_KEY2_ENABLE )// 2键按下 ,反对
		{
			neg++;
		}
		else if( (tmp->tmnl_dev.tmnl_status.vote_state & TVOTE_KEY_MARK) == TVOTE_KEY3_ENABLE )// 3键按下 ,弃权
		{
			neg++;
		}
		else if( (tmp->tmnl_dev.tmnl_status.vote_state & TVOTE_KEY_MARK) == TVOTE_KEY4_ENABLE )// 4键按下 ,赞成
		{
			neg++;
		}
	}

	vote_rslt.total = vote_total;
	vote_rslt.neg = neg;
	vote_rslt.abs = abs;
	vote_rslt.aff = aff;

	if( (tmp = found_terminal_dblist_node_by_addr( addr )) != NULL )
	{
		terminal_send_vote_result( tmp->tmnl_dev.entity_id, addr, vote_rslt );
	}
	else
	{
		terminal_send_vote_result( BRDCST_1722_ALL, addr, vote_rslt );
	}
}

void terminal_vote( uint16_t addr, uint8_t key_num, uint8_t key_value, uint8_t tmnl_state, const uint8_t recvdata )
{
	assert( dev_terminal_list_guard );
	tmnl_pdblist tmp_node = NULL, tmp_head = dev_terminal_list_guard;
	if( gvote_flag == NO_VOTE || ( key_num > 5 ) ) // 见协议(2.	终端按键的编号：表决键1~5，发言键6，主席优先键7)
	{
		DEBUG_INFO( "system not ready to vote or key num out of vote key num!" );
		return;
	}

	tmp_node = found_terminal_dblist_node_by_addr( addr );
	if( tmp_node == NULL )
	{
		DEBUG_INFO( "no such tmp_node: addr = %04x", addr );
		return;
	}

	// 保存key值
	if( terminal_key_action_value_judge_can_save( key_num,  tmp_node ) ) 
	{// 特殊响应2
		terminal_key_action_host_special_num2_reply( recvdata, 0, 0, 0, VOID_VOTE_INTERFACE ,tmp_node );
	}
	else
	{
		terminal_key_action_host_common_reply( recvdata, tmp_node );
	}

	upper_cmpt_sign_situation_report( tmp_node->tmnl_dev.tmnl_status.vote_state, tmp_node->tmnl_dev.address.addr );

	// 检查是否所有投票完成
	tmnl_pdblist tmp = tmp_head;
	int vote_num = 0;
	for( ; tmp != tmp_head; tmp = tmp->next )
	{
		if( tmp->tmnl_dev.address.addr != 0xffff && \
			tmp->tmnl_dev.tmnl_status.is_rgst && \
			(tmp->tmnl_dev.tmnl_status.mic_state & TWAIT_VOTE_FLAG ) )
		{
			break;
		}
		
		vote_num++;
	}

	DEBUG_INFO( "vote num = %d", vote_num );
	if( vote_num >= SYSTEM_TMNL_MAX_NUM )
	{
		terminal_option_endpoint( BRDCST_1722_ALL, BRDCST_EXE, OPT_TMNL_ALL_VOTE );
	}
}

bool terminal_key_action_value_judge_can_save( uint8_t key_num,  tmnl_pdblist vote_node )
{
	assert( vote_node );
	bool ret = false;
	uint8_t *p_vote_state = &vote_node->tmnl_dev.tmnl_status.vote_state;
	assert( p_vote_state );

	if( !gfirst_key_flag )	// last key effective 
	{
		*p_vote_state &= (~TVOTE_KEY_MARK);
		*p_vote_state |= (1<< ( key_num -1));
		return ret;
	}

	switch(gvote_mode)
	{
		case VOTE_MODE:
		case GRADE_MODE:
		case SLCT_2_1:
		case SLCT_3_1:
		case SLCT_4_1:
		case SLCT_5_1:
			*p_vote_state |= (1<< ( key_num -1));
			*p_vote_state &= (~TWAIT_VOTE_FLAG);// 设置结束标志
			ret = true;
			break;
		case SLCT_2_2:
		case SLCT_3_2:
		case SLCT_4_2:
		case SLCT_5_2:
			*p_vote_state |= (1<< ( key_num -1));
			if(COUNT_KEY_DOWN_NUM( *p_vote_state & TVOTE_KEY_MARK ) >= 2 )
			{
				*p_vote_state &= (~TWAIT_VOTE_FLAG);// 设置结束标志
				ret = true;
			}
			break;
		case SLCT_3_3:
		case SLCT_4_3:
		case SLCT_5_3:
			*p_vote_state |= (1<< ( key_num -1));
			if(COUNT_KEY_DOWN_NUM( *p_vote_state & TVOTE_KEY_MARK ) >= 3 )
			{
				*p_vote_state &= (~TWAIT_VOTE_FLAG);// 设置结束标志
				ret = true;
			}
			break;
		case SLCT_4_4:
		case SLCT_5_4:
			*p_vote_state |= (1<< ( key_num -1));
			if(COUNT_KEY_DOWN_NUM( *p_vote_state & TVOTE_KEY_MARK ) >= 4 )
			{
				*p_vote_state &= (~TWAIT_VOTE_FLAG);// 设置结束标志
				ret = true;
			}
			break;
		case SLCT_5_5:
			*p_vote_state |= (1<< ( key_num -1));
			if(COUNT_KEY_DOWN_NUM( *p_vote_state & TVOTE_KEY_MARK ) >= 5 )
			{
				*p_vote_state &= (~TWAIT_VOTE_FLAG);// 设置结束标志
				ret = true;
			}
			break;
		default:
			break;
	}

	return ret;
}

void terminal_key_action_host_special_num2_reply( const uint8_t recvdata, uint8_t key_down, uint8_t key_up, uint16_t key_led, uint8_t lcd_num, tmnl_pdblist node )
{
	assert( node );
	uint8_t data_len;
	tka_special2_reply reply_data;
	reply_data.recv_data = recvdata;
	reply_data.reply_num = REPLY_SPECAIL_NUM2;
	reply_data.key_down = key_down & TVOTE_KEY_MARK;
	reply_data.key_up = key_up & TVOTE_KEY_MARK;
	reply_data.sys = recvdata & KEY_ACTION_TMN_STATE_MASK;
	reply_data.key_led = key_led & 0x03ff;// 低十位
	reply_data.lcd_num = lcd_num;
	data_len = SPECIAL2_REPLY_KEY_AC_DATA_LEN;

	terminal_key_action_host_reply( node->tmnl_dev.entity_id, node->tmnl_dev.address.addr, data_len, NULL, NULL, &reply_data );
}

void terminal_key_action_host_special_num1_reply( const uint8_t recvdata, uint8_t mic_state, tmnl_pdblist node )
{
	assert( node );
	uint8_t data_len;
	tka_special1_reply reply_data;
	reply_data.mic_state = mic_state;
	reply_data.reply_num = REPLY_SPECAIL_NUM1;
	reply_data.recv_data = recvdata;
	data_len = SPECIAL1_REPLY_KEY_AC_DATA_LEN;

	terminal_key_action_host_reply( node->tmnl_dev.entity_id, node->tmnl_dev.address.addr, data_len, NULL, &reply_data, NULL );
}

void terminal_key_action_host_common_reply( const uint8_t recvdata, tmnl_pdblist node )
{
	assert( node );
	uint8_t data_len;
	tka_common_reply common_data;
	common_data.recv_data = recvdata;
	data_len = COMMON_REPLY_KEY_AC_DATA_LEN;
	
	terminal_key_action_host_reply( node->tmnl_dev.entity_id, node->tmnl_dev.address.addr, data_len, &common_data, NULL, NULL );
}

void terminal_key_speak( uint16_t addr, uint8_t key_num, uint8_t key_value, uint8_t tmnl_state, const uint8_t recvdata )
{
	assert( dev_terminal_list_guard );
	tmnl_pdblist tmp_node = NULL;
	
	tmp_node = found_terminal_dblist_node_by_addr( addr );
	if( tmp_node == NULL )
	{
		DEBUG_INFO( "no such tmp_node: addr = %04x", addr );
		return;
	}

	if( key_num == KEY6_SPEAK )
	{
		uint8_t mic_state;
		if( key_value )
		{
			// no limit time
			mic_state = MIC_OPEN_STATUS;
			connect_table_tarker_connect( tmp_node->tmnl_dev.entity_id, 0, tmp_node, true, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send );
		}
		else
		{
			mic_state = MIC_COLSE_STATUS;
			connect_table_tarker_disconnect( tmp_node->tmnl_dev.entity_id, tmp_node, true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
		}

		terminal_key_action_host_special_num1_reply( recvdata, mic_state, tmp_node );
	}
}

void terminal_key_action_chman_interpose( uint16_t addr, uint8_t key_num, uint8_t key_value, uint8_t tmnl_state, const uint8_t recvdata )
{
	if( key_num != KEY7_CHAIRMAN_FIRST )
	{
		DEBUG_INFO( "not valid chairman interpose value key " );
		return;
	}

	tmnl_pdblist tmp_node = found_terminal_dblist_node_by_addr( addr );
	if( tmp_node == NULL )
	{
		DEBUG_INFO( "no such 0x%04x addr chairman!", addr );
		return;
	}

	if( key_value )
	{
		terminal_chairman_interpose( addr, true, tmp_node, recvdata );
	}
	else
	{
		terminal_chairman_interpose( addr, false, tmp_node, recvdata );
	}
}

void terminal_chairman_interpose( uint16_t addr, bool key_down, tmnl_pdblist chman_node, const uint8_t recvdata )
{
	tcmpt_data_mic_status mic_list[SYSTEM_TMNL_MAX_NUM]; // 100-临时发言总人数
	uint16_t report_mic_num = 0;
	thost_system_set set_sys; // 系统配置文件的格式
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));

	assert( chman_node );
	if( chman_node == NULL )
		return;
	
	if( (key_down && gchm_int_ctl.is_int) ||\
		((!key_down) && (!gchm_int_ctl.is_int)) ||\
		((!key_down) && (gchm_int_ctl.chmaddr != addr)))
	{
		terminal_key_action_host_common_reply( recvdata, chman_node );
		return;
	}

	if( (get_sys_state() != INTERPOSE_STATE) && key_down )
	{
		bool tmp_close = false; // temp close
		
		set_terminal_system_state( INTERPOSE_STATE, true );
		gchm_int_ctl.is_int = true;
		gchm_int_ctl.chmaddr = addr;
		tmp_close = (set_sys.temp_close != 0)?true:false; 

		terminal_key_action_host_special_num1_reply( recvdata, MIC_CHM_INTERPOSE_STATUS, chman_node );// 设置主席mic状态
		terminal_mic_state_set( MIC_CHM_INTERPOSE_STATUS, BRDCST_ALL, BRDCST_1722_ALL, true, chman_node );

		/**
		 *2015-12-11
		 *打开主席mic,不保存状态
		 */
		if( chman_node->tmnl_dev.tmnl_status.mic_state != MIC_OPEN_STATUS );
			connect_table_tarker_connect( chman_node->tmnl_dev.entity_id,\
				0, chman_node, false, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send ); // 这里不保存其mic open的状态2015-12-11

		assert( dev_terminal_list_guard );
		tmnl_pdblist tmp_node = dev_terminal_list_guard->next;
		
		for( ; tmp_node != dev_terminal_list_guard; tmp_node = tmp_node->next )
		{
			if( tmp_node->tmnl_dev.address.tmn_type == TMNL_TYPE_COMMON_RPRST )
			{// 关闭所有普通代表机
				if( tmp_node->tmnl_dev.tmnl_status.mic_state != MIC_COLSE_STATUS )
				{
					connect_table_tarker_disconnect( tmp_node->tmnl_dev.entity_id, tmp_node, !tmp_close, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
					if( report_mic_num <= SYSTEM_TMNL_MAX_NUM )
					{
						mic_list[report_mic_num].addr.low_addr = (uint8_t)((tmp_node->tmnl_dev.address.addr&0x00ff) >> 0);
						mic_list[report_mic_num].addr.high_addr = (uint8_t)((tmp_node->tmnl_dev.address.addr&0xff00) >> 0);
						mic_list[report_mic_num].switch_flag = MIC_COLSE_STATUS;
						report_mic_num++;
					}
				}
			}
		}

		// 上报mic状态
		cmpt_miscrophone_status_list_from_set( mic_list, report_mic_num );
		
		gdisc_flags.apply_num = 0;
		gdisc_flags.speak_limit_num = 0;		
	}
	else if( !key_down )
	{
		set_terminal_system_state( INTERPOSE_STATE, false );
		assert(dev_terminal_list_guard);
		tmnl_pdblist end_node = dev_terminal_list_guard->next;
		
		terminal_key_action_host_special_num1_reply( recvdata, chman_node->tmnl_dev.tmnl_status.mic_state, chman_node );

		/**
		 *2015-12-11
		 *若主席mic的上一个状态是打开的状态不去管它，
		 *否则断开其mic，而不重新设置保存
		 */
		if( chman_node->tmnl_dev.tmnl_status.mic_state != MIC_OPEN_STATUS )
			connect_table_tarker_disconnect( chman_node->tmnl_dev.entity_id, chman_node, false, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
		
		for( ;end_node != dev_terminal_list_guard; end_node = end_node->next )
		{
			if( (end_node->tmnl_dev.address.addr != 0xffff)&&\
				(end_node->tmnl_dev.tmnl_status.is_rgst) &&\
				(end_node->tmnl_dev.tmnl_status.mic_state == MIC_OPEN_STATUS) &&\
				((end_node->tmnl_dev.address.tmn_type == TMNL_TYPE_COMMON_RPRST) ||(end_node->tmnl_dev.address.tmn_type == TMNL_TYPE_VIP)))
			{
				connect_table_tarker_connect( end_node->tmnl_dev.entity_id, set_sys.spk_limtime, end_node, true, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send );
			}
		}

		int i = 0;
		for( i = 0; i < gdisc_flags.apply_num; i++ )
		{
			uint16_t addr_apply = gdisc_flags.apply_addr_list[i];
			tmnl_pdblist tmp_node = found_terminal_dblist_node_by_addr(addr_apply);
			if( tmp_node == NULL )
			{
				continue;
			}
			
			if( i == gdisc_flags.currect_first_index )
			{
				terminal_mic_state_set( MIC_FIRST_APPLY_STATUS, addr_apply, tmp_node->tmnl_dev.entity_id, true, tmp_node );
			}
			else
			{
				terminal_mic_state_set( MIC_OTHER_APPLY_STATUS, addr_apply, tmp_node->tmnl_dev.entity_id, true, tmp_node );
			}
		}

		gchm_int_ctl.is_int = false;
		gchm_int_ctl.chmaddr = 0xffff;
	}
}

int terminal_key_discuccess( uint16_t addr, uint8_t key_num, uint8_t key_value, uint8_t tmnl_state, uint8_t recv_msg )
{
	tmnl_pdblist dis_node;
	
	dis_node = found_terminal_dblist_node_by_addr( addr );
	if( dis_node == NULL )
	{
		DEBUG_INFO( "not found discuccess termianl key!" );
		return -1;
	}

	if( key_num == KEY6_SPEAK )
	{
		if( key_value )
		{
			terminal_key_speak_proccess( dis_node, true, recv_msg );
		}
		else
		{
			terminal_key_speak_proccess( dis_node, false, recv_msg );
		}
	}

	return 0;
}

bool terminal_key_speak_proccess( tmnl_pdblist dis_node, bool key_down, uint8_t recv_msg )
{
	uint8_t dis_mode = gdisc_flags.edis_mode;
	thost_system_set set_sys; // 系统配置文件的格式
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));

	if( dis_node == NULL )
		return false;

	if( !dis_node->tmnl_dev.tmnl_status.is_rgst )
		return false;
	
	DEBUG_INFO( "dis mode = %d ", dis_mode );
	if( dis_mode == PPT_MODE ||\
			(dis_node->tmnl_dev.address.tmn_type == TMNL_TYPE_VIP) ||\
			(dis_node->tmnl_dev.address.tmn_type == TMNL_TYPE_CHM_COMMON)||\
			(dis_node->tmnl_dev.address.tmn_type == TMNL_TYPE_CHM_EXCUTE))
	{
		if( key_down )
		{
			terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, dis_node );
			connect_table_tarker_connect( dis_node->tmnl_dev.entity_id, 0, dis_node, true, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send );
		}
		else
		{
			terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, dis_node );
			connect_table_tarker_disconnect( dis_node->tmnl_dev.entity_id, dis_node, true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
		}
	}
	else
	{
		switch( dis_mode )
		{
			case FREE_MODE:
			{
				terminal_free_disccuss_mode_pro( key_down, set_sys.spk_limtime, dis_node, recv_msg );
				break;
			}
			case LIMIT_MODE:
			{
				terminal_limit_disccuss_mode_pro( key_down, set_sys.spk_limtime, dis_node, recv_msg );
				break;
			}
			case FIFO_MODE:
			{
				terminal_fifo_disccuss_mode_pro( key_down, set_sys.spk_limtime, dis_node, recv_msg );
				break;
			}
			case APPLY_MODE:
			{
				terminal_apply_disccuss_mode_pro( key_down, set_sys.spk_limtime, dis_node, recv_msg );
				break;
			}
			default:
			{
				DEBUG_INFO( " out of discuss mode bound!" );
				break;
			}
		}
	}
	
	return true;
}

int terminal_chairman_apply_reply( uint8_t tmnl_type, uint16_t addr, uint8_t key_num, uint8_t key_value, uint8_t tmnl_state, uint8_t recv_msg )
{
	tmnl_pdblist apply_node;
	
	apply_node = found_terminal_dblist_node_by_addr( addr );
	if( apply_node == NULL )
	{
		return -1;
	}
	
	if( tmnl_type != COMMAND_TMN_CHAIRMAN )
	{
		return -1;
	}

	if( tmnl_state == TMNL_SYS_STA_VOTE || tmnl_state == TMNL_SYS_STA_GRADE || tmnl_state == TMNL_SYS_STA_SELECT )
	{
		return -1;
	}

	enum_apply_pro apply_flag = REFUSE_APPLY;
	if( (key_num == KEY2_VOTE) && key_value )
	{
		apply_flag = REFUSE_APPLY;
		terminal_key_action_host_common_reply( recv_msg, apply_node );
		terminal_examine_apply( apply_flag );
	}
	else if( (key_num == KEY3_VOTE) && key_value )
	{
		apply_flag = NEXT_APPLY;
		terminal_key_action_host_common_reply( recv_msg, apply_node );
		terminal_examine_apply( apply_flag );
	}
	else if( (key_num == KEY4_VOTE) && key_value )
	{
		apply_flag = APPROVE_APPLY;
		terminal_key_action_host_common_reply( recv_msg, apply_node );
		terminal_examine_apply( apply_flag );
	}

	return 0;
}

void terminal_free_disccuss_mode_pro( bool key_down, uint8_t limit_time,tmnl_pdblist speak_node, uint8_t recv_msg )
{
	assert( speak_node );
	if( key_down )
	{
		if( gdisc_flags.speak_limit_num < FREE_MODE_SPEAK_MAX )
		{
			terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, speak_node );
			connect_table_tarker_connect( speak_node->tmnl_dev.entity_id, 0, speak_node, true, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send );
		}
	}
	else
	{
		terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );
		connect_table_tarker_disconnect( speak_node->tmnl_dev.entity_id, speak_node, true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );	
	}

	terminal_main_state_send( 0, NULL, 0 );
}

bool terminal_limit_disccuss_mode_pro( bool key_down, uint8_t limit_time,tmnl_pdblist speak_node, uint8_t recv_msg )
{
	assert( speak_node );

	bool ret = false;
	if( speak_node == NULL )
	{
		return ret;
	}
		
	uint16_t addr = speak_node->tmnl_dev.address.addr;
	uint16_t current_addr = 0;
	uint8_t cc_state = 0;
	tmnl_pdblist first_apply = NULL; // 首位申请发言
	if( key_down ) // 打开麦克风
	{
		if( speak_node->tmnl_dev.tmnl_status.mic_state == MIC_OPEN_STATUS )
		{
			terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, speak_node );
			ret = true;
		}
		else if(speak_node->tmnl_dev.tmnl_status.mic_state == MIC_OTHER_APPLY_STATUS)
		{
			terminal_key_action_host_special_num1_reply( recv_msg, MIC_OTHER_APPLY_STATUS, speak_node );
			ret = true;
		}
		else if(speak_node->tmnl_dev.tmnl_status.mic_state == MIC_FIRST_APPLY_STATUS)
		{
			terminal_key_action_host_special_num1_reply( recv_msg, MIC_FIRST_APPLY_STATUS, speak_node );
			ret = true;
		}
		else if( gdisc_flags.speak_limit_num < gdisc_flags.limit_num ) // 打开麦克风
		{
			terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, speak_node );
			connect_table_tarker_connect( speak_node->tmnl_dev.entity_id, limit_time, speak_node, true, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send );
			ret = true;
		}
		else if( gdisc_flags.apply_num < gdisc_flags.apply_limit ) // 申请发言
		{
			uint8_t state = MIC_OTHER_APPLY_STATUS;
			if( 0 == gdisc_flags.apply_num )
			{
				gdisc_flags.currect_first_index = 0;
				state = MIC_FIRST_APPLY_STATUS;
			}
			gdisc_flags.apply_addr_list[gdisc_flags.apply_num] = speak_node->tmnl_dev.address.addr;
			gdisc_flags.apply_num++;

			terminal_key_action_host_special_num1_reply( recv_msg, state, speak_node );
			terminal_main_state_send( 0, NULL, 0 );
			ret = true;
		}
	}
	else
	{
		terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );
		connect_table_tarker_disconnect( speak_node->tmnl_dev.entity_id, speak_node, true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
		current_addr = gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index];
		cc_state = speak_node->tmnl_dev.tmnl_status.mic_state;
		
		if( cc_state == MIC_FIRST_APPLY_STATUS || cc_state == MIC_OTHER_APPLY_STATUS )
		{
			addr_queue_delect_by_value( gdisc_flags.apply_addr_list, &gdisc_flags.apply_num, addr );

			if( gdisc_flags.apply_num > 0 && current_addr == addr )// 置下一个申请为首位申请状态
			{
				gdisc_flags.currect_first_index %= gdisc_flags.apply_num;
				first_apply = found_terminal_dblist_node_by_addr( gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index]);
				if( first_apply != NULL )
				{
					terminal_mic_state_set( MIC_FIRST_APPLY_STATUS, speak_node->tmnl_dev.address.addr, speak_node->tmnl_dev.entity_id, true, speak_node );
				}
			}

			terminal_main_state_send( 0, NULL, 0 );
			ret = true;
		}
		else if( cc_state == MIC_OPEN_STATUS )
		{
			if( gdisc_flags.speak_limit_num > 0 )
			{
				gdisc_flags.speak_limit_num--;
			}

			if(gdisc_flags.speak_limit_num < gdisc_flags.limit_num && gdisc_flags.apply_num > 0 )// 结束发言,并开始下一个申请终端的发言
			{
				if( addr_queue_delete_by_index( gdisc_flags.apply_addr_list, &gdisc_flags.apply_num, gdisc_flags.currect_first_index) )// 开启下一个申请话筒
				{
					tmnl_pdblist first_speak = found_terminal_dblist_node_by_addr( current_addr );
					if( first_speak != NULL )
					{
						connect_table_tarker_connect( first_speak->tmnl_dev.entity_id, limit_time, first_speak, true, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send );

						if( gdisc_flags.apply_num > 0 ) // 设置首位申请发言终端
						{
							gdisc_flags.currect_first_index %= gdisc_flags.apply_num;
							first_apply = found_terminal_dblist_node_by_addr( gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index]);
							if( first_apply != NULL )
							{
								terminal_mic_state_set( MIC_FIRST_APPLY_STATUS, speak_node->tmnl_dev.address.addr, speak_node->tmnl_dev.entity_id, true, speak_node );
							}
							else
							{
								DEBUG_INFO( " no such tmnl dblist node!");
							}
						}
						else
						{
							gdisc_flags.currect_first_index = gdisc_flags.apply_num;
						}
					}
					else
					{
						DEBUG_INFO( " no such tmnl dblist node!");
					}
				}
				else
				{
					gdisc_flags.currect_first_index = 0;
				}
			}

			terminal_main_state_send( 0, NULL, 0 );
			ret = true;
		}
	}

	return ret;
	
}

bool terminal_fifo_disccuss_mode_pro( bool key_down, uint8_t limit_time,tmnl_pdblist speak_node, uint8_t recv_msg )
{
	assert( speak_node );

	bool ret = false;
	uint16_t addr = speak_node->tmnl_dev.address.addr;
	if( speak_node == NULL )
	{
		DEBUG_INFO( " NULL speak node!" );
		return false;
	}

	uint8_t speak_limit_num = gdisc_flags.speak_limit_num;
	if( key_down ) // 打开话筒
	{
		if( addr_queue_find_by_value( gdisc_flags.speak_addr_list, speak_limit_num, addr, NULL))
		{
			terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, speak_node );
			connect_table_tarker_connect( speak_node->tmnl_dev.entity_id, limit_time, speak_node, true, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send );
			ret = true;
		}
		else if( speak_limit_num < gdisc_flags.limit_num )
		{
			terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, speak_node );
			connect_table_tarker_connect( speak_node->tmnl_dev.entity_id, limit_time, speak_node, true, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send );
			gdisc_flags.speak_addr_list[speak_limit_num] = addr;
			ret = true;
		}
		else // 发言人数大于或等于限制人数
		{
			if( gdisc_flags.speak_addr_list[0] != 0xffff ) // 先进先出
			{
				tmnl_pdblist first_speak = found_terminal_dblist_node_by_addr( gdisc_flags.speak_addr_list[0] );
				if( first_speak != NULL )
				{
					connect_table_tarker_disconnect( first_speak->tmnl_dev.entity_id, first_speak, true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
				}
				else
				{
					DEBUG_INFO( "fifo not found tmnl list node!");
				}
				
				addr_queue_delete_by_index( gdisc_flags.speak_addr_list, &gdisc_flags.speak_limit_num, 0 );// 首位发言删除

				uint8_t speak_limit_num1 = gdisc_flags.speak_limit_num;
				terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, speak_node );
				connect_table_tarker_connect( speak_node->tmnl_dev.entity_id, limit_time, speak_node, true, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send );
				gdisc_flags.speak_addr_list[speak_limit_num1] = speak_node->tmnl_dev.address.addr;
				ret = true;
			}
		}
	}
	else
	{
		addr_queue_delect_by_value( gdisc_flags.speak_addr_list, &gdisc_flags.speak_limit_num, speak_node->tmnl_dev.address.addr );
		terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );
		connect_table_tarker_disconnect( speak_node->tmnl_dev.entity_id, speak_node, true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
		ret = true;
	}

	return ret;
}

bool terminal_apply_disccuss_mode_pro( bool key_down, uint8_t limit_time,tmnl_pdblist speak_node, uint8_t recv_msg )
{
	assert( speak_node );

	if( speak_node == NULL )
	{
		return false;
	}

	bool ret = false;
	uint16_t addr = speak_node->tmnl_dev.address.addr;
	uint16_t current_addr = 0;
	tmnl_pdblist first_apply = NULL;

	if( key_down ) // 申请发言,加地址入申请列表
	{
		if(  gdisc_flags.apply_num < gdisc_flags.apply_limit )
		{
			uint8_t state = MIC_OTHER_APPLY_STATUS;
			if( 0 == gdisc_flags.apply_num )
			{
				gdisc_flags.currect_first_index = 0;
				state = MIC_FIRST_APPLY_STATUS;
			}
			gdisc_flags.apply_addr_list[gdisc_flags.apply_num] = speak_node->tmnl_dev.address.addr;
			gdisc_flags.apply_num++;

			terminal_key_action_host_special_num1_reply( recv_msg, state, speak_node );
			//terminal_mic_state_set( state, speak_node->tmnl_dev.address.addr, speak_node->tmnl_dev.entity_id, true, speak_node );
			terminal_main_state_send( 0, NULL, 0 );
			ret = true;
		}
	}
	else // 取消申请发言
	{
		
		current_addr = gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index];
		if(addr_queue_delect_by_value( gdisc_flags.apply_addr_list, &gdisc_flags.apply_num, addr ))
		{
			if( gdisc_flags.apply_num > 0 && current_addr == addr )// 置下一个申请为首位申请状态
			{
				gdisc_flags.currect_first_index %= gdisc_flags.apply_num;
				first_apply = found_terminal_dblist_node_by_addr( gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index] );
				if( first_apply != NULL )
				{
					terminal_key_action_host_special_num1_reply( recv_msg, MIC_FIRST_APPLY_STATUS, speak_node );
					//terminal_mic_state_set( MIC_FIRST_APPLY_STATUS, speak_node->tmnl_dev.address.addr, speak_node->tmnl_dev.entity_id, true, speak_node );
				}
			}
			
			terminal_main_state_send( 0, NULL, 0 );
		}	
		else
		{
			terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );
			connect_table_tarker_disconnect( speak_node->tmnl_dev.entity_id, speak_node, true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
		}
		
		ret = true;
	}

	return ret;
}

void terminal_key_preset( uint8_t tmnl_type, uint16_t tmnl_addr, uint8_t tmnl_state, uint8_t key_num, uint8_t key_value )
{
	uint16_t addr;
	if( key_value )
	{
		addr = tmnl_addr;
		find_func_command_link( SYSTEM_USE, SYS_PRESET_ADDR, 0, (uint8_t*)&addr, sizeof(uint16_t) );
	}
}

int terminal_speak_track( uint16_t addr, bool track_en )// 摄像跟踪接口
{
	uint16_t temp;
	uint16_t i;
	uint16_t index;
	uint8_t cmr_track;

	cmr_track = gset_sys.camara_track;
 	if( cmr_track )
	{
		return -1;
	}
	
	for( i = 0; i <gspeaker_track.spk_num; i++)
	{
		if( addr == gspeaker_track.spk_addrlist[i])
		{
			break;
		}
	}
	
	index = i;
	if( index < gspeaker_track.spk_num )
	{
		if( gspeaker_track.spk_num > 1)
		{
			for( i = index; i < (gspeaker_track.spk_num-1); i++)
			{
				gspeaker_track.spk_addrlist[i] = gspeaker_track.spk_addrlist[i+1];
			}
			
			gspeaker_track.spk_num--;
		}
		else if( gspeaker_track.spk_num == 1 )
		{
			gspeaker_track.spk_addrlist[0] = 0xFFFF;
			gspeaker_track.spk_num = 0;
		}
	}
	
	if( track_en )
	{
		if( gspeaker_track.spk_num < MAX_SPK_NUM )
		{
			gspeaker_track.spk_addrlist[gspeaker_track.spk_num] = addr;
			gspeaker_track.spk_num++;
			temp = addr;
			find_func_command_link( SYSTEM_USE, SYS_GET_PRESET, 0, (uint8_t*)&temp,sizeof(uint16_t) );
		}
	}
	else
	{
		if( 0 == gspeaker_track.spk_num )
		{
			temp = FULL_VIEW_ADDR;
		}
		else
		{
			temp = gspeaker_track.spk_addrlist[gspeaker_track.spk_num-1];
		}
		
		find_func_command_link( SYSTEM_USE, SYS_GET_PRESET, 0, (uint8_t*)&temp,sizeof(uint16_t) );
	}

	return 0;
}

/*===================================================
终端处理流程@}
=====================================================*/

/*==================================================
	start reallot address
====================================================*/
//清除终端链表
tmnl_pdblist terminal_system_dblist_except_free( void )
{
	tmnl_pdblist p_node = NULL;
	
	p_node = terminal_dblist_except_free( dev_terminal_list_guard );

	return p_node;
}

void terminal_open_addr_file_wt_wb( void )
{
	addr_file_fd = Fopen( ADDRESS_FILE, "wb+");
	if( addr_file_fd == NULL )
	{
		DEBUG_ERR( "terminal_open_addr_file_wt_wb open fd  Err!" );
		assert( NULL != addr_file_fd );
	}
	
	Fclose( addr_file_fd );	
}

// 摧毁终端链表
void terminal_system_dblist_destroy( void )
{
	tmnl_pdblist p_node = destroy_terminal_dblist( dev_terminal_list_guard );
	if( NULL != p_node )
	{
		DEBUG_INFO( "destroy terminal double list not success!" );
	}
}
	
/*===================================================
end reallot address
=====================================================*/

