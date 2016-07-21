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
#include "system.h"

#include "menu_exe.h"// 来自菜单命令

extern int lcd192x64_close( void );

// 非命令行菜单功能命令
static int menuModeSetPro( uint16_t value, uint8_t  *p_GetParam )
{
	uint8_t temp = (uint8_t)value + 1;// 实际的模式是value+1
	DEBUG_INFO( "ModeSet flags  = %d", temp );

	find_func_command_link( MENUMENT_USE, MENU_DISC_MODE_SET_CMD, 0, &temp, sizeof(uint8_t));

	return 0;
}

static int menuTempClosePro( uint16_t value, uint8_t  *p_GetParam )
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "TempClose flags  = %d", temp );

	find_func_command_link( MENUMENT_USE, MENU_TEMP_CLOSE_SET, 0, &temp, sizeof(uint8_t));

	return 0;
}

static int menuChairmanHintPro( uint16_t value, uint8_t  *p_GetParam )
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "ChairmanHint flags  = %d", temp );

	find_func_command_link( MENUMENT_USE, MENU_MUSIC_EN_SET, 0, &temp, sizeof(uint8_t));

	return 0;
}

static int menuCameraTrackPro( uint16_t value, uint8_t  *p_GetParam )
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "CameraTrack flags  = %d", temp );

	find_func_command_link( MENUMENT_USE, MENU_CMR_TRACK, 0, &temp, sizeof(uint8_t));

	return 0;
}

static int menuAutoClosePro( uint16_t value, uint8_t  *p_GetParam )
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "AutoClose flags  = %d", temp );

	find_func_command_link( MENUMENT_USE, MENU_AUTO_CLOSE_CMD, 0, &temp, sizeof(uint8_t));

	return 0;
}

static int menuSpeakLimitPro( uint16_t value, uint8_t  *p_GetParam )
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "SpeakLimit flags  = %d", temp );

	find_func_command_link( MENUMENT_USE, MENU_SPK_LIMIT_NUM_SET, 0, &temp, sizeof(uint8_t));

	return 0;
}

static int menuApplyNumSetPro( uint16_t value, uint8_t  *p_GetParam )
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "ApplyNum flags  = %d", temp );

	find_func_command_link( MENUMENT_USE, MUNU_APPLY_LIMIT_NUM_SET, 0, &temp, sizeof(uint8_t));

	return 0;
}

static int menuReAllotPro( uint16_t value, uint8_t  *p_GetParam )
{
	terminal_system_reallot_addr();

	return 0;
}

static int menuNewAllotPro( uint16_t value, uint8_t  *p_GetParam )
{
	terminal_new_endstation_allot_address( 0 );

	return 0;
}

extern struct threads_info threads;
static int menuSetFinishPro( uint16_t value, uint8_t  *p_GetParam )// reboot the program
{
	uint16_t temp = value;
	DEBUG_INFO( "SetFinish flags  = %d", temp );

	//sync();// 同步所有文件
	DEBUG_INFO( "System Close......" );
#if 0
	system_close( &threads );
#else
	lcd192x64_close();// yasirLiang add in 2016/05/16
#endif
	DEBUG_INFO( "System Close Success!" );
	system("reboot");
	//exit(0);
	return 0;
}

static int menuCameraCtlLeftRightPro( uint16_t value, uint8_t  *p_GetParam ) // 0 left 1 right
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "CameraCtlLeftRight flags  = %d", temp );

	find_func_command_link( MENUMENT_USE, MENU_CMR_CTRL_LR, 0, &temp, sizeof(uint8_t));

	return 0;
}

static int menuCameraCtlUpDownPro( uint16_t value, uint8_t  *p_GetParam )// 0 down 1 up
{
	uint8_t temp = (uint8_t)value;	
	DEBUG_INFO( "CameraCtlUpDown flags  = %d", temp );

	find_func_command_link( MENUMENT_USE, MENU_CMR_CTRL_UD, 0, &temp, sizeof(uint8_t));

	return 0;
}

static int menuCameraCtlFoucePro( uint16_t value, uint8_t  *p_GetParam )
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "CameraCtlFouce flags  = %d", temp );
	
	find_func_command_link( MENUMENT_USE, MENU_CMR_CTRL_FOUCE, 0, &temp, sizeof(uint8_t));

	return 0;
}

static int menuCameraCtlIrisPro( uint16_t value, uint8_t  *p_GetParam )
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "CameraCtlIris flags  = %d", temp );

	find_func_command_link( MENUMENT_USE, MENU_CMR_CTRL_APERT, 0, &temp, sizeof(uint8_t));

	return 0;
}

static int menuCameraCtlZoomPro( uint16_t value, uint8_t  *p_GetParam )
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "CameraCtlZoom flags  = %d", temp );

	find_func_command_link( MENUMENT_USE, MENU_CMR_CTRL_ZOOM, 0, &temp, sizeof(uint8_t));

	return 0;
}

static int menuDistanceCtlPro( uint16_t value, uint8_t  *p_GetParam )
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "DistanceCtl flags  = %d", temp );

	find_func_command_link( MENUMENT_USE, MENU_CMR_CTRL_ALIGN, 0, &temp, sizeof(uint8_t));

	return 0;
}

static int menuPresetSavePro( uint16_t value, uint8_t  *p_GetParam )
{
	uint16_t temp = value;
	find_func_command_link( MENUMENT_USE, MENU_CMR_SAVE_PRESET, 0,  (uint8_t*)&temp, sizeof(uint16_t) );

	return 0;
}

static int menuSwitchCmrPro( uint16_t value, uint8_t  *p_GetParam )
{
	uint8_t temp = (uint8_t)value;
	DEBUG_INFO( "SwitchCmr Num = %d", temp );

	find_func_command_link( MENUMENT_USE, MENU_CMR_SEL_CMR, 0, &temp, sizeof(uint8_t));

	return 0;
}

static int menuClearPresetPro( uint16_t value, uint8_t  *p_GetParam )
{
	uint16_t temp = value;
	DEBUG_INFO( "ClearPreset flags  = %d", temp );

	find_func_command_link( MENUMENT_USE, MENU_CMR_CLR_PRESET, 0, (uint8_t*)&temp, sizeof(uint16_t) );

	return 0;
}

static int menuSelectPresetAddrPro( uint16_t value, uint8_t  *p_GetParam )
{
	uint16_t addr = value;
	DEBUG_INFO( "SelectPresetAddr  = %d", addr );

	find_func_command_link( SYSTEM_USE, SYS_PRESET_ADDR, 0, (uint8_t*)&addr, sizeof(uint16_t));

	return 0;
}

static int menuEnterEscPresetPro( uint16_t value, uint8_t  *p_GetParam )
{
	value?camera_pro_enter_preset():camera_pro_esc_preset();

	return 0;
}

extern thost_system_set gset_sys; // 系统配置文件的格式
static int menuGetMeetParamPro( uint16_t value, uint8_t  *p_GetParam )
{
	thost_system_set set_sys; // 系统配置文件的格式
	
	assert( NULL != p_GetParam );
	if( p_GetParam == NULL )
		return -1;
	
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
	{
		*p_GetParam = set_sys.discuss_mode-1;// 实际会讨模式的值比菜单会讨模式对应的值多1，没有自由模式
	}
	else if( value == VAL_MENU_LANG )
		*p_GetParam = set_sys.menu_language;
	else
		DEBUG_INFO( "Error EPar value!" );

	return 0;
}

static int menuSaveMeetParamPro( uint16_t value, uint8_t  *p_saveParam )
{
	uint8_t save_value;
	thost_system_set set_sys; // 系统配置文件的格式
	assert( p_saveParam != NULL );
	if( p_saveParam == NULL )
		return -1;

	if ( -1 == system_db_queue_configure_system_table( &set_sys ))
	{// load from memory(system.dat)
		memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));
	}

	save_value = *p_saveParam;
	if( value == VAL_TEMP_CLOSE )
		set_sys.temp_close = save_value?1:0;
	else if( value == VAL_CHM_MUSIC )
		set_sys.chman_music = save_value?1:0;
	else if( value == VAL_CMR_TRACK_EN )
		set_sys.camara_track = save_value?1:0;
	else if( value == VAL_AUTO_CLOSE )
		set_sys.auto_close = save_value?1:0;
	else if( value == VAL_CUR_CMR )
		set_sys.current_cmr = save_value;
	else if( value == VAL_SPKER_LIMIT )
		set_sys.speak_limit = save_value>MAX_LIMIT_SPK_NUM?MAX_LIMIT_SPK_NUM:save_value;
	else if( value == VAL_APPLY_LIMIT )
		set_sys.apply_limit = save_value>MAX_LIMIT_APPLY_NUM?MAX_LIMIT_APPLY_NUM:save_value;
	else if( value == VAL_DSCS_MODE )
	{
		set_sys.discuss_mode = save_value+1;// 实际会讨模式的值比菜单会讨模式对应的值多1，没有自由模式
	}
	else if( value == VAL_MENU_LANG )
		set_sys.menu_language  = save_value;
	else
		DEBUG_INFO( "Error EPar value!" );

	if( -1 != system_db_update_configure_system_table( set_sys ) )
	{
		memcpy( &gset_sys, &set_sys, sizeof(thost_system_set));
	}
	
	return 0;
}

static int menuSendMainStatePro( uint16_t value, uint8_t  *p_GetParam )
{
	uint16_t temp = value;
	find_func_command_link( TERMINAL_USE, HOST_SEND_STATUS, 0, (uint8_t*)&temp, 2);
	
	return 0;
}

static int menuSaveWireAddrPro( uint16_t save_vsize, uint8_t  *p_GetParam )
{
	Sdb_wireless_addr_entity wire;
	
	assert( p_GetParam != NULL );
	if( p_GetParam == NULL || save_vsize != 3 )
		return -1;

	memcpy( wire.addrlist, p_GetParam, save_vsize );
	system_db_update_wireless_addr_table( wire );

	return 0;
}

static int menuGetWireAddrPro( uint16_t get_value_size, uint8_t  *p_GetParam )
{
	Sdb_wireless_addr_entity wire;
	int ret = -1;
	
	assert( p_GetParam != NULL );
	if( p_GetParam == NULL ||(3 != get_value_size) )
		return -1;

	if( 0 == system_db_query_wireless_addr_table( &wire ) )
	{
		memcpy( p_GetParam, wire.addrlist, 3 );
		ret = 0;
	}

	return ret;
}

static int menuChnPriorPro(uint16_t value, uint8_t  *p_GetParam)
{// 临时关闭菜单键
	uint8_t temp = (uint8_t)value;

	find_func_command_link( MENUMENT_USE, MENU_PRIOR_EN_SET, 0, (uint8_t*)&temp, sizeof(uint8_t) );

	return 0;
}

struct _type_menu_command_command
{
	enum enum_menu_cmd menu_cmd;
	int (*menu_run_commmand)( uint16_t value, uint8_t  *p_GetParam );
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
	{ MENU_UI_GET_PARAM, menuGetMeetParamPro },
	{ MENU_UI_SAVE_PARAM, menuSaveMeetParamPro },
	{ MENU_UI_SEND_MAIN_STATE, menuSendMainStatePro },
	{ MENU_UI_SAVE_WIRE_ADDR, menuSaveWireAddrPro },
	{ MENU_UI_GET_WIRE_ADDR, menuGetWireAddrPro },
	{ MENU_UI_CHN_PRIOR, menuChnPriorPro},
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
int menu_cmd_run( enum enum_menu_cmd menu_cmd, uint16_t value, uint8_t  *p_GetParam )
{	
	int ret = -1;
	struct _type_menu_command_command *p = &gtable_menu_command[0];

	while( p->menu_cmd != MENU_UI_MENU_CMD_ERR )
	{
		if( menu_cmd ==  p->menu_cmd )
		{
			if( p->menu_run_commmand != NULL )
				ret = p->menu_run_commmand( value, p_GetParam );
			
			ret = 0;
			break;
		}

		p++;
	}

	return ret;
}

