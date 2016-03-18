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

#define MENU_CMD_PARAM_MAX_NUM 128

#define CMD_OPTION_MAX_NUM 20 // 命令行参数个数
#define CMD_OPTION_STRING_LEN 50// 命令行参数长度

enum enum_menu_cmd
{
	MENU_UI_MODESET,		// 模式设置 除了 自由模式 value(1:ppt; 2:limit; 3:fifo; 4:apply)
	MENU_UI_TEMPCLOSE,		// 临时关闭
	MENU_UI_CHAIRMANHINT,	// 主席音乐(提示)
	MENU_UI_CAMERATRACK,	// 摄像跟踪 
	MENU_UI_AUTOCLOSE,		// 自动关闭
	MENU_UI_SPEAKLIMIT,		// 开启量上限
	MENU_UI_APPLYNUMSET,	// 申请上限
	MENU_UI_REALLOT,			// 重新分配
	MENU_UI_NEWALLOT,		// 新增分配
	MENU_UI_SETFINISH,		// 分配完成(这里程序暂不做处理(2016-3-18))
	MENU_UI_CAMERACTL, 		// 用于命令行接收参数,不可用于非命令行的参数接收
	MENU_UI_CAMERACTLLEFTRIGHT,// 可以用于命令行也可用于非命令行的参数接收，即都有相对应的执行函数
	MENU_UI_CAMERACTLUPDOWN,// 可以用于命令行也可用于非命令行的参数接收，即都有相对应的执行函数
	MENU_UI_CAMERACTLFOUCE,	// 可以用于命令行也可用于非命令行的参数接收，即都有相对应的执行函数
	MENU_UI_CAMERACTLIRIS,	// 可以用于命令行也可用于非命令行的参数接收，即都有相对应的执行函数
	MENU_UI_CAMERACTLZOOM,	// 可以用于命令行也可用于非命令行的参数接收，即都有相对应的执行函数
	MENU_UI_DISTANCECTL,		// 遥控器对码(具体的功能未能实现，因为与遥控器相关2016-3-18)
	MENU_UI_PRESETSAVE,		// 预置位保存
	MENU_UI_SWITCHCMR,		// 切换当前摄像头(有待完善2016-3-18)
	MENU_UI_CLEARPRESET,		// 消除预置位
	MENU_UI_SELECTPRESETADDR,// 预置位的终端选择
	MENU_UI_ENTERESCPRESET,	// 进入退出摄像头的控制(在控制前设置相应的参数:非零 进入; 0 退出)
	MENU_UI_MENU_CMD_HELP,	// 帮助菜单，只用于命令行参数
	MENU_UI_MENU_CMD_ERR = 0xffff
};

extern void menu_cmd_line_run( char (*args)[CMD_OPTION_STRING_LEN] );
extern void menu_cmd_run( enum enum_menu_cmd menu_cmd, uint16_t value );

#endif

