/*pelco_d.h
**
**
**
*/

#ifndef __PELCO_D_H__
#define __PELCO_D_H__

#include "jdksavdecc_world.h"
#include "host_controller_debug.h"

/*{@D�Ϳ���Э��*/
#define CAMERA_SYNC_BYTE_HEAD 0xFF 	// ͬ���ֽ�
#define CAMERA_PELCO_D_DEAL_LEN 7

#define CAMERA_CTRL_STOP 0x0000
#define CAMERA_CTRL_AUTO_SCAN 0x0010	 // �Զ�ɨ�蹦��
#define CAMERA_CTRL_IRIS_CLOSE 0x0004	 // ��Ȧ��С ok
#define CAMERA_CTRL_IRIS_OPEN 0x0002	 // ��Ȧ����ok
#define CAMERA_CTRL_FOCUCS_NEAR 0x0001	// ������۽� ok
#define CAMERA_CTRL_FOCUCS_FAR 0x8000 	// Զ����۽� ok
#define CAMERA_CTRL_ZOOM_WIDE 0x4000	// Զ������ ok
#define CAMERA_CTRL_ZOOM_TELE 0x2000 	// �ӽ�����ok
#define CAMERA_CTRL_DOWN 0x1000 		// �� ok
#define CAMERA_CTRL_UP 0x0800 			// �� ok
#define CAMERA_CTRL_LEFT 0x0400			// ��ok
#define CAMERA_CTRL_RIGHT 0x0200 		// ��ok

#define CAMERA_CTRL_PRESET_SET 0x0300	// ����Ԥ�õ�
#define CAMERA_CTRL_PRESET_CALL 0x0700	// ����Ԥ�õ�

/*@}*/

typedef struct _type_control_level_speed
{
	uint8_t level_speed;		// ��Χ(00-3FH)
}control_lv_speed;

typedef struct _type_control_vertical_speed
{
	uint8_t vertical_speed;		// ��Χ(00-3FH)
}control_vtcl_speed;

typedef struct _pelco_d_command_form	// D�Ϳ���Э�������ʽ
{
	uint8_t sync;						// ͬ���ֽ�
	uint8_t bit_id;						// ��ַ��
	uint16_t order;						// ָ����1  ע: ����Ԥ�õ�����ʱ ��Ԫ��Ϊ0  ָ����2   ע:����Ԥ�õ����� 03 07 �ֱ�Ϊ���������Ԥ�õ�����
	control_lv_speed data_code_1;		// ������1 ע: ����Ԥ�õ�����ʱ ��Ԫ��Ϊ0
	control_vtcl_speed data_code_2;		// ������2 ע:����Ԥ�õ������Ԫ���Ǵ���Ԥ�õ�� 
	uint8_t check_digit; 				// У����
}pelco_d_format;

#define DIVISOR_CHECK_COUNT 0x000000FF
#define CHECK_DIGIT_RESULT( addr,cmd1,cmd2,data1,data2 ) (((addr)+(cmd1)+(cmd2)+(data1)+(data2))&(DIVISOR_CHECK_COUNT))// ����У����

int pelco_d_cammand_set( uint8_t camera_address,  uint16_t d_cmd, uint8_t speed_lv, uint8_t speed_vertical, pelco_d_format* askbuf );

#endif
