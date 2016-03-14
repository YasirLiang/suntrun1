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

typedef struct _elem_data
{
	uint64_t listener_id;
	uint64_t tarker_id;
	uint16_t listener_index;
	uint16_t tarker_index;
	bool connect_flag;// true :cnnt success :false disconnect success!
}subject_data_elem;

typedef struct 
{
	void (*update)( subject_data_elem data );
}observer_t;

typedef struct _subject_t
{
	struct _elem_data sub_data;
	int observer_num;
	observer_t observer_list[OBSERVER_LIST_MAX_NUM];
}subject_t;

#endif