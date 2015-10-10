#ifndef __TERMINAL_PRO_H__
#define __TERMINAL_PRO_H__

#include "jdksavdecc_world.h"
#include "conference.h"

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


typedef struct _tterminal_state_set // �ն�״̬������
{
	uint8_t :1;
	uint8_t one_off:1;
	uint8_t sign_stype:1;
	uint8_t auto_close:1;
	uint8_t sys:4;
	
	uint8_t :3;
	uint8_t keydown:5;

	uint8_t :3;
	uint8_t keyup:5;
	
	uint8_t MicClose:1;
	uint8_t :2;
	uint8_t VoteType:5;
}tmnl_state_set;

typedef struct _tterminal_led_show_stype // �����ն� led ��ʾ��ʽ
{
	uint8_t stop_time:4;
	uint8_t speed_roll:4;
	uint8_t page_show_state:3;
	uint8_t bright_lv:4;
	uint8_t blink:1;
}tmnl_led_state_show_set;

typedef struct _tterminal_vote_result	// ͶƱ������
{
	uint16_t total;	// �μӱ������Ʊ��
	uint16_t abs;	// ��ȨƱ��
	uint16_t neg;	// ���Ե�Ʊ��
	uint16_t aff;	// �޳ɵ�Ʊ��
}tmnl_vote_result;

typedef struct _tterminal_limit_spk_time
{
	uint8_t :2;
	uint8_t limit_time:6;
}tmnl_limit_spk_time;

typedef struct _ttmerminal_main_state_send // ��������״̬
{
	uint16_t unit;				// �����������ն�����
	
	uint8_t conference_stype:4;
	uint8_t :2;
	uint8_t chm_first:1;
	uint8_t camera_follow:1;		// ģʽ����
	
	uint8_t limit;				// ���������������
	uint8_t apply_set;			// ������������
	uint8_t spk_num;			// ��������
	uint8_t apply;				// ��������
}tmnl_main_state_send;

typedef struct _tterminal_send_end_lcd_display
{
	uint8_t opt;	// ����ָʾ 
	uint8_t num;  // ����
}tmnl_send_end_lcd_display;

uint16_t ternminal_send( void *buf, uint16_t length, uint64_t uint64_target_id );
void terminal_recv_message_pro( struct terminal_deal_frame *conference_frame );

void init_terminal_address_list( void );


#endif

