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

typedef enum // 签到
{
	SIGN_IN_OVER = 0,
	SIGN_IN_ON_TIME,
	SIGN_IN_BE_LATE,
}enum_signstate;

/*{@终端表决标志*/
#define TVOTE_KEY_MARK	0x1F // 键的掩码
#define TVOTE_EN         		0x80 // 投票使能位
#define TVOTE_SET_FLAG   	0x40 // 未签到标志
#define TWAIT_VOTE_FLAG  	0x20 // 签到允许投票标志

#define TVOTE_KEY1_ENABLE 0x01
#define TVOTE_KEY2_ENABLE 0x02
#define TVOTE_KEY3_ENABLE 0x04
#define TVOTE_KEY4_ENABLE 0x08
#define TVOTE_KEY5_ENABLE 0x10
#define TVOTE_KEY6_ENABLE 0x20
#define TVOTE_KEY7_ENABLE 0x40

typedef enum _enum_vote_state_pro
{
	NO_VOTE=0, 
	VOTE_SET, // 开始投票
	VOTE_SET_OVER // 投票完成
}evote_state_pro;

/*终端表决标志@}*/

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
int terminal_end_sign( uint16_t cmd, void *data, uint32_t data_len );
int terminal_end_vote( uint16_t cmd, void *data, uint32_t data_len );

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
void terminal_start_sign_in( tcmpt_begin_sign sign_flag );
void terminal_begin_vote( tcmp_vote_start vote_start_flag,  uint8_t* sign_flag );
void terminal_vote_state_set( uint16_t addr );
void terminal_sign_in_special_event( tmnl_pdblist sign_node ); // 终端特殊事件-签到
void termianl_vote_enable_func_handle( tmnl_pdblist sign_node );
void terminal_state_all_copy_from_common( void );
void terminal_broadcast_end_vote_result( uint16_t addr ); // 根据终端的2 3 4键统计结果
void terminal_vote( uint16_t addr, uint8_t key_num, uint8_t key_value, uint8_t tmnl_state, const uint8_t recvdata );
void terminal_key_speak( uint16_t addr, uint8_t key_num, uint8_t key_value, uint8_t tmnl_state, const uint8_t recvdata );
bool terminal_key_action_value_judge_can_save( uint8_t key_num,  tmnl_pdblist vote_node );
void terminal_key_action_host_special_num2_reply( const uint8_t recvdata, uint8_t key_down, uint8_t key_up, uint16_t key_led, uint8_t lcd_num, tmnl_pdblist node );
void terminal_key_action_host_special_num1_reply( const uint8_t recvdata, uint8_t mic_state, tmnl_pdblist node );
void terminal_key_action_host_common_reply( const uint8_t recvdata, tmnl_pdblist node );
void terminal_chman_control_start_sign_in( uint8_t sign_type, uint8_t timeouts );
void terminal_chman_control_begin_vote(  uint8_t vote_type, bool key_effective, uint8_t* sign_flag );
void terminal_key_action_chman_interpose( uint16_t addr, uint8_t key_num, uint8_t key_value, uint8_t tmnl_state, const uint8_t recvdata );
void terminal_chairman_interpose( uint16_t addr, bool key_down, tmnl_pdblist chman_node, const uint8_t recvdata );
int terminal_key_discuccess( uint16_t addr, uint8_t key_num, uint8_t key_value, uint8_t tmnl_state, uint8_t recv_msg );
bool terminal_key_speak_proccess( tmnl_pdblist dis_node, bool key_down, uint8_t recv_msg );
int terminal_chairman_apply_reply( uint8_t tmnl_type, uint16_t addr, uint8_t key_num, uint8_t key_value, uint8_t tmnl_state, uint8_t recv_msg );
void terminal_free_disccuss_mode_pro( bool key_down, uint8_t limit_time,tmnl_pdblist speak_node, uint8_t recv_msg );
bool terminal_limit_disccuss_mode_pro( bool key_down, uint8_t limit_time,tmnl_pdblist speak_node, uint8_t recv_msg );
bool terminal_fifo_disccuss_mode_pro( bool key_down, uint8_t limit_time,tmnl_pdblist speak_node, uint8_t recv_msg );
bool terminal_apply_disccuss_mode_pro( bool key_down, uint8_t limit_time,tmnl_pdblist speak_node, uint8_t recv_msg );
void terminal_type_save( uint16_t address, uint8_t tmnl_type, bool is_chman );
void terminal_trasmint_message( uint16_t address, uint8_t *p_data, uint16_t msg_len );

/*@}*/

/*{@*/
tmnl_pdblist terminal_system_dblist_except_free( void );
void terminal_open_addr_file_wt_wb( void );
void terminal_system_dblist_destroy( void );

/*@}*/

#endif

