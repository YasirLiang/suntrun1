#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "jdksavdecc_adp.h"
#include "host_time.h"

enum entity_connect
{
	DISCONNECT,
	CONNECT
};

struct entity_timeout
{
	bool running;					// ����
	bool elapsed;					// ��ʱ��־
	uint32_t count_time;			// ��ʱʱ��
	uint32_t start_time;			// ��ʼʱ��
};

struct entity
{
	struct jdksavdecc_eui64 entity_model_id;
	uint8_t keep_value1;			// ����ֵ1�������ֽڶ���
	bool connect_flag;				// ʵ���������ԣ�
	struct entity_timeout time;		// ʵ��ĳ�ʱ��Ϣ
	uint32_t keep_value2;			// ����ֵ2�������ֽڶ���
	uint32_t entity_index;			// ʵ������(������ϵͳ����, �Ҵ� 0 ��ʼ)
	uint32_t available_index;
	uint64_t entity_id;				// ʵ��ID
	//struct jdksavdecc_adpdu adpdu;
};

typedef struct entities				// ʵ��ڵ㣬������ϵͳ��
{
	struct entity solid;
	struct entities *prior, *next;
}solid_dblist,*solid_pdblist;

#endif

