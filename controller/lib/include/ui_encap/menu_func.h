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

enum enum_menu_cmd
{
	MENU_UI_MODESET,		// ģʽ���� ���� ����ģʽ value(1:ppt; 2:limit; 3:fifo; 4:apply); p_GetParam = NULL
	MENU_UI_TEMPCLOSE,		// ��ʱ�ر� value( ����: �ر�; 0: �� );p_GetParam = NULL
	MENU_UI_CHAIRMANHINT,	// ��ϯ����(��ʾ)  value( 0:�ر�;  ����: �� );p_GetParam = NULL
	MENU_UI_CAMERATRACK,	// ������� value( ����: ����; 0: ������ );p_GetParam = NULL
	MENU_UI_AUTOCLOSE,		// �Զ��ر� value( ����: �ر�; 0: �� );p_GetParam = NULL
	MENU_UI_SPEAKLIMIT,		// ���������� value( ������ 6 );p_GetParam = NULL
	MENU_UI_APPLYNUMSET,	// �������� value( ������ 10 );p_GetParam = NULL
	MENU_UI_REALLOT,			// ���·��� value(��);p_GetParam = NULL
	MENU_UI_NEWALLOT,		// �������� value(��);p_GetParam = NULL
	MENU_UI_SETFINISH,		// ������� value(��);p_GetParam = NULL
	MENU_UI_CAMERACTLLEFTRIGHT,// value( ����: ��; 0: �� );p_GetParam = NULL
	MENU_UI_CAMERACTLUPDOWN,// value( ����: ��; 0: �� );p_GetParam = NULL
	MENU_UI_CAMERACTLFOUCE,	// value( ����: ������۽�; 0: Զ����۽� );p_GetParam = NULL
	MENU_UI_CAMERACTLIRIS,	// value( ����: ��Ȧ����; 0: ��Ȧ��С );p_GetParam = NULL
	MENU_UI_CAMERACTLZOOM,	// value( ����: �ӽ�����; 0: Զ������  );p_GetParam = NULL
	MENU_UI_DISTANCECTL,		// ң�������� value( ����: �� ; 0: �ر�);p_GetParam = NULL
	MENU_UI_PRESETSAVE,		// Ԥ��λ���� value(��);p_GetParam = NULL
	MENU_UI_SWITCHCMR,		// �л�Ϊ��ǰ����ͷ value( 1-4 );p_GetParam = NULL
	MENU_UI_CLEARPRESET,		// ����Ԥ��λ value(��);p_GetParam = NULL
	MENU_UI_SELECTPRESETADDR,// Ԥ��λ���ն�ѡ��value( �ն˵ĵ�ַ );p_GetParam = NULL
	MENU_UI_ENTERESCPRESET,	// �����˳�����ͷ�Ŀ���(�ڿ���ǰ������Ӧ�Ĳ��� value:���� ����; 0 �˳�);p_GetParam = NULL
	MENU_UI_GET_PARAM,// ��ȡ������� menu_cmd = MENU_UI_GET_PARAM); value ��EPar�е�ֵ;p_GetSaveParam �Ǻ�����ȡ��������ֵ��ŵĵ�ַ
	MENU_UI_SAVE_PARAM,// ���������� menu_cmd = MENU_UI_SAVE_PARAM); value ��EPar�е�ֵ;*p_GetSaveParam �Ǳ��������ֵ
	MENU_UI_SEND_MAIN_STATE,// ����������״̬; value(��) ;p_GetSaveParam = NULL
	MENU_UI_CHM_FIRST,// ��ϯ����
	MENU_UI_SAVE_WIRE_ADDR,// ��������ң�ص�ַ value = �����ַ�ĳ��� p_GetSaveParam Ϊ��ַ�ı���ĵ�ַָ��
	MENU_UI_GET_WIRE_ADDR,// ��ȡ����ң�ص�ַ value = 0��p_GetSaveParam Ϊ��ַ�ı���ĵ�ַָ��
	MENU_UI_MENU_CMD_ERR = 0xffff
};

extern int menu_cmd_run( enum enum_menu_cmd menu_cmd, uint16_t value, uint8_t  *p_GetSaveParam );

#endif

