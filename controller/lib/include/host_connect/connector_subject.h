/*
**connector_subject.h
**10-3-2016
**
*/

// *******************************************************************
// file built 2016-3-10
// ��������ϵͳ���ӱ�������
// ϵͳ�����߱��۲���
// ϵͳ�еĴ��ڹ۲���:�����Ľ���ģ��;ϵͳ�Ļ��鵥Ԫ����ģ�飨�����������ģ�飩;
//				�����Ĺ㲥�����ͣ�ģ��;����ϵͳ�����ӹ���ģ�飻���鵥Ԫ����˷�ģ��
// ********************************************************************

#ifndef __CONNECT_SUBJECT_H__
#define __CONNECT_SUBJECT_H__

#define OBSERVER_LIST_MAX_NUM 20

#include "jdksavdecc_world.h"

typedef struct _type_command
{
	uint16_t data_type;// ��������
	uint16_t msg_type;// ��Ϣ����
	uint16_t desc; // ����ɼ�aecp \app�ȵ����� ,desc ����
	uint16_t desc_index;
	int msg_resp_status;// ����ķ���ֵ״̬;-1Ϊ�������ݳ�ʱ״̬
}tsubject_control_command;

typedef struct _elem_data
{
	uint64_t listener_id;
	uint64_t tarker_id;// ����Ӧ������rx_stateʱ����ʱΪ��ID
	uint16_t listener_index;
	uint16_t tarker_index;
	bool connect_flag;// true :cnnt success :false disconnect success! ǰ��Ľṹ����
	tsubject_control_command ctrl_msg;// �����������Ľṹ����
}subject_data_elem;

typedef struct 
{
	void (*update)( subject_data_elem data );
}observer_t;

typedef struct _subject_t
{
	struct _elem_data sub_data;
	int observer_num;
	observer_t *observer_list[OBSERVER_LIST_MAX_NUM];
}subject_t;

extern subject_t gconnector_subjector;

void init_connector_subjector( void );

// **************************************************
int attach_observer( subject_t *subject, observer_t *observer );
int delect_observer( subject_t *subject, observer_t *observer );
int set_subject_data( subject_data_elem data, subject_t *subject );
void notify_observer( subject_t *subject );
void init_subject( subject_t *subject );
void init_observer( observer_t *observer, void (*update)(subject_data_elem) );

// *****************************************************//

#endif
