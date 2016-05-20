#ifndef __DATA_H__
#define __DATA_H__

#include "jdksavdecc_world.h"
#include "func_proccess.h"

#define MAX_FUNC_LINK_ITEMS 53
#define MAX_PROCCESS_FUNC 53

#define TERMINAL_USE   (1<<0)
#define MENUMENT_USE  (1<<1)
#define SYSTEM_USE   (1<<2)
#define COMPUTER_USE   (1<<3)

//ϵͳ����궨��
#define SYS_PRESET_ADDR     0x01
#define SYS_GET_PRESET        0x02

enum menu_command//�˵���������
{
	MENU_AUTO_CLOSE_CMD,
	MENU_DISC_MODE_SET_CMD,
	MENU_SPK_LIMIT_NUM_SET,
	MUNU_APPLY_LIMIT_NUM_SET,

	MENU_MUSIC_EN_SET,// 2015-12-09 add include all after
	MENU_CMR_SAVE_PRESET,
	MENU_CMR_SEL_CMR,// ��ǰ����ͷ
	MENU_CMR_CTRL_LR,
	MENU_CMR_CTRL_UD,
	MENU_CMR_CTRL_FOUCE,
	MENU_CMR_CTRL_ZOOM,
	MENU_CMR_CTRL_APERT, // ��Ȧ����
	MENU_CMR_CTRL_ALIGN, // ���ƶ���
	MENU_CMR_FULL_VIEW,
	MENU_CMR_CLR_PRESET, // Ԥ�õ����
	MENU_PRIOR_EN_SET, // ��ϯ����
	MENU_TERMINAL_SYS_REGISTER, // ϵͳע���ն�
	MENU_TEMP_CLOSE_SET, // ��ʱ�ر�
	MENU_CMR_TRACK,// �������
	MENU_FUNC_NUM
};

/*{@*/
enum enum_db_call_table_owner
{
	MUTICASTOR = 0,
	ERROR_OWNER = 0xffffff
};

enum enum_db_call_func_link
{
	MUTICASTOR_GET_HOST_MUTICAST = 0,
	DB_CALLBACK_FUNC_NUM
};

struct str_db_callback
{
	enum enum_db_call_table_owner owner;
	uint32_t func_link;
	int (*func_proccess)( void* );
};
/*@}*/

extern const proccess_func_items proccess_func_link_tables[MAX_FUNC_LINK_ITEMS];
extern const func_link_items func_link_tables[MAX_FUNC_LINK_ITEMS];
extern const struct str_db_callback gdb_callback_table[DB_CALLBACK_FUNC_NUM+1];

#endif

