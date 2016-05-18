/*
**central_control_transmit_unit.h
**9-3-2016
**
*/

// ****************************
// file built 2016-3-9
// ��������ϵͳ���ӱ�������
// ���Ŀ��Ƶ�Ԫ����ģ��
// ****************************

#ifndef __CENTRAL_CONTROL_TRANSMIT_UNIT_H__
#define __CENTRAL_CONTROL_TRANSMIT_UNIT_H__

#include "output_channel.h"
#include "global.h"
#include "descriptor.h"

//************************************

#define CCU_CONTROL_TRANSMIT_UNIT_NAME central_control_unit_name
#define CCU_CONTROL_TRANSMIT_UNIT_MAX_NUM central_control_unit_max_num
#define CCU_CONTROL_TRANSMIT_UINT_OUTPUT central_control_transmit_uint_output
#define CCU_CONTROL_TRANSMIT_UINT_MAX_OUTPUT_NUM 4

enum _enum_ccu_chout_pro// ����㲥�ߵĳ�ʼ������
{
	CENTRAL_OUT_PRIMITED = 0,// �������δ����
	CENTRAL_OUT_HANDLE,// �������������
	CENTRAL_OUT_FINISH//  ���������ʼ�����
};

typedef enum ccu_transmit_model_output_state
{
	TCCU_LEISURE,// ����
	TCCU_EXCHANGE,// ���ڸı�㲥�ߵ�״̬
	TCCU_MUTISCASTING,// ���ڹ㲥
}EccuTModelOutputState;

typedef struct _type_central_control_trans_model// ���봫��ģ��
{
	uint64_t tarker_id;
	TOutChannel out_ch; // ���ͨ������Ϊ���������
	struct list_head list; 
}TccuTModel,*T_pccuTModel;

typedef struct _type_central_control_trans_pro// �����㲥����Ϣ
{
	EccuTModelOutputState model_out_ch_state;
	TOutChannel *p_current_out_ch;
	T_pccuTModel p_current_model;
}tcentral_control_trans_pro;

//************************************//

#endif
