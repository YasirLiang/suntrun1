/*
**connector_subject.c
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

#include "connector_subject.h"

subject_t gconnector_subjector;

void init_connector_subjector( void )
{
	init_subject( &gconnector_subjector );
}

// ***************************************************
// ���۲��߽ӿ�
int attach_observer( subject_t *subject, observer_t *observer )
{

	assert( subject && observer );
	if( subject == NULL || observer == NULL )
		return -1;

	if( subject->observer_num < OBSERVER_LIST_MAX_NUM )
	{
		subject->observer_list[subject->observer_num] = observer;
		subject->observer_num++;
	}

	return 0;
}

// return -1 is Err or not found observer in the list
int delect_observer( subject_t *subject, observer_t *observer )
{	
	assert( subject && observer );
	if( subject == NULL || observer == NULL )
		return -1;

	int i = 0;
	for( i = 0; i < subject->observer_num; i++ )
	{
		if( subject->observer_list[i].update == observer.update )
		{
			// �����Ԫ��ǰ��һ��Ԫ��
			int j = 0;
			subject->observer_list[i].update = NULL;
			for( j = i; j < (subject->observer_num - i - 1); j++ )
			{
				subject->observer_list[j].update = subject->observer_list[j+1].update;
			}
			
			subject->observer_num--;
			return 0;
		}
	}
	
	return -1;
}

int set_subject_data( subject_data_elem data, subject_t *subject )
{
	assert( subject );
	if( subject == NULL )
		return -1;
	
	subject->sub_data.connect_flag = data.connect_flag;
	subject->sub_data.listener_id = data.listener_id;
	subject->sub_data.listener_index = data.listener_index;
	subject->sub_data.tarker_id = data.tarker_id;
	subject->sub_data.tarker_index = data.tarker_index;

	return 0;
}

void notify_observer( subject_t *subject )
{
	assert( subject );
	if( subject == NULL )
		return -1;
	
	int i = 0;
	for( i = 0; i < subject->observer_num; i++ )
	{
		subject.observer_list[i]->update( subject->sub_data );
	}
}

void init_subject( subject_t *subject )
{
	assert( subject );
	if( subject == NULL )
		return;

	memset( subject, 0, sizeof(subject_t));
}

// ���۲��߽ӿ�
// ***************************************************

// ***************************************************
// �۲��߽ӿ�

// �󶨽ӿڵ�����
void init_observer( observer_t *observer, void (*update)(subject_data_elem) )
{
	assert( observer );
	if( observer == NULL )
		return;

	observer->update = update;
}

// �۲��߽ӿ�
// ***************************************************

