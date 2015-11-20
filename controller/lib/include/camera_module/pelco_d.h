#ifndef __PELCO_D_H__
#define __PELCO_D_H__

#include "jdksavdecc_world.h"
#include "host_controller_debug.h"

typedef struct _type_instruction_word_1 
{	
	uint8_t focucs_near:1;	// 近距离聚焦
	uint8_t iris_open:1;  	// 光圈扩大
	uint8_t irs_close:1; 		// 光圈缩小
	uint8_t zero_elem1:1;     	// 始终为零
	uint8_t auto_scan:1;	// 自动扫描功能控制位
	uint8_t zero_elem3:3; 	// 3位都为零且始终为零
}instruction_word_1;

typedef struct _type_instruction_word_2
{
	uint8_t zero_elem1:1;	// 始终为零
	uint8_t right:1;			// 右移  
	uint8_t left:1;			// 左移
	uint8_t up:1;			// 上移
	uint8_t down:1;		// 下移
	uint8_t zoom_tele:1;	// 控制摄像机的变倍 接近物体
	uint8_t zoom_wide:1;	// 控制摄像机的变倍 远离物体
	uint8_t fucus_far:1;		// 远距离聚焦
}instruction_word_2;

typedef struct _type_control_level_speed
{
	uint8_t level_speed;		// 范围(00-3FH)
}control_lv_speed;

typedef struct _type_control_vertical_speed
{
	uint8_t vertical_speed;		// 范围(00-3FH)
}control_vtcl_speed;

typedef struct _pelco_d_command_form	// D型控制协议命令格式
{
	uint8_t sync;						// 同步字节
	uint8_t bit_id;						// 地址码
	instruction_word_1 order_1;			// 指令码1  注: 当是预置点命令时 此元素为0
	instruction_word_2 order_2;			// 指令码2   注:当是预置点命令 03 07 分别为设置与调用预置点命令
	control_lv_speed data_code_1;		// 数据码1 注: 当是预置点命令时 此元素为0
	control_vtcl_speed data_code_2;		// 数据码2 注:当是预置点命令，此元素是代表预置点号 
	uint8_t check_digit; 				// 校验码
}pelco_d_format;

#define DIVISOR_CHECK_COUNT 0x100
#define CHECK_DIGIT_RESULT(x,y,z,m,n) (((x)+(y)+(z)+(m)+(n))/(DIVISOR_CHECK_COUNT))

#endif
