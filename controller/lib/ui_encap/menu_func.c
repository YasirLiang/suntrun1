/*
**file:menu_func.c
**data:2016-3-17
**
**
*/

// ********************
// 添加界面菜单功能函数模块
// 以便其他模块的调用
// 
//*******************

#include "menu_func.h"
#include "data.h"
#include "host_controller_debug.h"
#include "terminal_system_func.h"
#include "terminal_command.h"
#include "camera_pro.h"

static void menuClihelpPro( char (*args)[CMD_OPTION_STRING_LEN] )
{
	MSGINFO( "\nEnter follow String command to Test menu Function (\"quit\" to exit control)\n" );
	MSGINFO( "ModeSet: 模式设置(1:ppt 2:limit 3:fifo 4:apply)\n" );
	MSGINFO( "TempClose: Usage: TempClose 0;TempClose 1\n" );
	MSGINFO( "ChairmanHint: Usage: ChairmanHint 0;ChairmanHint 1\n" );
	MSGINFO( "CameraTrack: Usage: CameraTrack 0;CameraTrack 1\n" );
	MSGINFO( "AutoClose: Usage: AutoClose 0;AutoClose 1\n" );
	MSGINFO( "SpeakLimit: Usage: SpeakLimit 1\\2\\3\\4\\5\\6\n" );
	MSGINFO( "ApplyNumSet: Usage:ApplyNumSet 1\\2\\3\\4\\5\\6...\n" );
	MSGINFO( "ReAllot: 重新分配地址\n" );
	MSGINFO( "NewAllot: 新增分配\n" );
	MSGINFO( "SetFinish: 重启程序\n" );
	MSGINFO( "CameraCtl: Usage: CameraCtl 1(向上);2(向下);3(向左);4(向右);5(聚焦);6(光圈);7(缩放);\n" );
	MSGINFO( "CameraCtlLeftRight: 左右(0 1)\n" );
	MSGINFO( "CameraCtlUpDown: 上下(1 0)\n" );
	MSGINFO( "CameraCtlFouce: 聚焦\n" );
	MSGINFO( "CameraCtlIris: 光圈\n" );
	MSGINFO( "CameraCtlZoom: 缩放\n" );
	MSGINFO( "DistanceCtl: 遥控器对码:\n" );
	MSGINFO( "PresetSave: 预置位存储\n" );
	MSGINFO( "SwitchCmr: 切换摄像头 (SwitchCmr 1\\2\\3\\4)\n" );
	MSGINFO( "ClearPreset: 清除预置位\n" );
	MSGINFO( "SelectPresetAddr: 预置位终端选择\n" );
	MSGINFO( "EnterEscPreset: 在进行摄像头控制时必须设置 0:退出 1:进入\n" );
	MSGINFO( "help: 帮助菜单\n" );
	MSGINFO( "quit: 退出菜单控制,返回主菜单\n" );
}

static void menuCliModeSetPro( char (*args)[CMD_OPTION_STRING_LEN] )
{
	assert( NULL != args );
	if( NULL == args )
		return;
	
	uint8_t temp = (uint8_t)atoi( args[1] );
	DEBUG_INFO( "mode_value = %d", temp );

	find_func_command_link( MENU_USE, MENU_DISC_MODE_SET_CMD, 0, &temp, sizeof(uint8_t));
}

static void menuCliTempClosePro( char (*args)[CMD_OPTION_STRING_LEN] )
{
	assert( NULL != args );
	if( NULL == args )
		return;
	
	uint8_t temp = (uint8_t)atoi( args[1] );
	DEBUG_INFO( "temp close flags = %d", temp );

	find_func_command_link( MENU_USE, MENU_TEMP_CLOSE_SET, 0, &temp, sizeof(uint8_t));
}

static void menuCliChairmanHintPro( char (*args)[CMD_OPTION_STRING_LEN] )// 主席提示
{
	assert( NULL != args );
	if( NULL == args )
		return;
	
	uint8_t temp = (uint8_t)atoi( args[1] );
	DEBUG_INFO( "ChairmanHint flags  = %d", temp );

	find_func_command_link( MENU_USE, MENU_MUSIC_EN_SET, 0, &temp, sizeof(uint8_t));
}

static void menuCliCameraTrackPro( char (*args)[CMD_OPTION_STRING_LEN] )
{
	assert( NULL != args );
	if( NULL == args )
		return;
	
	uint8_t temp = (uint8_t)atoi( args[1] );
	DEBUG_INFO( "CameraTrack flags  = %d", temp );

	find_func_command_link( MENU_USE, MENU_CMR_TRACK, 0, &temp, sizeof(uint8_t));
}

static void menuCliAutoClosePro( char (*args)[CMD_OPTION_STRING_LEN] )
{
	assert( NULL != args );
	if( NULL == args )
		return;
	
	uint8_t temp = (uint8_t)atoi( args[1] );
	DEBUG_INFO( "AutoClose flags  = %d", temp );

	find_func_command_link( MENU_USE, MENU_AUTO_CLOSE_CMD, 0, &temp, sizeof(uint8_t));
}

static void menuCliSpeakLimitPro( char (*args)[CMD_OPTION_STRING_LEN] )
{
	assert( NULL != args );
	if( NULL == args )
		return;
	
	uint8_t temp = (uint8_t)atoi( args[1] );
	DEBUG_INFO( "SpeakLimit  Num = %d", temp );

	find_func_command_link( MENU_USE, MENU_SPK_LIMIT_NUM_SET, 0, &temp, sizeof(uint8_t));
}

static void menuCliApplyNumSetPro( char (*args)[CMD_OPTION_STRING_LEN] )
{
	assert( NULL != args );
	if( NULL == args )
		return;
	
	uint8_t temp = (uint8_t)atoi( args[1] );
	DEBUG_INFO( "SpeakLimit  Num  = %d", temp );

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

// 非命令行菜单功能命令
static void menuModeSetPro( uint16_t value )
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "ModeSet flags  = %d", temp );

	if( temp == 0 )// free mode
		return;

	find_func_command_link( MENU_USE, MENU_DISC_MODE_SET_CMD, 0, &temp, sizeof(uint8_t));
}

static void menuTempClosePro( uint16_t value )
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "TempClose flags  = %d", temp );

	find_func_command_link( MENU_USE, MENU_TEMP_CLOSE_SET, 0, &temp, sizeof(uint8_t));
}

static void menuChairmanHintPro( uint16_t value )
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "ChairmanHint flags  = %d", temp );

	find_func_command_link( MENU_USE, MENU_MUSIC_EN_SET, 0, &temp, sizeof(uint8_t));
}

static void menuCameraTrackPro( uint16_t value )
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "CameraTrack flags  = %d", temp );

	find_func_command_link( MENU_USE, MENU_CMR_TRACK, 0, &temp, sizeof(uint8_t));
}

static void menuAutoClosePro( uint16_t value )
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "AutoClose flags  = %d", temp );

	find_func_command_link( MENU_USE, MENU_AUTO_CLOSE_CMD, 0, &temp, sizeof(uint16_t));
}

static void menuSpeakLimitPro( uint16_t value )
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "SpeakLimit flags  = %d", temp );

	find_func_command_link( MENU_USE, MENU_SPK_LIMIT_NUM_SET, 0, &temp, sizeof(uint8_t));
}

static void menuApplyNumSetPro( uint16_t value )
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "ApplyNum flags  = %d", temp );

	find_func_command_link( MENU_USE, MUNU_APPLY_LIMIT_NUM_SET, 0, &temp, sizeof(uint8_t));
}

static void menuReAllotPro( uint16_t value )
{
	terminal_system_reallot_addr();
}

static void menuNewAllotPro( uint16_t value )
{
	terminal_new_endstation_allot_address( 0 );
}

static void menuSetFinishPro( uint16_t value )// reboot the program
{
	uint16_t temp = value;
	DEBUG_INFO( "SetFinish flags  = %d", temp );

}

static void menuCameraCtlLeftRightPro( uint16_t value ) // 0 left 1 right
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "CameraCtlLeftRight flags  = %d", temp );

	find_func_command_link( MENU_USE, MENU_CMR_CTRL_LR, 0, &temp, sizeof(uint8_t));
}

static void menuCameraCtlUpDownPro( uint16_t value )// 0 down 1 up
{
	uint8_t temp = (uint8_t)value;	
	DEBUG_INFO( "CameraCtlUpDown flags  = %d", temp );

	find_func_command_link( MENU_USE, MENU_CMR_CTRL_UD, 0, &temp, sizeof(uint8_t));
}

static void menuCameraCtlFoucePro( uint16_t value )
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "CameraCtlFouce flags  = %d", temp );
	
	find_func_command_link( MENU_USE, MENU_CMR_CTRL_FOUCE, 0, &temp, sizeof(uint8_t));
}

static void menuCameraCtlIrisPro( uint16_t value )
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "CameraCtlIris flags  = %d", temp );

	find_func_command_link( MENU_USE, MENU_CMR_CTRL_APERT, 0, &temp, sizeof(uint8_t));
}

static void menuCameraCtlZoomPro( uint16_t value )
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "CameraCtlZoom flags  = %d", temp );

	find_func_command_link( MENU_USE, MENU_CMR_CTRL_ZOOM, 0, &temp, sizeof(uint8_t));
}

static void menuDistanceCtlPro( uint16_t value )
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "DistanceCtl flags  = %d", temp );

	find_func_command_link( MENU_USE, MENU_CMR_CTRL_ALIGN, 0, &temp, sizeof(uint8_t));
}

static void menuPresetSavePro( uint16_t value )
{
	find_func_command_link( MENU_USE, MENU_CMR_SAVE_PRESET, 0, NULL, 0 );
}

static void menuSwitchCmrPro( uint16_t value )
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "SwitchCmr Num = %d", temp );

	find_func_command_link( MENU_USE, MENU_CMR_SEL_CMR, 0, &temp, sizeof(uint8_t));
}

static void menuClearPresetPro( uint16_t value )
{
	uint16_t temp = value;
	DEBUG_INFO( "ClearPreset flags  = %d", temp );

	find_func_command_link( MENU_USE, MENU_CMR_CLR_PRESET, 0, NULL, 0 );
}

static void menuSelectPresetAddrPro( uint16_t value )
{
	uint16_t addr = value;
	DEBUG_INFO( "SelectPresetAddr  = %d", addr );

	find_func_command_link( SYSTEM_USE, SYS_PRESET_ADDR, 0, (uint8_t*)&addr, sizeof(uint16_t));
}

static void menuEnterEscPresetPro( uint16_t value )
{
	value?camera_pro_enter_preset():camera_pro_esc_preset();
}

struct _type_menu_command_line_command
{
	char *cmd_string;
	enum enum_menu_cmd menu_cmd;
	void (*menu_run_cli)(char (*args)[CMD_OPTION_STRING_LEN]);
	void (*menu_run_commmand)( uint16_t );
};

static struct _type_menu_command_line_command gtable_menu_command_line[] = 
{
	{ "ModeSet", MENU_UI_MODESET, menuCliModeSetPro, menuModeSetPro },
	{ "TempClose", MENU_UI_TEMPCLOSE, menuCliTempClosePro, menuTempClosePro },
	{ "ChairmanHint", MENU_UI_CHAIRMANHINT, menuCliChairmanHintPro, menuChairmanHintPro },
	{ "CameraTrack", MENU_UI_CAMERATRACK, menuCliCameraTrackPro, menuCameraTrackPro },
	{ "AutoClose", MENU_UI_AUTOCLOSE, menuCliAutoClosePro, menuAutoClosePro },
	{ "SpeakLimit", MENU_UI_SPEAKLIMIT, menuCliSpeakLimitPro, menuSpeakLimitPro },
	{ "ApplyNumSet", MENU_UI_APPLYNUMSET, menuCliApplyNumSetPro, menuApplyNumSetPro },
	{ "ReAllot", MENU_UI_REALLOT, menuCliReAllotPro, menuReAllotPro },
	{ "NewAllot", MENU_UI_NEWALLOT,menuCliNewAllotPro, menuNewAllotPro },
	{ "SetFinish", MENU_UI_SETFINISH, menuCliSetFinishPro, menuSetFinishPro },
	{ "CameraCtl", MENU_UI_CAMERACTL, menuCliCameraCtlPro, NULL },
	{ "CameraCtlLeftRight", MENU_UI_CAMERACTLLEFTRIGHT, menuCliCameraCtlLeftRightPro, menuCameraCtlLeftRightPro },
	{ "CameraCtlUpDown", MENU_UI_CAMERACTLUPDOWN, menuCliCameraCtlUpDownPro, menuCameraCtlUpDownPro },
	{ "CameraCtlFouce", MENU_UI_CAMERACTLFOUCE, menuCliCameraCtlFoucePro, menuCameraCtlFoucePro },
	{ "CameraCtlIris", MENU_UI_CAMERACTLIRIS, menuCliCameraCtlIrisPro, menuCameraCtlIrisPro },
	{ "CameraCtlZoom", MENU_UI_CAMERACTLZOOM, menuCliCameraCtlZoomPro, menuCameraCtlZoomPro },
	{ "DistanceCtl", MENU_UI_DISTANCECTL, menuCliDistanceCtlPro, menuDistanceCtlPro },
	{ "PresetSave", MENU_UI_PRESETSAVE, menuCliPresetSavePro, menuPresetSavePro },
	{ "SwitchCmr", MENU_UI_SWITCHCMR, menuCliSwitchCmrPro, menuSwitchCmrPro },
	{ "ClearPreset", MENU_UI_CLEARPRESET, menuCliClearPresetPro,  menuClearPresetPro },
	{ "SelectPresetAddr", MENU_UI_SELECTPRESETADDR, menuCliSelectPresetAddrPro, menuSelectPresetAddrPro },
	{ "EnterEscPreset", MENU_UI_ENTERESCPRESET, menuCliEnterEscPresetPro, menuEnterEscPresetPro },
	{ "help", MENU_UI_MENU_CMD_HELP, menuClihelpPro, NULL },
	{ "NULL", MENU_UI_MENU_CMD_ERR, NULL, NULL }
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

	while( p->menu_cmd != MENU_UI_MENU_CMD_ERR )
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

/************************************************************/
// Author:YasirLiang
// Data: 2016-3-18
// Name: menu_cmd_run
// Param:
//		menu_cmd:菜单的功能的命令
//		value:菜单产生的值
//return Value: None
/*************************************************************/
void menu_cmd_run( enum enum_menu_cmd menu_cmd, uint16_t value )
{	
	struct _type_menu_command_line_command *p = &gtable_menu_command_line[0];

	while( p->menu_cmd != MENU_UI_MENU_CMD_ERR )
	{
		if( menu_cmd ==  p->menu_cmd )
		{
			if( p->menu_run_commmand != NULL )
				p->menu_run_commmand( value );
			
			return;
		}

		p++;
	}
}

