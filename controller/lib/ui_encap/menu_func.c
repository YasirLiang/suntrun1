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
#include "profile_system.h"
#include "system_database.h"

// 非命令行菜单功能命令
static void menuModeSetPro( uint16_t value, uint8_t  *p_GetParam )
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "ModeSet flags  = %d", temp );

	if( temp == 0 )// free mode
		return;

	// 保存到数据库中
	gset_sys.discuss_mode = temp;
	system_db_update_configure_system_table( gset_sys );

	find_func_command_link( MENU_USE, MENU_DISC_MODE_SET_CMD, 0, &temp, sizeof(uint8_t));
}

static void menuTempClosePro( uint16_t value, uint8_t  *p_GetParam )
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "TempClose flags  = %d", temp );

	// 保存到数据库中
	gset_sys.temp_close = temp?1:0;
	system_db_update_configure_system_table( gset_sys );

	find_func_command_link( MENU_USE, MENU_TEMP_CLOSE_SET, 0, &temp, sizeof(uint8_t));
}

static void menuChairmanHintPro( uint16_t value, uint8_t  *p_GetParam )
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "ChairmanHint flags  = %d", temp );

	// 保存到数据库中
	gset_sys.chman_music = temp?1:0;
	system_db_update_configure_system_table( gset_sys );

	find_func_command_link( MENU_USE, MENU_MUSIC_EN_SET, 0, &temp, sizeof(uint8_t));
}

static void menuCameraTrackPro( uint16_t value, uint8_t  *p_GetParam )
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "CameraTrack flags  = %d", temp );

	// 保存到数据库中
	gset_sys.camara_track = temp?1:0;
	system_db_update_configure_system_table( gset_sys );

	find_func_command_link( MENU_USE, MENU_CMR_TRACK, 0, &temp, sizeof(uint8_t));
}

static void menuAutoClosePro( uint16_t value, uint8_t  *p_GetParam )
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "AutoClose flags  = %d", temp );

	// 保存到数据库中
	gset_sys.auto_close = temp?1:0;
	system_db_update_configure_system_table( gset_sys );

	find_func_command_link( MENU_USE, MENU_AUTO_CLOSE_CMD, 0, &temp, sizeof(uint16_t));
}

static void menuSpeakLimitPro( uint16_t value, uint8_t  *p_GetParam )
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "SpeakLimit flags  = %d", temp );

	// 保存到数据库中
	if( temp <= MAX_LIMIT_SPK_NUM ) 
	{
		temp = MAX_LIMIT_SPK_NUM;	
	}

	gset_sys.speak_limit = temp;
	system_db_update_configure_system_table( gset_sys );

	find_func_command_link( MENU_USE, MENU_SPK_LIMIT_NUM_SET, 0, &temp, sizeof(uint8_t));

}

static void menuApplyNumSetPro( uint16_t value, uint8_t  *p_GetParam )
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "ApplyNum flags  = %d", temp );

	// 保存到数据库中
	if( temp > MAX_LIMIT_APPLY_NUM )
	{
		temp = MAX_LIMIT_APPLY_NUM;	
	}

	gset_sys.apply_limit = temp;
	system_db_update_configure_system_table( gset_sys );

	find_func_command_link( MENU_USE, MUNU_APPLY_LIMIT_NUM_SET, 0, &temp, sizeof(uint8_t));
}

static void menuReAllotPro( uint16_t value, uint8_t  *p_GetParam )
{
	terminal_system_reallot_addr();
}

static void menuNewAllotPro( uint16_t value, uint8_t  *p_GetParam )
{
	terminal_new_endstation_allot_address( 0 );
}

static void menuSetFinishPro( uint16_t value, uint8_t  *p_GetParam )// reboot the program
{
	uint16_t temp = value;
	DEBUG_INFO( "SetFinish flags  = %d", temp );

}

static void menuCameraCtlLeftRightPro( uint16_t value, uint8_t  *p_GetParam ) // 0 left 1 right
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "CameraCtlLeftRight flags  = %d", temp );

	find_func_command_link( MENU_USE, MENU_CMR_CTRL_LR, 0, &temp, sizeof(uint8_t));
}

static void menuCameraCtlUpDownPro( uint16_t value, uint8_t  *p_GetParam )// 0 down 1 up
{
	uint8_t temp = (uint8_t)value;	
	DEBUG_INFO( "CameraCtlUpDown flags  = %d", temp );

	find_func_command_link( MENU_USE, MENU_CMR_CTRL_UD, 0, &temp, sizeof(uint8_t));
}

static void menuCameraCtlFoucePro( uint16_t value, uint8_t  *p_GetParam )
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "CameraCtlFouce flags  = %d", temp );
	
	find_func_command_link( MENU_USE, MENU_CMR_CTRL_FOUCE, 0, &temp, sizeof(uint8_t));
}

static void menuCameraCtlIrisPro( uint16_t value, uint8_t  *p_GetParam )
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "CameraCtlIris flags  = %d", temp );

	find_func_command_link( MENU_USE, MENU_CMR_CTRL_APERT, 0, &temp, sizeof(uint8_t));
}

static void menuCameraCtlZoomPro( uint16_t value, uint8_t  *p_GetParam )
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "CameraCtlZoom flags  = %d", temp );

	find_func_command_link( MENU_USE, MENU_CMR_CTRL_ZOOM, 0, &temp, sizeof(uint8_t));
}

static void menuDistanceCtlPro( uint16_t value, uint8_t  *p_GetParam )
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "DistanceCtl flags  = %d", temp );

	find_func_command_link( MENU_USE, MENU_CMR_CTRL_ALIGN, 0, &temp, sizeof(uint8_t));
}

static void menuPresetSavePro( uint16_t value, uint8_t  *p_GetParam )
{
	find_func_command_link( MENU_USE, MENU_CMR_SAVE_PRESET, 0, NULL, 0 );
}

static void menuSwitchCmrPro( uint16_t value, uint8_t  *p_GetParam )
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "SwitchCmr Num = %d", temp );

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

static void menuClearPresetPro( uint16_t value, uint8_t  *p_GetParam )
{
	uint16_t temp = value;
	DEBUG_INFO( "ClearPreset flags  = %d", temp );

	find_func_command_link( MENU_USE, MENU_CMR_CLR_PRESET, 0, NULL, 0 );
}

static void menuSelectPresetAddrPro( uint16_t value, uint8_t  *p_GetParam )
{
	uint16_t addr = value;
	DEBUG_INFO( "SelectPresetAddr  = %d", addr );

	find_func_command_link( SYSTEM_USE, SYS_PRESET_ADDR, 0, (uint8_t*)&addr, sizeof(uint16_t));
}

static void menuEnterEscPresetPro( uint16_t value, uint8_t  *p_GetParam )
{
	value?camera_pro_enter_preset():camera_pro_esc_preset();
}

extern thost_system_set gset_sys; // 系统配置文件的格式
static void menuGetMeetParamPro( uint16_t value, uint8_t  *p_GetParam )
{
	thost_system_set set_sys; // 系统配置文件的格式
	
	assert( NULL != p_GetParam );
	if( p_GetParam == NULL )
		return;
	
	if ( -1 == system_db_queue_configure_system_table( &set_sys ))
	{// load from memory(system.dat)
		memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));
	}

	if( value == VAL_TEMP_CLOSE )
		*p_GetParam = set_sys.temp_close;
	else if( value == VAL_CHM_MUSIC )
		*p_GetParam = set_sys.chman_music;
	else if( value == VAL_CMR_TRACK_EN )
		*p_GetParam = set_sys.camara_track;
	else if( value == VAL_AUTO_CLOSE )
		*p_GetParam = set_sys.auto_close;
	else if( value == VAL_CUR_CMR )
		*p_GetParam = set_sys.current_cmr;
	else if( value == VAL_SPKER_LIMIT )
		*p_GetParam = set_sys.speak_limit ;
	else if( value == VAL_APPLY_LIMIT )
		*p_GetParam = set_sys.apply_limit;
	else if( value == VAL_DSCS_MODE )
		*p_GetParam = set_sys.discuss_mode;
	else
		DEBUG_INFO( "Error EPar value!" );

}

struct _type_menu_command_command
{
	enum enum_menu_cmd menu_cmd;
	void (*menu_run_commmand)( uint16_t value, uint8_t  *p_GetParam );
};

static struct _type_menu_command_command gtable_menu_command[] = 
{
	{ MENU_UI_MODESET, menuModeSetPro },
	{ MENU_UI_TEMPCLOSE, menuTempClosePro },
	{ MENU_UI_CHAIRMANHINT, menuChairmanHintPro },
	{ MENU_UI_CAMERATRACK, menuCameraTrackPro },
	{ MENU_UI_AUTOCLOSE, menuAutoClosePro },
	{ MENU_UI_SPEAKLIMIT, menuSpeakLimitPro },
	{ MENU_UI_APPLYNUMSET, menuApplyNumSetPro },
	{ MENU_UI_REALLOT, menuReAllotPro },
	{ MENU_UI_NEWALLOT, menuNewAllotPro },
	{ MENU_UI_SETFINISH, menuSetFinishPro },
	{ MENU_UI_CAMERACTLLEFTRIGHT, menuCameraCtlLeftRightPro },
	{ MENU_UI_CAMERACTLUPDOWN, menuCameraCtlUpDownPro },
	{ MENU_UI_CAMERACTLFOUCE, menuCameraCtlFoucePro },
	{ MENU_UI_CAMERACTLIRIS, menuCameraCtlIrisPro },
	{ MENU_UI_CAMERACTLZOOM, menuCameraCtlZoomPro },
	{ MENU_UI_DISTANCECTL, menuDistanceCtlPro },
	{ MENU_UI_PRESETSAVE, menuPresetSavePro },
	{ MENU_UI_SWITCHCMR, menuSwitchCmrPro },
	{ MENU_UI_CLEARPRESET,  menuClearPresetPro },
	{ MENU_UI_SELECTPRESETADDR, menuSelectPresetAddrPro },
	{ MENU_UI_ENTERESCPRESET, menuEnterEscPresetPro },
	{MENU_UI_GET_PARAM, menuGetMeetParamPro },
	{ MENU_UI_MENU_CMD_ERR, NULL }
};

/************************************************************/
// Author:YasirLiang
// Data: 2016-3-18
// Name: menu_cmd_run
// Param:
//		menu_cmd:菜单的功能的命令
//		value:菜单产生的值
//return Value: None
/*************************************************************/
void menu_cmd_run( enum enum_menu_cmd menu_cmd, uint16_t value, uint8_t  *p_GetParam )
{	
	struct _type_menu_command_command *p = &gtable_menu_command[0];

	while( p->menu_cmd != MENU_UI_MENU_CMD_ERR )
	{
		if( menu_cmd ==  p->menu_cmd )
		{
			if( p->menu_run_commmand != NULL )
				p->menu_run_commmand( value, p_GetParam );
			
			return;
		}

		p++;
	}
}

