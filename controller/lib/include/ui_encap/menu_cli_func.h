/*
**file:menu_cli_func.h
**data:2016-3-21
**
**
*/

// ********************
// 添加界面菜单功能函数的命令行模块
// 以便其他模块的调用
// 
//*******************

#ifndef __MENU_CLI_FUNC_H__
#define __MENU_CLI_FUNC_H__

#include "jdksavdecc_world.h"

#define MENU_CMD_PARAM_MAX_NUM 128

#define CMD_OPTION_MAX_NUM 20 // 命令行参数个数
#define CMD_OPTION_STRING_LEN 50// 命令行参数长度

extern void menu_cmd_line_run( char (*args)[CMD_OPTION_STRING_LEN] );

#endif
