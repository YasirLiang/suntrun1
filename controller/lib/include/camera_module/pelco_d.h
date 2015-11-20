#ifndef __PELCO_D_H__
#define __PELCO_D_H__

#include "jdksavdecc_world.h"
#include "host_controller_debug.h"

typedef struct _type_instruction_word_1 
{	
	uint8_t focucs_near:1;	// ������۽�
	uint8_t iris_open:1;  	// ��Ȧ����
	uint8_t irs_close:1; 		// ��Ȧ��С
	uint8_t zero_elem1:1;     	// ʼ��Ϊ��
	uint8_t auto_scan:1;	// �Զ�ɨ�蹦�ܿ���λ
	uint8_t zero_elem3:3; 	// 3λ��Ϊ����ʼ��Ϊ��
}instruction_word_1;

typedef struct _type_instruction_word_2
{
	uint8_t zero_elem1:1;	// ʼ��Ϊ��
	uint8_t right:1;			// ����  
	uint8_t left:1;			// ����
	uint8_t up:1;			// ����
	uint8_t down:1;		// ����
	uint8_t zoom_tele:1;	// ����������ı䱶 �ӽ�����
	uint8_t zoom_wide:1;	// ����������ı䱶 Զ������
	uint8_t fucus_far:1;		// Զ����۽�
}instruction_word_2;

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
	instruction_word_1 order_1;			// ָ����1  ע: ����Ԥ�õ�����ʱ ��Ԫ��Ϊ0
	instruction_word_2 order_2;			// ָ����2   ע:����Ԥ�õ����� 03 07 �ֱ�Ϊ���������Ԥ�õ�����
	control_lv_speed data_code_1;		// ������1 ע: ����Ԥ�õ�����ʱ ��Ԫ��Ϊ0
	control_vtcl_speed data_code_2;		// ������2 ע:����Ԥ�õ������Ԫ���Ǵ���Ԥ�õ�� 
	uint8_t check_digit; 				// У����
}pelco_d_format;

#define DIVISOR_CHECK_COUNT 0x100
#define CHECK_DIGIT_RESULT(x,y,z,m,n) (((x)+(y)+(z)+(m)+(n))/(DIVISOR_CHECK_COUNT))

#endif
