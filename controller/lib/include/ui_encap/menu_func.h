/*
**file:menu_func.h
**data:2016-3-17
**
**
*/

// ********************
// ��ӽ���˵����ܺ���ģ��
// �Ա�����ģ��ĵ���
// 
//*******************

#ifndef __MENU_FUNC_H__
#define __MENU_FUNC_H__

#include "jdksavdecc_world.h"

#define MENU_CMD_PARAM_MAX_NUM 128

#define CMD_OPTION_MAX_NUM 20 // �����в�������
#define CMD_OPTION_STRING_LEN 50// �����в�������

enum enum_menu_cmd
{
	MENU_UI_MODESET,		// ģʽ���� ���� ����ģʽ value(1:ppt; 2:limit; 3:fifo; 4:apply)
	MENU_UI_TEMPCLOSE,		// ��ʱ�ر�
	MENU_UI_CHAIRMANHINT,	// ��ϯ����(��ʾ)
	MENU_UI_CAMERATRACK,	// ������� 
	MENU_UI_AUTOCLOSE,		// �Զ��ر�
	MENU_UI_SPEAKLIMIT,		// ����������
	MENU_UI_APPLYNUMSET,	// ��������
	MENU_UI_REALLOT,			// ���·���
	MENU_UI_NEWALLOT,		// ��������
	MENU_UI_SETFINISH,		// �������(��������ݲ�������(2016-3-18))
	MENU_UI_CAMERACTL, 		// ���������н��ղ���,�������ڷ������еĲ�������
	MENU_UI_CAMERACTLLEFTRIGHT,// ��������������Ҳ�����ڷ������еĲ������գ����������Ӧ��ִ�к���
	MENU_UI_CAMERACTLUPDOWN,// ��������������Ҳ�����ڷ������еĲ������գ����������Ӧ��ִ�к���
	MENU_UI_CAMERACTLFOUCE,	// ��������������Ҳ�����ڷ������еĲ������գ����������Ӧ��ִ�к���
	MENU_UI_CAMERACTLIRIS,	// ��������������Ҳ�����ڷ������еĲ������գ����������Ӧ��ִ�к���
	MENU_UI_CAMERACTLZOOM,	// ��������������Ҳ�����ڷ������еĲ������գ����������Ӧ��ִ�к���
	MENU_UI_DISTANCECTL,		// ң��������(����Ĺ���δ��ʵ�֣���Ϊ��ң�������2016-3-18)
	MENU_UI_PRESETSAVE,		// Ԥ��λ����
	MENU_UI_SWITCHCMR,		// �л���ǰ����ͷ(�д�����2016-3-18)
	MENU_UI_CLEARPRESET,		// ����Ԥ��λ
	MENU_UI_SELECTPRESETADDR,// Ԥ��λ���ն�ѡ��
	MENU_UI_ENTERESCPRESET,	// �����˳�����ͷ�Ŀ���(�ڿ���ǰ������Ӧ�Ĳ���:���� ����; 0 �˳�)
	MENU_UI_MENU_CMD_HELP,	// �����˵���ֻ���������в���
	MENU_UI_MENU_CMD_ERR = 0xffff
};

extern void menu_cmd_line_run( char (*args)[CMD_OPTION_STRING_LEN] );
extern void menu_cmd_run( enum enum_menu_cmd menu_cmd, uint16_t value );

#endif

