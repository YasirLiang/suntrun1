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
	MENU_UI_GET_PARAM,// ��ȡ������� menu_cmd = MENU_UI_GET_PARAM); value ��EPar�е�ֵ;p_GetParam �Ǻ�����ȡ��������ֵ��ŵĵ�ַ
	MENU_UI_MENU_CMD_ERR = 0xffff
};

typedef enum
{
	VAL_TEMP_CLOSE=0,	//��ʱ�ر�
	VAL_CHM_MUSIC,		//��ϯ����
	VAL_CMR_TRACK_EN,	//�������ʹ��
	VAL_AUTO_CLOSE,		//�Զ��ر�
	VAL_CUR_CMR,		//��ǰ����ͷ
	VAL_SPKER_LIMIT,		//������������
	VAL_APPLY_LIMIT,		//������������
	VAL_DSCS_MODE, 		//����ģʽ
}EPar;

extern void menu_cmd_run( enum enum_menu_cmd menu_cmd, uint16_t value, uint8_t  *p_GetParam );

#endif

