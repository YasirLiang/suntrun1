#ifndef __UPPER_COMPUTER_COMMON_H__
#define __UPPER_COMPUTER_COMMON_H__

#include "jdksavdecc_world.h"
#include "upper_computer.h"
#include "inflight.h"
#include "host_controller_debug.h"
#include "system_packet_tx.h"
#include "upper_computer_data_form.h"


/*{@��λ��������Э��ı�������*/ 
#define CMPT_MSG_TYPE_RESPONSE	0x80 	 // ��Ӧ
#define CMPT_MSG_TYPE_CPT_DIS		0x01 	 // �����˵����ڲ���,��λ��������
#define CMPT_MSG_TYPE_MARK        	0x60 	 // ��������
#define CMPT_MSG_TYPE_QUERY        	0x00 	 // ��ѯ
#define CMPT_MSG_TYPE_SET             	0x20 	 // ����
#define CMPT_MSG_TYPE_REPORT      	0x40 	 // �ϱ�
/*@}*/

/*{@ enable and forbid*/
#define FORBID_VAL  0
#define ENABLE_VAL  1
/*@}*/

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
	uint8_t identity;//��ݴ���
}tcmpt_data_meeting_authority;
/*@}*/

/*{@*/
#define UPPER_MSG_MAX_LENGTH 64
typedef struct _type_upper_computer_data_message
{
	struct application_common_address addr;
	uint8_t msg_type;	// ����Ϣ���� ��terminal_command.h
	uint8_t msg_buf[UPPER_MSG_MAX_LENGTH];//��ݴ���
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

typedef struct _type_cotent_page_roll_type// ҳ������ķ�ʽ
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
	uint8_t msg_type;	// ����Ϣ����
	uint8_t msg_buf[UPPER_MSG_MAX_LENGTH];//��ݴ���
}tcmpt_table_card; 
/*@}*/

/*{@ sign begin*/
typedef struct _type_begin_sign_in
{
	uint8_t sign_type;
	uint8_t retroactive_timeouts; // ��ǩ�ĳ�ʱʱ�䣬��λ�Ƿ���
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
typedef struct _type_end_allot_address // �ն˷���Ӧ�õ�ַ
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

typedef struct _type_end_register_situation // �ն˱������
{
	struct application_common_address addr;
	uint8_t device_type; // �豸����
	uint8_t end_type:2; // �ն�����
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

/*{@ wait allocation ����λ���ն�Ӧ�õ�ַ*/ 
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
	uint8_t close_focus_flags:1; 			// ������۽�
	uint8_t aperture_amplification_flags:1; 	// ��Ȧ�Ŵ�
	uint8_t aperture_shrinks_flags:1;		// ��Ȧ��С
	uint8_t :5;

	uint8_t :1;
	uint8_t right_flags:1;
	uint8_t left_flags:1;
	uint8_t up_flags:1;
	uint8_t down_flags:1;
	uint8_t close_object:1;				// �ӽ�����
	uint8_t remote_object:1;				// Զ������
	uint8_t remote_focus_flags:1;			// Զ����۽�

	uint8_t level_speed;
	uint8_t vertical_speed;
}tcmp_camara_ctl;
/*@}*/

/*{@*/
enum
{
	SAVE_PRESET_BIT, // Ԥ��λ����
	ENTER_PRESET_BIT, // ����Ԥ��λ����
	EXIT_PRESET_BIT, // �˳�Ԥ��λ����
	SAVE_CAMARA_ALLOCATION_ADJUST // ����λ��������
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
#define CAMARA_PANORAMA 0 // ȫ��
#define CAMARA_TRACE 0 // ׷��
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
	uint8_t vote_type:4;  // ��terminal_common.h ��terminal VOTE MODE
	uint8_t key_effective:1;
	uint8_t :3;
}tcmp_vote_start;
/*@}*/

/*{@*/
typedef struct _type_vote_result
{
	struct application_common_address addr;
	uint8_t key_value:5; // ����λ��Ӧ��Ӧ�ն˰�����ֵ
	uint8_t :3;
}tcmp_vote_result;

/*@}*/

/*{@*/
#define MESSAGE_DATA_MAX_LENGHT 64
#define HOST_REPORT_CARD_COTENT 0 // �ϱ���������
#define HOST_REPORT_PLUG_CARD_ACTION 1 // �忨�����ϱ�
struct terminal_message_card_common
{
	uint8_t msg_type; // �ն˶���Ϣ����
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

extern int upper_computer_send( void* data_send );
extern void proccess_udp_client_msg_recv( uint8_t *frame, int frame_len );
extern int  send_upper_and_host_deal_command( uint8_t deal_type, uint8_t command, const void *data, uint16_t data_len );

#endif

