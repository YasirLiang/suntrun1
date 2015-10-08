#ifndef __TERMINAL_PRO_H__
#define __TERMINAL_PRO_H__

#include "jdksavdecc_world.h"

#define SYSTEM_TMNL_MAX_NUM 450	// ϵͳ���ն���������

typedef struct tterminal_addr_list	// �ն˷����ַ
{
	uint16_t addr;
	uint8_t  tmn_type;
}terminal_address_list;

typedef struct _tterminal_address_list_pro // �ն˷����ַ����
{
	uint16_t addr_start;
	uint16_t index;
	uint16_t renew_flag;
}terminal_address_list_pro;

typedef  struct _tterminal_state	// �ն˵�״̬��һ���ն˶�Ӧһ��״̬
{
	uint8_t device_type;		// �豸����
	uint8_t mic_state;			// ��˷�״̬
	bool is_rgst;				// �ն˱�������, �ڷ����ַ��ɺ�
	uint8_t sys_state;			// �ն���ϵͳ�е�״̬
	uint8_t sign_state;			// �ն˵�ǩ��״̬
	uint8_t vote_state;			// ͶƱ״̬
}terminal_state; 

typedef  struct _tterminal
{
	uint64_t entity_id;			// ʵ��ID
	uint16_t addr;				// �ն˵�ַ
	terminal_state tmnl_status;	// �ն˵�״̬ 
}conference_terminal_device; 

typedef  struct tmnl_list_node		// �ն�����ڵ� 
{
	conference_terminal_device tmnl_dev;
	struct tmnl_list_node *next,*prior;
}tmnl_dblist, *tmnl_pdblist; 

#endif

