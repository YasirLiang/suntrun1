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

FILE* addr_file_fd = NULL; 		// 终端地址信息读取文件描述符
terminal_address_list tmnl_addr_list[SYSTEM_TMNL_MAX_NUM];	// 终端地址分配列表
terminal_address_list_pro allot_addr_pro;	
tmnl_pdblist dev_terminal_list_guard = NULL; // 终端链表表头结点
bool reallot_flag = false; // 重新分配标志
tmnl_state_set gtmnl_state_opt[TMNL_TYPE_NUM];
tsys_discuss_pro gdisc_flags; // 系统讨论参数
tchairman_control_in gchm_int_ctl; // 主席插话
ttmnl_register_proccess gregister_tmnl_pro; // 终端报到处理


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
		DEBUG_LINE();
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
			if( temp_addr = p_gaddr_list[i].addr == 0xffff)
				break;
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
	DEBUG_LINE();
	FILE* fd = NULL;
	uint8_t auto_close = 0;
	int i = 0;
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
	
	/*关闭所有麦克风，这里需要一个机制，即通道分配机制与麦克风设置机制(这时未实现10/29)*/
	

	Fclose( fd ); // fd must be closed
	return 0;
}

/*主机发送状态*/
int terminal_main_state_send( uint16_t cmd, void *data, uint32_t data_len )
{
	assert( data && dev_terminal_list_guard );
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

	terminal_host_send_state( 0, host_main_state ); // target id is 0

	Fclose( fd );
	return 0;
}

/*终端发送显示屏号，可以继续完善11/2*/
int terminal_lcd_display_num_send( uint16_t addr, uint8_t display_opt, uint8_t display_num )
{
	tmnl_send_end_lcd_display lcd_dis;
	lcd_dis.opt = display_opt;
	lcd_dis.num = display_num;

	terminal_send_end_lcd_display( 0, addr, lcd_dis );
	
	return 0;
}

int terminal_pause_vote( uint16_t cmd, void *data, uint32_t data_len )
{
	assert( data );
	terminal_option_endpoint( 0, CONFERENCE_BROADCAST_ADDRESS, OPT_TMNL_SUSPEND_VOTE );

	return 0;
}

int terminal_regain_vote( uint16_t cmd, void *data, uint32_t data_len )
{
	assert( data );
	terminal_option_endpoint( 0, CONFERENCE_BROADCAST_ADDRESS, OPT_TMNL_RECOVER_VOTE );

	return 0;
}

int terminal_system_discuss_mode_set( uint16_t cmd, void *data, uint32_t data_len )
{
	uint8_t dis_mode = *((uint8_t*)data);
	gdisc_flags.edis_mode = (ttmnl_discuss_mode)dis_mode;
	gdisc_flags.currect_first_index = 0;
	gdisc_flags.apply_num = 0;
	gdisc_flags.speak_limit_num = 0;

	/*关闭所有麦克风*/

	/*发送主机状态*/
	terminal_main_state_send( 0, NULL, 0 );
}

/*==================================================
					结束终端命令函数
====================================================*/


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



/*===================================================
终端处理流程@}
=====================================================*/
