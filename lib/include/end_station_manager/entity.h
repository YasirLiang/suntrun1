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
	bool connect_flag;				// ʵ���������ԣ�
	struct entity_timeout time;		// ʵ��ĳ�ʱ��Ϣ
	uint32_t entity_index;			// ʵ������(������ϵͳ����, �Ҵ� 0 ��ʼ)
	uint64_t entity_id;				// ʵ��ID
	uint32_t available_index;
	struct jdksavdecc_eui64 entity_model_id;
	//struct jdksavdecc_adpdu adpdu;
};

typedef struct entities				// ʵ��ڵ㣬������ϵͳ��
{
	struct entity solid;
	struct entities *prior, *next;
}solid_dblist,*solid_pdblist;

#endif

