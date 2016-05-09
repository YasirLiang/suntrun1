/*
**file:menu_func.h
**data:2016-3-17
**
**
*/

// ********************
// 添加界面菜单功能函数模块
// 以便其他模块的调用
// 
//*******************

#ifndef __MENU_FUNC_H__
#define __MENU_FUNC_H__

#include "jdksavdecc_world.h"

enum enum_menu_cmd
{
	MENU_UI_MODESET,		// 模式设置 除了 自由模式 value(1:ppt; 2:limit; 3:fifo; 4:apply); p_GetParam = NULL
	MENU_UI_TEMPCLOSE,		// 临时关闭 value( 非零: 关闭; 0: 打开 );p_GetParam = NULL
	MENU_UI_CHAIRMANHINT,	// 主席音乐(提示)  value( 0:关闭;  非零: 打开 );p_GetParam = NULL
	MENU_UI_CAMERATRACK,	// 摄像跟踪 value( 非零: 跟踪; 0: 不跟踪 );p_GetParam = NULL
	MENU_UI_AUTOCLOSE,		// 自动关闭 value( 非零: 关闭; 0: 打开 );p_GetParam = NULL
	MENU_UI_SPEAKLIMIT,		// 开启量上限 value( 不大于 6 );p_GetParam = NULL
	MENU_UI_APPLYNUMSET,	// 申请上限 value( 不大于 10 );p_GetParam = NULL
	MENU_UI_REALLOT,			// 重新分配 value(无);p_GetParam = NULL
	MENU_UI_NEWALLOT,		// 新增分配 value(无);p_GetParam = NULL
	MENU_UI_SETFINISH,		// 分配完成 value(无);p_GetParam = NULL
	MENU_UI_CAMERACTLLEFTRIGHT,// value( 非零: 右; 0: 左 );p_GetParam = NULL
	MENU_UI_CAMERACTLUPDOWN,// value( 非零: 上; 0: 下 );p_GetParam = NULL
	MENU_UI_CAMERACTLFOUCE,	// value( 非零: 近距离聚焦; 0: 远距离聚焦 );p_GetParam = NULL
	MENU_UI_CAMERACTLIRIS,	// value( 非零: 光圈扩大; 0: 光圈缩小 );p_GetParam = NULL
	MENU_UI_CAMERACTLZOOM,	// value( 非零: 接近物体; 0: 远离物体  );p_GetParam = NULL
	MENU_UI_DISTANCECTL,		// 遥控器对码 value( 非零: 打开 ; 0: 关闭);p_GetParam = NULL
	MENU_UI_PRESETSAVE,		// 预置位保存 value(无);p_GetParam = NULL
	MENU_UI_SWITCHCMR,		// 切换为当前摄像头 value( 1-4 );p_GetParam = NULL
	MENU_UI_CLEARPRESET,		// 消除预置位 value(无);p_GetParam = NULL
	MENU_UI_SELECTPRESETADDR,// 预置位的终端选择value( 终端的地址 );p_GetParam = NULL
	MENU_UI_ENTERESCPRESET,	// 进入退出摄像头的控制(在控制前设置相应的参数 value:非零 进入; 0 退出);p_GetParam = NULL
	MENU_UI_GET_PARAM,// 获取会议参数 menu_cmd = MENU_UI_GET_PARAM); value 是EPar中的值;p_GetSaveParam 是函数获取参数返回值存放的地址
	MENU_UI_SAVE_PARAM,// 保存会议参数 menu_cmd = MENU_UI_SAVE_PARAM); value 是EPar中的值;*p_GetSaveParam 是保存参数的值
	MENU_UI_SEND_MAIN_STATE,// 发送主机的状态; value(无) ;p_GetSaveParam = NULL
	MENU_UI_CHM_FIRST,// 主席优先
	MENU_UI_SAVE_WIRE_ADDR,// 保存无线遥控地址 value = 保存地址的长度 p_GetSaveParam 为地址的保存的地址指针
	MENU_UI_GET_WIRE_ADDR,// 获取无线遥控地址 value = 0；p_GetSaveParam 为地址的保存的地址指针
	MENU_UI_MENU_CMD_ERR = 0xffff
};

extern int menu_cmd_run( enum enum_menu_cmd menu_cmd, uint16_t value, uint8_t  *p_GetSaveParam );

#endif

