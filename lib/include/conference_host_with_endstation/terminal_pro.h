#ifndef __TERMINAL_PRO_H__
#define __TERMINAL_PRO_H__

#include "endstation_connection.h"
#include "jdksavdecc_world.h"
#include "conference.h"
#include "terminal_common.h"
#include "profile_system.h"
#include "upper_computer_common.h"

#define ADDRESS_FILE "address.dat"	// 终端地址信息存放的文件
//#define ADDRESS_FILE "profile/address.dat"	// 终端地址信息存放的文件

#define MAX_LIMIT_SPK_NUM 4
#define MAX_LIMIT_APPLY_NUM 10

extern tmnl_pdblist dev_terminal_list_guard; // 终端链表表头结点

typedef enum
{
	RGST_IDLE=0,
	RGST_WAIT,
	RGST_QUERY,
}register_state;	// 报到状态

typedef struct _type_tmnl_register_pro
{
  register_state rgs_state;
  uint16_t tmn_total;
  uint16_t tmn_rgsted;
}ttmnl_register_proccess;

typedef enum _enum_apply_pro
{
	REFUSE_APPLY=0,
	NEXT_APPLY,
	APPROVE_APPLY
}enum_apply_pro;

typedef struct _tsystem_discuccess
{
	ttmnl_discuss_mode edis_mode;	// 讨论模式
	uint8_t    limit_num;     			//受限的发言人上限
	uint8_t    speak_limit_num;		//受限的发言人数
	uint8_t    currect_first_index;		//申请中优先发言的索引
	uint8_t    apply_limit; 			// 申请发言的限制人数
	uint8_t    apply_num;			// 申请人数
	uint16_t   speak_addr_list[MAX_LIMIT_SPK_NUM];	// 发言地址表
	uint16_t   apply_addr_list[MAX_LIMIT_APPLY_NUM];	 // 申请地址表
}tsys_discuss_pro;

typedef enum
{
	NO_VOTE=0,
	VOTE_SET,
	VOTE_SET_OVER,
}evote_flag;

typedef struct _type_vote_proccess
{
  evote_flag flag;
  uint16_t index;
}tvote_pro;

typedef struct
{
	bool is_int;
	uint16_t 	chmaddr;
}tchairman_control_in;

int init_terminal_discuss_param( void );
int init_terminal_address_list_from_file( void );
void init_terminal_address_list( void );
inline void init_terminal_allot_address( void );
inline void init_terminal_device_double_list();
void 	test_interface_terminal_address_list_write_file( FILE** fd );
void print_out_terminal_addr_infomation( terminal_address_list* p, int num);
void init_terminal_proccess_system( void );
bool terminal_register( uint16_t address, uint8_t dev_type, tmnl_pdblist p_tmnl_station );
uint16_t find_new_apply_addr( terminal_address_list_pro* p_gallot, terminal_address_list* p_gaddr_list, uint16_t* new_index);

/*{@命令处理函数，处理协议的命令数据或系统的命令*/
int terminal_func_allot_address( uint16_t cmd, void *data, uint32_t data_len );
int terminal_func_key_action( uint16_t cmd, void *data, uint32_t data_len );
int terminal_func_chairman_control( uint16_t cmd, void *data, uint32_t data_len );
int terminal_func_send_main_state( uint16_t cmd, void *data, uint32_t data_len );
int terminal_func_cmd_event( uint16_t cmd, void *data, uint32_t data_len );
int terminal_mic_auto_close( uint16_t cmd, void *data, uint32_t data_len );
int terminal_main_state_send( uint16_t cmd, void *data, uint32_t data_len );
int terminal_lcd_display_num_send( uint16_t addr, uint8_t display_opt, uint8_t display_num );
int terminal_pause_vote( uint16_t cmd, void *data, uint32_t data_len );
int terminal_regain_vote( uint16_t cmd, void *data, uint32_t data_len );
int terminal_system_discuss_mode_set( uint16_t cmd, void *data, uint32_t data_len );
int terminal_speak_limit_num_set( uint16_t cmd, void *data, uint32_t data_len );// 处理函数有待完善(11/4)
int terminal_apply_limit_num_set( uint16_t cmd, void *data, uint32_t data_len );
int terminal_limit_speak_time_set( uint16_t cmd, void *data, uint32_t data_len );

/*@}*/

/*{@会议流程处理函数*/
void terminal_remove_unregitster( void ); // 这里没有清除终端地址文件以及内存终端列表里相应的内容
void terminal_mic_state_set( uint8_t mic_status, uint16_t addr, uint64_t tarker_id, bool is_report_cmpt, tmnl_pdblist tmnl_node );
int terminal_mic_speak_limit_time_manager_event( void );
int terminal_start_discuss( bool mic_flag );
void terminal_state_set_base_type( uint16_t addr, tmnl_state_set tmnl_state );
void terminal_chairman_apply_type_set( uint16_t addr );
bool terminal_led_set_save( uint16_t addr, uint8_t led_id, uint8_t  led_state );
void fterminal_led_set_send( uint16_t addr );
int terminal_upper_computer_speak_proccess( tcmpt_data_mic_switch mic_flag );
extern bool terminal_read_profile_file( thost_system_set *set_sys );
void terminal_free_disccuss_mode_cmpt_pro( uint8_t mic_flag, uint8_t limit_time, tmnl_pdblist speak_node );
bool terminal_limit_disccuss_mode_cmpt_pro( uint8_t mic_flag, uint8_t limit_time, tmnl_pdblist speak_node );
bool terminal_fifo_disccuss_mode_cmpt_pro( uint8_t mic_flag, uint8_t limit_time, tmnl_pdblist speak_node );
bool terminal_apply_disccuss_mode_cmpt_pro( uint8_t mic_flag, uint8_t limit_time, tmnl_pdblist speak_node );
bool addr_queue_delete_by_index( uint16_t *addr_queue, uint8_t *queue_len, uint8_t index );
bool addr_queue_delect_by_value( uint16_t *addr_queue, uint8_t *queue_len, uint16_t value);
bool addr_queue_find_by_value( uint16_t *addr_queue, uint8_t queue_len, uint16_t value, uint8_t *index);
tmnl_pdblist found_terminal_dblist_node_by_addr( uint16_t addr );
void terminal_select_apply( uint16_t addr ); // 使选择的申请人是首位申请人
bool terminal_examine_apply( enum_apply_pro apply_value );
void terminal_type_set( tcmpt_data_meeting_authority tmnl_type );
void terminal_chairman_excute_set( uint16_t addr, bool is_set_excute );
void terminal_vip_type_set( uint16_t addr, bool is_set_vip );
int terminal_type_save_to_address_profile( uint16_t addr, uint16_t tmnl_type );
void terminal_send_upper_message( uint8_t *data_msg, uint16_t addr, uint16_t msg_len );
void terminal_tablet_stands_manager( tcmpt_table_card *table_card, uint16_t addr, uint16_t contex_len );// 桌牌管理
void terminal_chairman_apply_type_clear( uint16_t addr );
int terminal_socroll_synch(void );


/*@}*/

/*{@*/
tmnl_pdblist terminal_system_dblist_except_free( void );
void terminal_open_addr_file_wt_wb( void );
/*@}*/

#endif

