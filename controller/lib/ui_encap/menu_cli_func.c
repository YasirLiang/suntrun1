/*
**file:menu_func.c
**data:2016-3-17
**
**
*/

// ********************
// 添加界面菜单功能函数的命令行模块
// 以便其他模块的调用
// 
//*******************

#include "menu_cli_func.h"
#include "data.h"
#include "host_controller_debug.h"
#include "terminal_system_func.h"
#include "terminal_command.h"
#include "camera_pro.h"
#include "system_database.h"

static void menuClihelpPro( char (*args)[CMD_OPTION_STRING_LEN] )
{
	MSGINFO( "\nEnter follow String command to Test menu Function (\"quit\" to exit control)\n" );
	MSGINFO( "ModeSet: 模式设置(1:ppt 2:limit 3:fifo 4:apply)" );
	MSGINFO( "TempClose: Usage: TempClose 0;TempClose 1" );
	MSGINFO( "ChairmanHint: Usage: ChairmanHint 0;ChairmanHint 1" );
	MSGINFO( "CameraTrack: Usage: CameraTrack 0;CameraTrack 1" );
	MSGINFO( "AutoClose: Usage: AutoClose 0;AutoClose 1" );
	MSGINFO( "SpeakLimit: Usage: SpeakLimit 1\\2\\3\\4\\5\\6" );
	MSGINFO( "ApplyNumSet: Usage:ApplyNumSet 1\\2\\3\\4\\5\\6..." );
	MSGINFO( "ReAllot: 重新分配地址" );
	MSGINFO( "NewAllot: 新增分配" );
	MSGINFO( "SetFinish: 重启程序" );
	MSGINFO( "CameraCtl: Usage: CameraCtl 1(向上);2(向下);3(向左);4(向右);5(聚焦);6(光圈);7(缩放);" );
	MSGINFO( "CameraCtlLeftRight: 左右(0 1)" );
	MSGINFO( "CameraCtlUpDown: 上下(1 0)" );
	MSGINFO( "CameraCtlFouce: 聚焦" );
	MSGINFO( "CameraCtlIris: 光圈" );
	MSGINFO( "CameraCtlZoom: 缩放" );
	MSGINFO( "DistanceCtl: 遥控器对码:" );
	MSGINFO( "PresetSave: 预置位存储" );
	MSGINFO( "SwitchCmr: 切换摄像头 (SwitchCmr 1\\2\\3\\4)" );
	MSGINFO( "ClearPreset: 清除预置位" );
	MSGINFO( "SelectPresetAddr: 预置位终端选择" );
	MSGINFO( "EnterEscPreset: 在进行摄像头控制时必须设置 0:退出 1:进入" );
	MSGINFO( "help: 帮助菜单" );
	MSGINFO( "quit: 退出菜单控制,返回主菜单" );
}

static void menuCliModeSetPro( char (*args)[CMD_OPTION_STRING_LEN] )
{
	assert( NULL != args );
	if( NULL == args )
		return;
	
	uint8_t temp = (uint8_t)atoi( args[1] );
	DEBUG_INFO( "mode_value = %d", temp );

	// 保存到数据库中
	gset_sys.discuss_mode = temp;
	system_db_update_configure_system_table( gset_sys );

	find_func_command_link( MENU_USE, MENU_DISC_MODE_SET_CMD, 0, &temp, sizeof(uint8_t));
}

static void menuCliTempClosePro( char (*args)[CMD_OPTION_STRING_LEN] )
{
	assert( NULL != args );
	if( NULL == args )
		return;
	
	uint8_t temp = (uint8_t)atoi( args[1] );
	DEBUG_INFO( "temp close flags = %d", temp );

	// 保存到数据库中
	gset_sys.temp_close = temp;
	system_db_update_configure_system_table( gset_sys );
	
	find_func_command_link( MENU_USE, MENU_TEMP_CLOSE_SET, 0, &temp, sizeof(uint8_t));
}

static void menuCliChairmanHintPro( char (*args)[CMD_OPTION_STRING_LEN] )// 主席提示
{
	assert( NULL != args );
	if( NULL == args )
		return;
	
	uint8_t temp = (uint8_t)atoi( args[1] );
	DEBUG_INFO( "ChairmanHint flags  = %d", temp );

	// 保存到数据库中
	gset_sys.chman_music = temp;
	system_db_update_configure_system_table( gset_sys );
	
	find_func_command_link( MENU_USE, MENU_MUSIC_EN_SET, 0, &temp, sizeof(uint8_t));
}

static void menuCliCameraTrackPro( char (*args)[CMD_OPTION_STRING_LEN] )
{
	assert( NULL != args );
	if( NULL == args )
		return;
	
	uint8_t temp = (uint8_t)atoi( args[1] );
	DEBUG_INFO( "CameraTrack flags  = %d", temp );

	// 保存到数据库中
	gset_sys.camara_track = temp;
	system_db_update_configure_system_table( gset_sys );

	find_func_command_link( MENU_USE, MENU_CMR_TRACK, 0, &temp, sizeof(uint8_t));
}

static void menuCliAutoClosePro( char (*args)[CMD_OPTION_STRING_LEN] )
{
	assert( NULL != args );
	if( NULL == args )
		return;
	
	uint8_t temp = (uint8_t)atoi( args[1] );
	DEBUG_INFO( "AutoClose flags  = %d", temp );

	// 保存到数据库中
	gset_sys.auto_close = temp;
	system_db_update_configure_system_table( gset_sys );

	find_func_command_link( MENU_USE, MENU_AUTO_CLOSE_CMD, 0, &temp, sizeof(uint8_t));
}

static void menuCliSpeakLimitPro( char (*args)[CMD_OPTION_STRING_LEN] )
{
	assert( NULL != args );
	if( NULL == args )
		return;
	
	uint8_t temp = (uint8_t)atoi( args[1] );
	DEBUG_INFO( "SpeakLimit  Num = %d", temp );

	// 保存到数据库中
	if( temp > MAX_LIMIT_SPK_NUM ) 
	{
		temp = MAX_LIMIT_SPK_NUM;
	}

	gset_sys.speak_limit = temp;
	system_db_update_configure_system_table( gset_sys );

	find_func_command_link( MENU_USE, MENU_SPK_LIMIT_NUM_SET, 0, &temp, sizeof(uint8_t));

}

static void menuCliApplyNumSetPro( char (*args)[CMD_OPTION_STRING_LEN] )
{
	assert( NULL != args );
	if( NULL == args )
		return;
	
	uint8_t temp = (uint8_t)atoi( args[1] );
	DEBUG_INFO( "Apply limit  Num  = %d", temp );

	// 保存到数据库中
	if( temp > MAX_LIMIT_APPLY_NUM )
	{
		temp = MAX_LIMIT_APPLY_NUM;
	}

	gset_sys.apply_limit = temp;
	system_db_update_configure_system_table( gset_sys );

	find_func_command_link( MENU_USE, MUNU_APPLY_LIMIT_NUM_SET, 0, &temp, sizeof(uint8_t));

}

static void menuCliReAllotPro( char (*args)[CMD_OPTION_STRING_LEN] )
{
	terminal_system_reallot_addr();
}

static void menuCliNewAllotPro( char (*args)[CMD_OPTION_STRING_LEN] )
{
	terminal_new_endstation_allot_address( 0 );
}

static void menuCliSetFinishPro( char (*args)[CMD_OPTION_STRING_LEN] )// 重启程序
{
	
}

static void menuCliCameraCtlPro( char (*args)[CMD_OPTION_STRING_LEN] )
{
	assert( NULL != args );
	if( NULL == args )
		return;
	
	uint8_t temp = (uint8_t)atoi( args[1] );
	uint8_t temp2 = (uint8_t)atoi( args[2] ); // 光圈 聚焦 缩放 的第二参数 , 值为非零或零
	DEBUG_INFO( "CameraCtl flags = %d - %d", temp, temp2  );

	switch( temp )
	{
		case 1:
			temp = 1;
			find_func_command_link( MENU_USE, MENU_CMR_CTRL_UD, 0, &temp, sizeof(uint8_t));
			break;
		case 2:
			temp = 0;
			find_func_command_link( MENU_USE, MENU_CMR_CTRL_UD, 0, &temp, sizeof(uint8_t));
			break;
		case 3:
			temp = 0;
			find_func_command_link( MENU_USE, MENU_CMR_CTRL_LR, 0, &temp, sizeof(uint8_t));
			break;
		case 4:
			temp = 1;
			find_func_command_link( MENU_USE, MENU_CMR_CTRL_LR, 0, &temp, sizeof(uint8_t));
			break;
		case 5:
			find_func_command_link( MENU_USE, MENU_CMR_CTRL_FOUCE, 0, &temp2, sizeof(uint8_t));
			break;
		case 6:
			find_func_command_link( MENU_USE, MENU_CMR_CTRL_APERT, 0, &temp2, sizeof(uint8_t));
			break;
		case 7:
			find_func_command_link( MENU_USE, MENU_CMR_CTRL_ZOOM, 0, &temp2, sizeof(uint8_t));
			break;
		default:
			break;
	}
}

static void menuCliCameraCtlLeftRightPro( char (*args)[CMD_OPTION_STRING_LEN] ) 
{
	assert( NULL != args );
	if( NULL == args )
		return;
	
	uint8_t temp = (uint8_t)atoi( args[1] ); // 0 letf; 1 right
	DEBUG_INFO( "CameraCtlLeftRight flags = %d", temp );

	find_func_command_link( MENU_USE, MENU_CMR_CTRL_LR, 0, &temp, sizeof(uint8_t));
}

static void menuCliCameraCtlUpDownPro( char (*args)[CMD_OPTION_STRING_LEN] )
{
	assert( NULL != args );
	if( NULL == args )
		return;
	
	uint8_t temp = (uint8_t)atoi( args[1] );// 0 down; 1 up
	DEBUG_INFO( "CameraCtlUpDown  flags  = %d", temp );

	find_func_command_link( MENU_USE, MENU_CMR_CTRL_UD, 0, &temp, sizeof(uint8_t));
}

static void menuCliCameraCtlFoucePro( char (*args)[CMD_OPTION_STRING_LEN] )
{
	assert( NULL != args );
	if( NULL == args )
		return;
	
	uint8_t temp = (uint8_t)atoi( args[1] );
	DEBUG_INFO( "CameraCtlFouce  flags  = %d", temp );

	find_func_command_link( MENU_USE, MENU_CMR_CTRL_FOUCE, 0, &temp, sizeof(uint8_t));
}

static void menuCliCameraCtlIrisPro( char (*args)[CMD_OPTION_STRING_LEN] )
{
	assert( NULL != args );
	if( NULL == args )
		return;
	
	uint8_t temp = (uint8_t)atoi( args[1] );
	DEBUG_INFO( "CameraCtlIris  flags  = %d", temp );
	
	find_func_command_link( MENU_USE, MENU_CMR_CTRL_APERT, 0, &temp, sizeof(uint8_t));
}

static void menuCliCameraCtlZoomPro( char (*args)[CMD_OPTION_STRING_LEN] )
{
	assert( NULL != args );
	if( NULL == args )
		return;
	
	uint8_t temp = (uint8_t)atoi( args[1] );
	DEBUG_INFO( "CameraCtlZoom flags  = %d", temp );

	find_func_command_link( MENU_USE, MENU_CMR_CTRL_ZOOM, 0, &temp, sizeof(uint8_t));
}

static void menuCliDistanceCtlPro( char (*args)[CMD_OPTION_STRING_LEN] )
{
	assert( NULL != args );
	if( NULL == args )
		return;
	
	uint8_t temp = (uint8_t)atoi( args[1] );
	DEBUG_INFO( "DistanceCtl flags = %d ", temp );
	
	find_func_command_link( MENU_USE, MENU_CMR_CTRL_ALIGN, 0, &temp, sizeof(uint8_t));
}

static void menuCliPresetSavePro( char (*args)[CMD_OPTION_STRING_LEN] ) 
{
	assert( NULL != args );
	if( NULL == args )
		return;
	
	uint8_t temp = (uint8_t)atoi( args[1] );
	DEBUG_INFO( "PresetSave flags = %d ", temp );

	find_func_command_link( MENU_USE, MENU_CMR_SAVE_PRESET, 0, &temp, sizeof(uint8_t));
}

static void menuCliSwitchCmrPro( char (*args)[CMD_OPTION_STRING_LEN] )
{
	assert( NULL != args );
	if( NULL == args )
		return;
	
	uint8_t temp = (uint8_t)atoi( args[1] );
	DEBUG_INFO( "SwitchCmr num = %d ", temp );

	// 保存到数据库中 1-4
	if( temp < 1 )
	{
		temp = 1;
	}
	else if( temp > 4 )
		temp = 4;
	
	gset_sys.current_cmr = temp;
	system_db_update_configure_system_table( gset_sys );

	find_func_command_link( MENU_USE, MENU_CMR_SEL_CMR, 0, &temp, sizeof(uint8_t));

}

static void menuCliClearPresetPro( char (*args)[CMD_OPTION_STRING_LEN] )
{
	assert( NULL != args );
	if( NULL == args )
		return;
	
	uint8_t temp = (uint8_t)atoi( args[1] );
	DEBUG_INFO( "ClearPreset flags = %d ", temp );
	
	find_func_command_link( MENU_USE, MENU_CMR_CLR_PRESET, 0, NULL, 0 );
}

static void menuCliSelectPresetAddrPro( char (*args)[CMD_OPTION_STRING_LEN] )
{
	assert( NULL != args );
	if( NULL == args )
		return;
	
	uint16_t temp = (uint16_t)atoi( args[1] );
	DEBUG_INFO( "SelectPreset flags = %d ", temp );

	find_func_command_link( SYSTEM_USE, SYS_PRESET_ADDR, 0, (uint8_t*)&temp, sizeof(uint16_t));
}

static void menuCliEnterEscPresetPro( char (*args)[CMD_OPTION_STRING_LEN] )
{
	assert( NULL != args );
	if( NULL == args )
		return;
	
	uint8_t temp = (uint8_t)atoi( args[1] );
	DEBUG_INFO( "EnterEscPreset flags = %d", temp );
	
	temp?camera_pro_enter_preset():camera_pro_esc_preset();
}

struct _type_menu_command_line_command
{
	char *cmd_string;
	void (*menu_run_cli)(char (*args)[CMD_OPTION_STRING_LEN]);
};

static struct _type_menu_command_line_command gtable_menu_command_line[] = 
{
	{ "ModeSet", menuCliModeSetPro },
	{ "TempClose",  menuCliTempClosePro },
	{ "ChairmanHint", menuCliChairmanHintPro },
	{ "CameraTrack", menuCliCameraTrackPro },
	{ "AutoClose", menuCliAutoClosePro },
	{ "SpeakLimit", menuCliSpeakLimitPro },
	{ "ApplyNumSet", menuCliApplyNumSetPro },
	{ "ReAllot", menuCliReAllotPro },
	{ "NewAllot",menuCliNewAllotPro },
	{ "SetFinish", menuCliSetFinishPro },
	{ "CameraCtl", menuCliCameraCtlPro },
	{ "CameraCtlLeftRight", menuCliCameraCtlLeftRightPro },
	{ "CameraCtlUpDown", menuCliCameraCtlUpDownPro },
	{ "CameraCtlFouce", menuCliCameraCtlFoucePro },
	{ "CameraCtlIris", menuCliCameraCtlIrisPro },
	{ "CameraCtlZoom", menuCliCameraCtlZoomPro },
	{ "DistanceCtl", menuCliDistanceCtlPro },
	{ "PresetSave", menuCliPresetSavePro },
	{ "SwitchCmr", menuCliSwitchCmrPro },
	{ "ClearPreset", menuCliClearPresetPro },
	{ "SelectPresetAddr", menuCliSelectPresetAddrPro },
	{ "EnterEscPreset", menuCliEnterEscPresetPro },
	{ "help", menuClihelpPro },
	{ "NULL", NULL }
};

/************************************************************/
// Author:YasirLiang
// Data: 2016-3-18
// Name: menu_cmd_run
// Param:
//		args:命令参数字符串，也包括命令本身
//			args[0] 命令本身
// 			args[1] 命令参数1
//			args[2] 命令参数2
// 			......
//return Value: 
//			None
/*************************************************************/
void menu_cmd_line_run( char (*args)[CMD_OPTION_STRING_LEN] )
{
	assert( NULL != args );
	if( args == NULL )
		return;
	
	struct _type_menu_command_line_command *p = &gtable_menu_command_line[0];

	while( strcmp( p->cmd_string, "NULL")  != 0 )
	{
		if( strcmp( p->cmd_string, args[0]) == 0 )
		{
			if( p->menu_run_cli != NULL )
				p->menu_run_cli( args );
			
			return;
		}

		p++;
	}
}

