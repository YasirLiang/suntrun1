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

FILE* addr_file_fd = NULL; 		// 终端地址信息读取文件描述符
terminal_address_list tmnl_addr_list[SYSTEM_TMNL_MAX_NUM];	// 终端地址分配列表
terminal_address_list_pro allot_addr_pro;	
tmnl_pdblist dev_terminal_list_guard = NULL; // 终端链表表头结点，对其正确地操作，必须先注册完终端
bool reallot_flag = false; // 重新分配标志
tmnl_state_set gtmnl_state_opt[TMNL_TYPE_NUM];
tsys_discuss_pro gdisc_flags; // 系统讨论参数
tchairman_control_in gchm_int_ctl; // 主席插话
ttmnl_register_proccess gregister_tmnl_pro; // 终端报到处理
uint32_t speak_limit_time = 0; // 发言时长， 0表示无限时；1-63表示限时1-63分钟
uint8_t glcd_num = 0; // lcd 显示的屏号
uint8_t gled_buf[2] = {0}; // 终端指示灯

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

inline void init_terminal_device_double_list()
{
	// init terminal system double list
	init_terminal_dblist( &dev_terminal_list_guard );
	assert( dev_terminal_list_guard != NULL );
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
				bret = true;
				break;
			}
		}
	}

	return bret;
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
	DEBUG_LINE();
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
			}
			else
			{
				p_addr_list[p_allot->index].tmn_type = TMNL_TYPE_COMMON_RPRST;
			}

			// save new addr to file
			addr_file_fd = Fopen( ADDRESS_FILE, "ab+");
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
		if( msg.data && (!p_allot->renew_flag) && (p_addr_list[p_allot->index].addr != 0xffff))
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
	FILE* fd = NULL;
	uint8_t auto_close = 0;
	int i = 0;
	tmnl_pdblist tmnl_node = dev_terminal_list_guard->next;
	
	fd = Fopen( STSTEM_SET_STUTUS_PROFILE, "rb" );
	if( NULL == fd )
	{
		DEBUG_INFO( "open files %s Err!",  STSTEM_SET_STUTUS_PROFILE );
		return -1;
	}

	if(profile_system_file_read_byte( fd, &auto_close, VAL_AUTO_CLOSE, sizeof(uint8_t)) == -1)
	{	
		Fclose( fd ); // fd must be closed
		return -1;
	}

	/* 设置终端的麦克风状态*/
	for( i = 0; i < TMNL_TYPE_NUM; i++)
	{
		gtmnl_state_opt[i].auto_close = auto_close?1:0;
		gtmnl_state_opt[i].MicClose = MIC_CLOSE;
	}
	
	/*关闭所有麦克风，这里需要一个机制，即通道分配机制与麦克风设置机制(这时未实现10/29), 使用连接表管理系统的麦克风的连接状态，暂时未考虑同步的问题(11/4)*/
	for( ; tmnl_node != dev_terminal_list_guard; tmnl_node = tmnl_node->next )
	{
		connect_table_tarker_disconnect( tmnl_node->tmnl_dev.entity_id );
		if( tmnl_node->tmnl_dev.tmnl_status.is_rgst )
		{
			terminal_mic_state_set( MIC_COLSE_STATUS, tmnl_node->tmnl_dev.address.addr, tmnl_node->tmnl_dev.entity_id, true, tmnl_node );
		}
	}

	/*发送主机状态*/
	terminal_main_state_send( 0, NULL, 0 );

	Fclose( fd ); // fd must be closed
	return 0;
}

/*主机发送状态, */
int terminal_main_state_send( uint16_t cmd, void *data, uint32_t data_len )
{
	assert( dev_terminal_list_guard );
	FILE* fd = NULL;
	tmnl_main_state_send host_main_state;
	thost_system_set set_sys; // 系统配置文件的格式
	uint8_t spk_num = 0;
	tmnl_pdblist p_tmnl_list = dev_terminal_list_guard->next;

	fd = Fopen( STSTEM_SET_STUTUS_PROFILE, "rb" ); // 只读读出数据
	if( NULL == fd )
	{
		DEBUG_INFO( "mian state send ->open files %s Err!",  STSTEM_SET_STUTUS_PROFILE );
		return -1;
	}
	if( profile_system_file_read( fd, &set_sys ) == -1)
	{
		DEBUG_INFO( "Read profile system Err!" );
		Fclose( fd );
		return -1;
	}

	host_main_state.unit = gregister_tmnl_pro.tmn_total;
	host_main_state.camera_follow = set_sys.camara_track ? 1 : 0;
	host_main_state.chm_first = set_sys.temp_close ? 1 : 0;
	host_main_state.conference_stype = set_sys.discuss_mode;
	host_main_state.limit = set_sys.speak_limit; 		// 讲话人数上限
	host_main_state.apply_set = set_sys.apply_limit;	// 申请人数上限
	
	for( ;p_tmnl_list != dev_terminal_list_guard; p_tmnl_list = p_tmnl_list->next )
	{
		if( p_tmnl_list->tmnl_dev.address.addr != 0xffff && \
			(p_tmnl_list->tmnl_dev.tmnl_status.mic_state==MIC_OPEN_STATUS))
			spk_num++;
	}
	host_main_state.spk_num = spk_num; // 当前讲话人数
	host_main_state.apply = gdisc_flags.apply_num;

	terminal_host_send_state( BRDCST_1722_ALL, host_main_state ); // target id is 0

	Fclose( fd );
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
	assert( data );
	terminal_option_endpoint( BRDCST_1722_ALL, CONFERENCE_BROADCAST_ADDRESS, OPT_TMNL_SUSPEND_VOTE );

	return 0;
}

/*重新投票*/
int terminal_regain_vote( uint16_t cmd, void *data, uint32_t data_len )
{
	assert( data );
	terminal_option_endpoint( BRDCST_1722_ALL, CONFERENCE_BROADCAST_ADDRESS, OPT_TMNL_RECOVER_VOTE );

	return 0;
}

int terminal_system_discuss_mode_set( uint16_t cmd, void *data, uint32_t data_len )
{
	assert( data && dev_terminal_list_guard );
	uint8_t dis_mode = *((uint8_t*)data);
	tmnl_pdblist tmnl_node = dev_terminal_list_guard->next;
		
	gdisc_flags.edis_mode = (ttmnl_discuss_mode)dis_mode;
	gdisc_flags.currect_first_index = 0;
	gdisc_flags.apply_num = 0;
	gdisc_flags.speak_limit_num = 0;

	/*关闭所有麦克风*/
	for( ; tmnl_node != dev_terminal_list_guard; tmnl_node = tmnl_node->next )
	{
		// 1.查看连接表，断开所有的连接(1722.1协议),暂时不考虑同步的问题11/3
		connect_table_tarker_disconnect( tmnl_node->tmnl_dev.entity_id );
		
		// 2.设置麦克风tarker的状态,上报麦克风状态, 设置相应终端的麦克风状态(会议主机与终端协议)
		if( tmnl_node->tmnl_dev.tmnl_status.is_rgst )
		{
			terminal_mic_state_set( MIC_COLSE_STATUS, tmnl_node->tmnl_dev.address.addr, tmnl_node->tmnl_dev.entity_id,true, tmnl_node );
		}
	}

	/*发送主机状态*/
	terminal_main_state_send( 0, NULL, 0 );

	return 0;
}

int terminal_speak_limit_num_set( uint16_t cmd, void *data, uint32_t data_len )// 处理函数有待完善(11/4)
{
	FILE* fd = NULL;
	thost_system_set set_sys; // 系统配置文件的格式
	
	fd = Fopen( STSTEM_SET_STUTUS_PROFILE, "rb" ); // 只读读出数据
	if( NULL == fd )
	{
		DEBUG_INFO( "mian state send ->open files %s Err!",  STSTEM_SET_STUTUS_PROFILE );
		return -1;
	}
	if( profile_system_file_read( fd, &set_sys ) == -1)
	{
		DEBUG_INFO( "Read profile system Err!" );
		Fclose( fd );
		return -1;
	}

	uint8_t spk_limt_num = *((uint8_t*)data);
	//uint8_t dis_mode = set_sys.discuss_mode;

	gdisc_flags.limit_num = spk_limt_num;

	Fclose( fd );
	return 0;
}

int terminal_apply_limit_num_set( uint16_t cmd, void *data, uint32_t data_len )
{
	FILE* fd = NULL;
	thost_system_set set_sys; // 系统配置文件的格式
	
	fd = Fopen( STSTEM_SET_STUTUS_PROFILE, "rb" ); // 只读读出数据
	if( NULL == fd )
	{
		DEBUG_INFO( "mian state send ->open files %s Err!",  STSTEM_SET_STUTUS_PROFILE );
		return -1;
	}
	if( profile_system_file_read( fd, &set_sys ) == -1)
	{
		DEBUG_INFO( "Read profile system Err!" );
		Fclose( fd );
		return -1;
	}

	uint8_t apply_limt_num = *((uint8_t*)data);
	gdisc_flags.apply_limit = apply_limt_num;

	Fclose( fd );
	return 0;
}

/* 终端发言限时的控制,由于终端不能控制麦克风的关闭与打开，
所以系统不仅仅是会议协议数据的发送，还需管理终端的发言时长;而发言时长由系统的超时机制进行管理,
因此只需在终端连接时设置连接表相应的超时时间即可*/
int terminal_limit_speak_time_set( uint16_t cmd, void *data, uint32_t data_len )
{
	FILE* fd = NULL;
	thost_system_set set_sys; // 系统配置文件的格式
	
	fd = Fopen( STSTEM_SET_STUTUS_PROFILE, "rb" ); // 只读读出数据
	if( NULL == fd )
	{
		DEBUG_INFO( "mian state send ->open files %s Err!",  STSTEM_SET_STUTUS_PROFILE );
		return -1;
	}
	if( profile_system_file_read( fd, &set_sys ) == -1)
	{
		DEBUG_INFO( "Read profile system Err!" );
		Fclose( fd );
		return -1;
	}

	tmnl_limit_spk_time spk_time;
	speak_limit_time = spk_time.limit_time = set_sys.spk_limtime;

	if( !set_sys.spk_limtime ) // 无限时
	{
		terminal_limit_spk_time( 0, BRDCST_ALL, spk_time );
	}
	else
	{
		uint16_t nolimit_addr = 0;
		uint16_t limit_addr = 0;
		tmnl_limit_spk_time nolimit_spk_time;
		
		if( set_sys.vip_limitime ) // vip 限时
		{
			limit_addr |= BRDCST_VIP;
		}
		else
		{
			nolimit_addr |= BRDCST_CHM |BRDCST_EXE;
		}
		
		if( set_sys.chman_limitime ) // 主席限时
		{
			limit_addr |= BRDCST_CHM |BRDCST_EXE;
		}
		else
		{
			nolimit_addr |= BRDCST_VIP;
		}

		if( nolimit_addr )
		{
			DEBUG_LINE();
			nolimit_spk_time.limit_time = 0;
			terminal_limit_spk_time( BRDCST_1722_ALL, nolimit_addr, nolimit_spk_time );
		}

		terminal_limit_spk_time( BRDCST_1722_ALL, nolimit_addr, spk_time );
	}

	Fclose( fd );
	
	return 0;
}

/*==================================================
					结束终端命令函数
====================================================*/


/*===================================================
{@终端处理流程
=====================================================*/
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
	DEBUG_INFO( "mic state = %d ",  mic_status );
	if( is_report_cmpt && (mic_status != MIC_CHM_INTERPOSE_STATUS) && tmnl_node != NULL)
	{
		tmnl_node->tmnl_dev.tmnl_status.mic_state = mic_status;
		upper_cmpt_report_mic_state( mic_status, tmnl_node->tmnl_dev.address.addr );
	}

	terminal_set_mic_status( mic_status, addr, tarker_id );
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
	uint8_t system_state = get_sys_state();
	tmnl_pdblist p_node = dev_terminal_list_guard->next;	
	
	if(  system_state != DISCUSS_STATE )
	{
		return -1;
	}
	
	if( p_node == dev_terminal_list_guard )// 无终端
	{
		return -1;
	}
	
	if( speak_limit_time == 0 ) // 不限时
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
	FILE* fd = NULL;
	thost_system_set set_sys; // 系统配置文件的格式
	
	fd = Fopen( STSTEM_SET_STUTUS_PROFILE, "rb" ); // 只读读出数据
	if( NULL == fd )
	{
		DEBUG_INFO( "mian state send ->open files %s Err!",  STSTEM_SET_STUTUS_PROFILE );
		return -1;
	}
	if( profile_system_file_read( fd, &set_sys ) == -1)
	{
		DEBUG_INFO( "Read profile system Err!" );
		Fclose( fd );
		return -1;
	}

	if( !mic_flag ) // 关闭所有麦克风
	{
		tmnl_pdblist  tmnl_node = dev_terminal_list_guard->next;
		
		/*关闭所有麦克风*/
		for( ; tmnl_node != dev_terminal_list_guard; tmnl_node = tmnl_node->next )
		{
			connect_table_tarker_disconnect( tmnl_node->tmnl_dev.entity_id );

			if( tmnl_node->tmnl_dev.tmnl_status.is_rgst )
			{
				terminal_mic_state_set( MIC_COLSE_STATUS, tmnl_node->tmnl_dev.address.addr, tmnl_node->tmnl_dev.entity_id,true, tmnl_node );
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
		gtmnl_state_opt[i].sys = DISCUSS_STATE; // 系统模式
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
	
	Fclose( fd );
	return 0;
}

/*依据类型设置终端的状态*/
void terminal_state_set_base_type( uint16_t addr, tmnl_state_set tmnl_state )
{
	terminal_state_set( tmnl_state, addr, BRDCST_1722_ALL );
}

/*主席申请类型终端状态设置*/
void terminal_chairman_apply_type_set( uint16_t addr )
{
	gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE].keydown = 0x0e; // 2 3 4键按下有效
	gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE].keyup = 0;
	gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE].sys = DISCUSS_STATE;

	terminal_state_set_base_type( addr, gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE] );
	terminal_lcd_display_num_send( BRDCST_MEM |BRDCST_VIP|BRDCST_CHM|BRDCST_EXE,\
			LCD_OPTION_DISPLAY, CHM_APPROVE_APPLY_INTERFACE );// 发送lcd显示屏号
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
	terminal_set_indicator_lamp( led_lamp, addr, BRDCST_1722_ALL );
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
/*===================================================
end reallot address
=====================================================*/

