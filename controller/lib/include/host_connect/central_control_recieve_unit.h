/*
**central_control_recieve_unit.h
**9-3-2016
**
*/

// ****************************
// file built 2016-3-9
// ��������ϵͳ���ӱ�������
// ���Ŀ��Ƶ�Ԫ����ģ��
// ****************************

#ifndef __CENTRAL_CONTROL_RECIEVE_UNIT_H__
#define __CENTRAL_CONTROL_RECIEVE_UNIT_H__

#include "endstation_connection.h"

#ifdef __DEBUG__
#define __CCU_RECV_DEBUG__
#endif

//************************************

#define CHANNEL_MUX_NUM 6 // ����ͨ����
#define CCU_TR_MODEL_MAX_NUM 3 // ������Ƶ�Ԫ���շ���ģ����������
#define CCU_APIECE_TR_MODEL_CHANNEL_MAX_NUM 4 //������Ƶ�Ԫÿ�����շ���ģ�������ͨ���� 
#define CCU_TR_MODEL_CHANNEL_MAX_NUM \ 
	(CCU_TR_MODEL_MAX_NUM*CCU_APIECE_TR_MODEL_CHANNEL_MAX_NUM)// ������Ƶ�Ԫ���շ���ģ���ͨ������
#define CCU_TR_MODEL_NAME "DCS6000_MODEL0"// ������ƽ��շ���ģ������
#define CCU_R_MODEL_NAME "DCS6000_MODEL1" // ������ƽ���ģ�������
#define PER_CCU_CONNECT_MAX_NUM 2 // ÿ��ccu����ģ�����������

//*****************************
#define CCU_TR_MODEL_RECIEVE_ENABLE // ʹ��������Ƶ�Ԫ���շ���ģ��Ľ��չ���
#ifndef CCU_TR_MODEL_RECIEVE_ENABLE
#define CCU_TR_MODEL_RECIEVE_DISABLE// ��ʹ��������Ƶ�Ԫ���շ���ģ��Ľ��չ���
#endif
//*****************************////

enum recieve_model_state// ����ģ��ͨ����״̬--�ѳ�ʼ��δ����-�ѳ�ʼ������-δ��ʼ��
{
	CCU_RECIEVE_MODEL_UNINIT = 0,// δ��ʼ���Ҳ����ã��ǽڵ�������õ�״̬
	CCU_RECIEVE_MODEL_OK,// �ѳ�ʼ��δ����
	CCU_RECIEVE_MODEL_ALL_CHANNEL_INIT,
};

typedef struct _type_central_control_recieve_model// ����ģ��
{
	uint64_t entity_id;
	solid_pdblist solid_pnode; // ָ��adp����ڵ�
	struct endpoint_decriptor* desc_pnode;// ָ��desc����ڵ�
	TInChannel connect_channel_head;// ģ�������ӱ�����ͨ��
	TInChannel unconnect_channel_head;// ģ��δ���ӱ�����ͨ��
	enum recieve_model_state model_state;
	uint8_t channel_num;// ͨ����
	uint8_t chanel_connect_num;// ͨ����������
	pthread_mutex_t RModel_mutex;// �ڵ���
}TccuRModel;

enum channel_list_pro_flags
{
	CH_ALLOT_PRIMED = 0,// Ԥ����
	CH_ALLOT_HANDLING, // ���ڴ���
	CH_ALLOT_FINISH// �������
};

typedef struct _type_channel_alloction_proccess// ͨ�����䴦��ṹ
{
	TInChannel *p_current_input_channel;// ָ��ͨ�����е�ǰ���Ա������ģ��,��ΪNULL�����ʾ�޿���ͨ��
	uint8_t elem_num;// ͨ���ĸ���
	uint8_t cnnt_num;// �����Ӹ���
	channel_list_pro_flags pro_eflags;// ������̲���
}tchannel_allot_pro;

// *****************************************//

#endif
