/*pelco_d.h
**
**
**
*/

#ifndef __PELCO_D_H__
#define __PELCO_D_H__

#include "jdksavdecc_world.h"
#include "host_controller_debug.h"

/*{@D型控制协议*/
#define CAMERA_SYNC_BYTE_HEAD 0xFF 	// 同步字节
#define CAMERA_PELCO_D_DEAL_LEN 7

#define CAMERA_CTRL_STOP 0x0000
#define CAMERA_CTRL_AUTO_SCAN 0x0010	 // 自动扫描功能
#define CAMERA_CTRL_IRIS_CLOSE 0x0004	 // 光圈缩小 ok
#define CAMERA_CTRL_IRIS_OPEN 0x0002	 // 光圈扩大ok
#define CAMERA_CTRL_FOCUCS_NEAR 0x0001	// 近距离聚焦 ok
#define CAMERA_CTRL_FOCUCS_FAR 0x8000 	// 远距离聚焦 ok
#define CAMERA_CTRL_ZOOM_WIDE 0x4000	// 远离物体 ok
#define CAMERA_CTRL_ZOOM_TELE 0x2000 	// 接近物体ok
#define CAMERA_CTRL_DOWN 0x1000 		// 下 ok
#define CAMERA_CTRL_UP 0x0800 			// 上 ok
#define CAMERA_CTRL_LEFT 0x0400			// 左ok
#define CAMERA_CTRL_RIGHT 0x0200 		// 右ok

#define CAMERA_CTRL_PRESET_SET 0x0300	// 设置预置点
#define CAMERA_CTRL_PRESET_CALL 0x0700	// 调用预置点

/*@}*/

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
	uint16_t order;						// 指令码1  注: 当是预置点命令时 此元素为0  指令码2   注:当是预置点命令 03 07 分别为设置与调用预置点命令
	control_lv_speed data_code_1;		// 数据码1 注: 当是预置点命令时 此元素为0
	control_vtcl_speed data_code_2;		// 数据码2 注:当是预置点命令，此元素是代表预置点号 
	uint8_t check_digit; 				// 校验码
}pelco_d_format;

#define DIVISOR_CHECK_COUNT 0x000000FF
#define CHECK_DIGIT_RESULT( addr,cmd1,cmd2,data1,data2 ) (((addr)+(cmd1)+(cmd2)+(data1)+(data2))&(DIVISOR_CHECK_COUNT))// 计算校验码

int pelco_d_cammand_set( uint8_t camera_address,  uint16_t d_cmd, uint8_t speed_lv, uint8_t speed_vertical, pelco_d_format* askbuf );

#endif
