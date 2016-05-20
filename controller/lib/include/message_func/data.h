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

//系统命令宏定义
#define SYS_PRESET_ADDR     0x01
#define SYS_GET_PRESET        0x02

enum menu_command//菜单相关命令定义
{
	MENU_AUTO_CLOSE_CMD,
	MENU_DISC_MODE_SET_CMD,
	MENU_SPK_LIMIT_NUM_SET,
	MUNU_APPLY_LIMIT_NUM_SET,

	MENU_MUSIC_EN_SET,// 2015-12-09 add include all after
	MENU_CMR_SAVE_PRESET,
	MENU_CMR_SEL_CMR,// 当前摄像头
	MENU_CMR_CTRL_LR,
	MENU_CMR_CTRL_UD,
	MENU_CMR_CTRL_FOUCE,
	MENU_CMR_CTRL_ZOOM,
	MENU_CMR_CTRL_APERT, // 光圈控制
	MENU_CMR_CTRL_ALIGN, // 控制对齐
	MENU_CMR_FULL_VIEW,
	MENU_CMR_CLR_PRESET, // 预置点清除
	MENU_PRIOR_EN_SET, // 主席优先
	MENU_TERMINAL_SYS_REGISTER, // 系统注册终端
	MENU_TEMP_CLOSE_SET, // 临时关闭
	MENU_CMR_TRACK,// 摄像跟踪
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

