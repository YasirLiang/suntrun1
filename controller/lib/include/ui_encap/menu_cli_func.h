/*
**file:menu_cli_func.h
**data:2016-3-21
**
**
*/

// ********************
// ��ӽ���˵����ܺ�����������ģ��
// �Ա�����ģ��ĵ���
// 
//*******************

#ifndef __MENU_CLI_FUNC_H__
#define __MENU_CLI_FUNC_H__

#include "jdksavdecc_world.h"

#define MENU_CMD_PARAM_MAX_NUM 128

#define CMD_OPTION_MAX_NUM 20 // �����в�������
#define CMD_OPTION_STRING_LEN 50// �����в�������

extern void menu_cmd_line_run( char (*args)[CMD_OPTION_STRING_LEN] );

#endif
