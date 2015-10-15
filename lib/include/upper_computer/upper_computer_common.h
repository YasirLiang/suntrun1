#ifndef __UPPER_COMPUTER_COMMON_H__
#define __UPPER_COMPUTER_COMMON_H__

#include "jdksavdecc_world.h"

/*{@ discuss parameter */
enum
{
	UPPER_CMPT_DISCUSS_FREE_MODE,
	UPPER_CMPT_DISCUSS_PPT_MODE,
	UPPER_CMPT_DISCUSS_LIMIT_MODE,
	UPPER_CMPT_DISCUSS_FIFO_MODE,
	UPPER_CMPT_DISCUSS_APPLY_MODE
};

typedef struct _type_upper_computer_discuss_parameter
{
	uint8_t discuss_mode:3;// mode set
	uint8_t :2;
	uint8_t auto_close:1;
	uint8_t chair_music:1;
	uint8_t chairman_first:1; 

	uint8_t limit_speak_num;
	uint8_t limit_apply_num;

	uint8_t limit_speak_time:6;
	uint8_t limit_vip_time:1;
	uint8_t limit_chm_time:1;
}tcmpt_discuss_parameter;
/*@}*/

#define UPPER_COMPUTER_MIC_CLOSE 0
#define UPPER_COMPUTER_MIC_OPEN 1

struct application_common_address
{
	uint8_t low_addr;
	uint8_t high_addr;
};
/*{@ mic state*/
typedef struct _type_upper_computer_data_mic_switch
{
	struct application_common_address addr;
	uint8_t switch_flag;
}tcmpt_data_mic_switch;
/*@}*/

/*{@*/
typedef struct _type_upper_computer_data_mic_status
{
	struct application_common_address addr;
	uint8_t switch_flag;
	// ?
}tcmpt_data_mic_status;
/*@}*/

/*{@*/
typedef struct _type_upper_computer_data_choose_apply_man
{
	struct application_common_address addr;
}tcmpt_data_choose_apply_man;
/*@}*/

/*{@*/
typedef struct _type_upper_computer_data_examine_apply
{
	uint8_t apply_flag;
}tcmpt_data_examine_apply;
/*@}*/

/*{@*/
typedef struct _type_upper_computer_data_meeting_authority
{
	struct application_common_address addr;
	uint8_t identity;//身份代表
}tcmpt_data_meeting_authority;
/*@}*/

/*{@*/
#define UPPER_MSG_MAX_LENGTH 64
typedef struct _type_upper_computer_data_message
{
	struct application_common_address addr;
	uint8_t msg_type;	// 短信息类型 见terminal_command.h
	uint8_t msg_buf[UPPER_MSG_MAX_LENGTH];//身份代表
}tcmpt_data_message;
/*@}*/

/*{@*/
enum // table card cotent type
{
	UPPER_CMPT_TABLE_CARD_MANAGE_SYNC_ROLL,
	UPPER_CMPT_TABLE_CARD_MANAGE_DISPLAY_SET
};

#define COTENT_DISPLAY_NOMAL 0
#define COTENT_DISPLAY_FLASH 1

enum // display cotent type 
{
	COTENT_ROLL_PAGE,
	COTENT_LEFT_ROLL_PAGE,
	COTENT_RIGHT_ROLL_PAGE,
	COTENT_UP_ROLL_PAGE,
	COTENT_DOWN_ROLL_PAGE
};

typedef struct _type_cotent_page_roll_type// 页面滚动的方式
{
	uint8_t roll_type:3;
	uint8_t bright_lv:4;
	uint8_t flash_flag:1;
	uint8_t stop_time:4;
	uint8_t roll_speed:4;
}tcmpt_cotent_page;

typedef struct _type_upper_computer_table_card
{
	struct application_common_address addr;
	uint8_t msg_type;	// 短信息类型
	uint8_t msg_buf[UPPER_MSG_MAX_LENGTH];//身份代表
}tcmpt_table_card; 
/*@}*/

/*{@ sign begin*/
typedef struct _type_begin_sign_in
{
	uint8_t sign_type;
	uint8_t retroactive_timeouts; // 补签的超时时间，单位是分钟
}tcmpt_begin_sign;
/*@}*/

/*{@ sign situation*/
#define ENDSTATIONS_UNSIGN 0
#define ENDSTATIONS_SIGNED 1
#define ENDSTATIONS_RETROACTIVE_SIGN 1
typedef struct _type_sign_situation
{
	struct application_common_address addr;
	uint8_t sign_situation;
}tcmpt_sign_situation;
/*@}*/

/*{@*/
typedef struct _type_end_allot_address // 终端分配应用地址
{
	uint8_t allot_type;
}tcmpt_end_allot_addr;
/*@}*/

/*{@*/
enum // terminal type
{
	ENDSTATION_TYPE_REPRESENT,
	ENDSTATION_TYPE_VIP,
	ENDSTATION_TYPE_COMMAN_CHAIRMAN,
	ENDSTATION_TYPE_EXCUTE_CHAIRMAN
};

typedef struct _type_end_register_situation // 终端报到情况
{
	struct application_common_address addr;
	uint8_t device_type; // 设备类型
	uint8_t end_type:2; // 终端类型
	uint8_t :5;
	uint8_t register_flags:1;
}tcmpt_end_register;
/*@}*/

/*{@*/
typedef struct _type_current_camara
{
	uint8_t camara_num;
}tcmp_current_camara;

/*@}*/

/*{@ wait allocation 待定位的终端应用地址*/ 
typedef struct _type_wait_allocation_address
{
	struct application_common_address addr;
}tcmp_wait_allocation;
/*@}*/

/*{@ camara controller*/
#define UNEFFECTIVE_CAMARA_FLAGS 0
#define EFFECTIVE_CAMARA_FLAGS 1

typedef struct _type_camara_controll
{
	uint8_t close_focus_flags:1; 			// 近距离聚焦
	uint8_t aperture_amplification_flags:1; 	// 光圈放大
	uint8_t aperture_shrinks_flags:1;		// 光圈缩小
	uint8_t :5;

	uint8_t :1;
	uint8_t right_flags:1;
	uint8_t left_flags:1;
	uint8_t up_flags:1;
	uint8_t down_flags:1;
	uint8_t close_object:1;				// 接近物体
	uint8_t remote_object:1;				// 远离物体
	uint8_t remote_focus_flags:1;			// 远距离聚焦

	uint8_t level_speed;
	uint8_t vertical_speed;
}tcmp_camara_ctl;
/*@}*/

/*{@*/
enum
{
	SAVE_PRESET_BIT, // 预置位保存
	ENTER_PRESET_BIT, // 进入预置位设置
	EXIT_PRESET_BIT, // 退出预置位设置
	SAVE_CAMARA_ALLOCATION_ADJUST // 摄像定位调整保存
};
typedef struct _type_camara_bit_preset
{
	uint8_t camara_preset_flags;
}tcmp_camara_bit_preset;
/*@}*/

/*{@*/
#define CAMARA_LOCK 0
#define CAMARA_UNLOCK 1
typedef struct _type_camara_lock
{
	uint8_t camara_lock_flags; 
}tcmp_camara_lock;
/*@}*/

/*{@*/
#define CAMARA_PANORAMA 0 // 全景
#define CAMARA_TRACE 0 // 追踪
typedef struct _type_camara_output
{
	uint8_t camara_output_1; 
	uint8_t camara_output_2;
}tcmp_camara_output;
/*@}*/

/*{@ begin vote */
#define NOMAL_SIGN_FLAGS 0
#define UNSIGN_FLAGS 1
typedef struct _type_vote_start
{
	uint8_t vote_type:4;  // 见terminal_common.h 的terminal VOTE MODE
	uint8_t key_effective:1;
	uint8_t :3;
}tcmp_vote_start;
/*@}*/

/*{@*/
typedef struct _type_vote_result
{
	struct application_common_address addr;
	uint8_t key_value:5; // 各个位对应相应终端按键的值
	uint8_t :3;
}tcmp_vote_result;

/*@}*/

/*{@*/
#define MESSAGE_DATA_MAX_LENGHT 64
#define HOST_REPORT_CARD_COTENT 0 // 上报卡的内容
#define HOST_REPORT_PLUG_CARD_ACTION 1 // 插卡动作上报
struct terminal_message_card_common
{
	uint8_t msg_type; // 终端短信息类型
	uint8_t check_in_cotent;
	uint8_t cotent_data_len;
	uint16_t card_id;
};

struct terminal_card_message
{
	struct terminal_message_card_common card_msg_head;
	uint8_t cotent_buf[MESSAGE_DATA_MAX_LENGHT];
};

typedef struct _type_report_terminal_message
{
	struct application_common_address addr;
	struct terminal_card_message msg_cotent;
}tcmp_report_terminal_message;

/*@}*/

/*{@set camara switch*/
#define UPPER_SWICH_COMMAN_COTENT_MAX_LEN 24
typedef struct _type_set_camara_switch
{
	uint8_t channel_input;
	uint8_t channel_output;
	uint8_t switch_command_len:7;
	uint8_t select_send_port:1;
	uint8_t switch_command_cotent[UPPER_SWICH_COMMAN_COTENT_MAX_LEN];
}tcmp_set_camara_switch;
/*@}*/

#endif

