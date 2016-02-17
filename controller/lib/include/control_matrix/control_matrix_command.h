/*
**control_matrix_command.h
**17-2-2016
**
**
*/

#ifndef __CONTROL_MATRIX_COMMAND_H__
#define __CONTROL_MATRIX_COMMAND_H__

#include "jdksavdecc_world.h"

typedef struct _type_matrix_command
{
	uint16_t command;// ����
	uint16_t command_len;// �����ַ�������
	uint8_t string_command[];// ansii������
}smatrix_command;

/*{@����VGAָ��ϵͳָ���д(vga�����л�ϵͳ�û��ֲ��ʮ��--ͨ��Э�������ָ�����)*/
typedef enum _enum_matrix_command
{
	MATRIX_QUEUE_TYPE,// ��ѯ������ͺ�
	MATRIX_CHANGE_KEY_PASSWORD,// �޸ľ�����̵Ŀ�������
	MATRIX_LOCK_KEYBOARD,// ��������
	MATRIX_UNLOCK_KEYBOARD,// �⿪���̵�����
	MATRIX_CLOSE_BELL,// �رշ�����
	MATRIX_OPEN_BELL,// �򿪷�����
	MATRIX_QUEUE_VERSION,// ��ѯ����İ汾
	MATRIX_SET_EXTRON,// ���óɼ���ָ��ϵͳ
	MATRIX_SET_CREATOR20,// ���ó�CREAROR2.0ָ��ϵͳ
	MATRIX_CLOSE_SERIAL_RETURN_MSG,// �رմ��ڷ���
	MATRIX_OPEN_SERIAL_RETURN_MSG,// �򿪴��ڷ���
	MATRIX_SET_LCD_BACKLIGHT_TIME,// ����lcd�ı���ʱ��

	MATRIX_SET_OUTIN_CORRESPONDING, // ��������ͨ��һһ��Ӧ
	MATRIX_CLOSE_ALL_OUTPUT_CHANNAL,// �ر��������ͨ��
	MATRIX_QUEUE_OUT_CHANNAL_INPUT_STATUS,// ��ѯ�������ͨ���ĵ��źŵ�����״̬
	MATRIX_COMMAND_NUM
}enum_matrix_command;

/*ASCII������ȶ���*/
#define ERROR_MATRIX_STRING_CMD_LEN 0xffff

#define MATRIX_STRING_CMD_QUEUE_TYPE_LEN 7
#define MATRIX_STRING_CMD_CHANGE_KEY_PASSWORD_LEN 2// ʹ��ʱ���������µ�������";"������������";"�ĳ���
#define MATRIX_STRING_CMD_LOCK_KEYBOARD_LEN 7
#define MATRIX_STRING_CMD_UNLOCK_KEYBOARD_LEN 9
#define MATRIX_STRING_CMD_CLOSE_BELL_LEN 10
#define MATRIX_STRING_CMD_OPEN_BELL_LEN 9
#define MATRIX_STRING_CMD_QUEUE_VERSION_LEN 10
#define MATRIX_STRING_CMD_SET_EXTRON_LEN 9
#define MATRIX_STRING_CMD_SET_CREATOR20_LEN 12
#define MATRIX_STRING_CMD_CLOSE_SERIAL_RETURN_MSG_LEN 13
#define MATRIX_STRING_CMD_OPEN_SERIAL_RETURN_MSG_LEN 12
#define MATRIX_STRING_CMD_SET_LCD_BACKLIGHT_TIME_LEN 11// ����lcd�ı���ʱ��,ʹ��ʱ������������λ����ʱʱ����";"������λ����ʱʱ����";"�ĳ���

#define MATRIX_STRING_CMD_SET_OUTIN_CORRESPONDING_LEN 5
#define MATRIX_STRING_CMD_CLOSE_ALL_OUTPUT_CHANNAL_LEN 5
#define MATRIX_STRING_CMD_QUEUE_OUT_CHANNAL_INPUT_STATUS_LEN 7
/*ASCII������ȶ���*/

/*ASCII�������*/
#define MATRIX_STRING_CMD_QUEUE_TYPE "/*Type;"
#define MATRIX_STRING_CMD_CHANGE_KEY_PASSWORD "/+"// ʹ��ʱ���������µ�������";"
#define MATRIX_STRING_CMD_LOCK_KEYBOARD "/%Lock;"
#define MATRIX_STRING_CMD_UNLOCK_KEYBOARD "/%Unlock;"
#define MATRIX_STRING_CMD_CLOSE_BELL "/:BellOff;"
#define MATRIX_STRING_CMD_OPEN_BELL "/:BellOn;"
#define MATRIX_STRING_CMD_QUEUE_VERSION "/^Version;"
#define MATRIX_STRING_CMD_SET_EXTRON "/~EXTRON;"
#define MATRIX_STRING_CMD_SET_CREATOR20 "/~CREATOR20;"
#define MATRIX_STRING_CMD_CLOSE_SERIAL_RETURN_MSG "/:MessageOff;"
#define MATRIX_STRING_CMD_OPEN_SERIAL_RETURN_MSG "/:MessageOn;"
#define MATRIX_STRING_CMD_SET_LCD_BACKLIGHT_TIME "/%Backlight"// ����lcd�ı���ʱ��,ʹ��ʱ������������λ����ʱʱ����";"

#define MATRIX_STRING_CMD_SET_OUTIN_CORRESPONDING "All#." 
#define MATRIX_STRING_CMD_CLOSE_ALL_OUTPUT_CHANNAL "All$."
#define MATRIX_STRING_CMD_QUEUE_OUT_CHANNAL_INPUT_STATUS "Status."
/*ASCII�������*/

extern const uint8_t* control_matrix_comand_get( uint16_t cmd_index, uint16_t* p_ascii_cmd_len );

#endif
