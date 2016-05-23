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
#include "connector_subject.h"

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

extern int central_control_transmit_unit_init( const uint8_t *frame, int pos, size_t frame_len, const desc_pdblist desc_node, const uint64_t endtity_id );
extern void central_control_transmit_unit_update( subject_data_elem reflesh_data );// ���»�����յ�Ԫģ�������״̬
extern void central_control_transmit_unit_model_pro_init( void );

//************************************//

#endif
