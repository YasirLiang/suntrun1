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
	uint16_t command;// 命令
	uint16_t command_len;// 命令字符串长度
	uint8_t string_command[];// ansii命令码
}smatrix_command;

/*{@依据VGA指令系统指令编写(vga矩阵切换系统用户手册第十节--通信协议与控制指令代码)*/
typedef enum _enum_matrix_command
{
	MATRIX_QUEUE_TYPE,// 查询矩阵的型号
	MATRIX_CHANGE_KEY_PASSWORD,// 修改矩阵键盘的开启密码
	MATRIX_LOCK_KEYBOARD,// 锁定键盘
	MATRIX_UNLOCK_KEYBOARD,// 解开键盘的锁定
	MATRIX_CLOSE_BELL,// 关闭蜂鸣器
	MATRIX_OPEN_BELL,// 打开蜂鸣器
	MATRIX_QUEUE_VERSION,// 查询软件的版本
	MATRIX_SET_EXTRON,// 设置成兼容指令系统
	MATRIX_SET_CREATOR20,// 设置成CREAROR2.0指令系统
	MATRIX_CLOSE_SERIAL_RETURN_MSG,// 关闭串口返回
	MATRIX_OPEN_SERIAL_RETURN_MSG,// 打开串口返回
	MATRIX_SET_LCD_BACKLIGHT_TIME,// 设置lcd的背光时间

	MATRIX_SET_OUTIN_CORRESPONDING, // 设置所有通道一一对应
	MATRIX_CLOSE_ALL_OUTPUT_CHANNAL,// 关闭所有输出通道
	MATRIX_QUEUE_OUT_CHANNAL_INPUT_STATUS,// 查询所有输出通道的的信号的输入状态
	MATRIX_COMMAND_NUM
}enum_matrix_command;

/*ASCII码命令长度定义*/
#define ERROR_MATRIX_STRING_CMD_LEN 0xffff

#define MATRIX_STRING_CMD_QUEUE_TYPE_LEN 7
#define MATRIX_STRING_CMD_CHANGE_KEY_PASSWORD_LEN 2// 使用时在其后加上新的密码与";"；除了密码与";"的长度
#define MATRIX_STRING_CMD_LOCK_KEYBOARD_LEN 7
#define MATRIX_STRING_CMD_UNLOCK_KEYBOARD_LEN 9
#define MATRIX_STRING_CMD_CLOSE_BELL_LEN 10
#define MATRIX_STRING_CMD_OPEN_BELL_LEN 9
#define MATRIX_STRING_CMD_QUEUE_VERSION_LEN 10
#define MATRIX_STRING_CMD_SET_EXTRON_LEN 9
#define MATRIX_STRING_CMD_SET_CREATOR20_LEN 12
#define MATRIX_STRING_CMD_CLOSE_SERIAL_RETURN_MSG_LEN 13
#define MATRIX_STRING_CMD_OPEN_SERIAL_RETURN_MSG_LEN 12
#define MATRIX_STRING_CMD_SET_LCD_BACKLIGHT_TIME_LEN 11// 设置lcd的背光时间,使用时需在命令后加三位的延时时间与";"除了三位的延时时间与";"的长度

#define MATRIX_STRING_CMD_SET_OUTIN_CORRESPONDING_LEN 5
#define MATRIX_STRING_CMD_CLOSE_ALL_OUTPUT_CHANNAL_LEN 5
#define MATRIX_STRING_CMD_QUEUE_OUT_CHANNAL_INPUT_STATUS_LEN 7
/*ASCII码命令长度定义*/

/*ASCII码命令定义*/
#define MATRIX_STRING_CMD_QUEUE_TYPE "/*Type;"
#define MATRIX_STRING_CMD_CHANGE_KEY_PASSWORD "/+"// 使用时在其后加上新的密码与";"
#define MATRIX_STRING_CMD_LOCK_KEYBOARD "/%Lock;"
#define MATRIX_STRING_CMD_UNLOCK_KEYBOARD "/%Unlock;"
#define MATRIX_STRING_CMD_CLOSE_BELL "/:BellOff;"
#define MATRIX_STRING_CMD_OPEN_BELL "/:BellOn;"
#define MATRIX_STRING_CMD_QUEUE_VERSION "/^Version;"
#define MATRIX_STRING_CMD_SET_EXTRON "/~EXTRON;"
#define MATRIX_STRING_CMD_SET_CREATOR20 "/~CREATOR20;"
#define MATRIX_STRING_CMD_CLOSE_SERIAL_RETURN_MSG "/:MessageOff;"
#define MATRIX_STRING_CMD_OPEN_SERIAL_RETURN_MSG "/:MessageOn;"
#define MATRIX_STRING_CMD_SET_LCD_BACKLIGHT_TIME "/%Backlight"// 设置lcd的背光时间,使用时需在命令后加三位的延时时间与";"

#define MATRIX_STRING_CMD_SET_OUTIN_CORRESPONDING "All#." 
#define MATRIX_STRING_CMD_CLOSE_ALL_OUTPUT_CHANNAL "All$."
#define MATRIX_STRING_CMD_QUEUE_OUT_CHANNAL_INPUT_STATUS "Status."
/*ASCII码命令定义*/

extern const uint8_t* control_matrix_comand_get( uint16_t cmd_index, uint16_t* p_ascii_cmd_len );

#endif
