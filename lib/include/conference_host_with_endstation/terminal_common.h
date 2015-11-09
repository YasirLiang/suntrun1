/**
*terminal_common.h
*
*terminal system status define
*/

#ifndef __TERMINAL_COMMON_H__
#define __TERMINAL_COMMON_H__

#include "jdksavdecc_world.h"
#include "conference.h"

#define SYSTEM_TMNL_MAX_NUM 256	// ϵͳ���ն���������
#define INIT_ADDRESS 0xffff
#define BRDCST_1722_ALL 0 // 

#define COMMAND_TMN_MASK	 	0x1F	//��������,���������ڵ�5λ
#define COMMAND_FROM_TMN	 	0x80	//�����
#define COMMAND_TMN_REPLY	 	0x40	//��Ӧ���ı�־
#define COMMAND_TMN_CHAIRMAN	0x20	//��ϯ��Ԫ��ע 

#define CONFERENCE_BROADCAST_ADDRESS 0x8000
#define TMN_ADDR_MASK   0x0FFF
#define BROADCAST_FLAG  0x8000
#define BRCT_RANGE_MARK 0xFE00
#define BRDCST_ALL        0x8000
#define BRDCST_MEM      0x8200
#define BRDCST_VIP      0x8400
#define BRDCST_CHM      0x8800
#define BRDCST_EXE      0x9000

#define BRDCST_NEED_RESPONSE 0x0100

#define FREE_MODE_SPEAK_MAX 10 // ����ģʽ��󽲻�����

/*{@ set terminal status*/
#define MIC_CLOSE 1
#define MIC_INVALID 0


enum // tmnl sys status
{
	TMNL_SYS_STA_DISC = 1,
	TMNL_SYS_STA_SIGN,
	TMNL_SYS_STA_VOTE,
	TMNL_SYS_STA_GRADE,
	TMNL_SYS_STA_SELECT,
	TMNL_SYS_STA_CHM_INTERPOSE,
	TMNL_SYS_STA_SYS_SET
};

enum
{
	AUTO_CLOSE_FUNC_CLOSE,
	AUTO_CLOSE_FUNC_OPEN
};

enum
{
	KEY_SIGN_IN,
	CARD_SIGN_IN
};

enum
{
	END_KEY_VALUE,
	FIRST_KEY_VALUE
};

enum // terminal VOTE MODE
{
	VOTE_MODE,
	GRADE_MODE,
	SLCT_2_1,
	SLCT_2_2,
	SLCT_3_1,
	SLCT_3_2,
	SLCT_3_3,
	SLCT_4_1,
	SLCT_4_2,
	SLCT_4_3,
	SLCT_4_4,
	SLCT_5_1,
	SLCT_5_2,
	SLCT_5_3,
	SLCT_5_4,
	SLCT_5_5,
	VOTE_MODE_NUM
};

enum		//terminal type
{
	TMNL_TYPE_COMMON_RPRST,
	TMNL_TYPE_VIP,
	TMNL_TYPE_CHM_COMMON,
	TMNL_TYPE_CHM_EXCUTE,
	TMNL_TYPE_NUM
};
/*@}*/

/*{@*/
enum		// mic STATUS
{
	MIC_COLSE_STATUS,
	MIC_OPEN_STATUS,
	MIC_FIRST_APPLY_STATUS,
	MIC_OTHER_APPLY_STATUS,
	MIC_PRESET_BIT_STATUS,
	MIC_CHM_INTERPOSE_STATUS
};

/*@}*/
enum		// terminal lamp STATUS
{
	TMNL_LAMP_OFF,
	TMNL_LAMP_SLOW_FLASH,
	TMNL_LAMP_QUICK_FLASH,
	TMNL_LAMP_ALWAYS_ON
};
/*{@*/

/*{@ KEY ACTION*/


/*@}*/


/*@}*/

#define LCD_OR_LED_DIS_DATA_ADD_SCREEN 0
#define LCD_OR_LED_DIS_DATA_CHANGE_SCREEN 1
/*{@ LCD DISPLAY STYPE AND DISPLAY DATA*/
#define TLCD_DATA_ENG 1
#define TLCD_DATA_CHN 2
#define TLCD_DATA_DOT 3

#define MAX_LCD_DATA_LEN  30
typedef struct _tterminal_lcd_data
{
	uint8_t data_type;
	uint8_t x;
	uint8_t y;
	uint8_t width;
	uint8_t high;
	uint8_t data_buf[MAX_LCD_DATA_LEN];
}tmnl_lcd_data;

/*@}*/

/*{@ LED DISPLAY STYPE AND DISPLAY DATA*/ 
#define LED_DISPLAY_NOMAL 0
#define LED_DISPLAY_FLASH 1
#define MAX_LED_DATA_LEN 30

enum
{
	LED_PAGE_TURN,
	LED_PAGE_LEFT_ROLL,
	LED_PAGE_RIGHT_ROLL,
	LED_PAGE_UP_ROLL,
	LED_PAGE_DOWN_ROLL,
};

typedef struct _tterminal_led_data_display
{
	uint8_t  data_type;
	uint8_t data_buf[MAX_LED_DATA_LEN];
}tmnl_led_data_dis;
/*@}*/

/*{@ chairman control meeting */
enum
{
	DATA_BEGIN_SIGN,
	DATA_END_SIGN,
	DATA_BEGIN_VOTE,
	DATA_END_VOTE,
	DATA_SUSPEND_VOTE,
	DATA_RECOVER_VOTE,
	DATA_RETURN_DISCUSS,
	DATA_CLOSE_ALL_MIC,
	DATA_CHM_CONTROL_LEN
};
/*@}*/

/*{@*/
#define LIMITE_MAX_SPEAK_TIME 63
/*@}*/

/*{@*/
typedef enum _tterminal_discuss_mode 	// ����ģʽ
{
	FREE_MODE,
	PPT_MODE,
	LIMIT_MODE,
	FIFO_MODE,
	APPLY_MODE		
}ttmnl_discuss_mode;

/*@}*/

/*{@�����նˣ�0x12��*/
typedef enum _eoption_terminal
{
	OPT_TMNL_REBOOT,
	OPT_TMNL_SUSPEND_VOTE,
	OPT_TMNL_RECOVER_VOTE,
	OPT_TMNL_SET_VIP,
	OPT_TMNL_CANCEL_VIP,
	OPT_TMNL_SET_EXCUTE_CHM,
	OPT_TMNL_CANCEL_EXCUTE_CHM,
	OPT_TMNL_LED_DISPLAY_ROLL_SYNC,
	OPT_TMNL_ALL_SIGN,
	OPT_TMNL_ALL_VOTE
}eopt_tmnl;
/*�����նˣ�0x12��@}*/

/*{@ trasmit upper computer msg*/
enum
{
	UPPER_CMPT_MSG_TYPE_LED_DISPLAY,
	UPPER_CMPT_MSG_TYPE_LCD_DISPLAY,
	UPPER_CMPT_MSG_TYPE_READ_CARD,
	UPPER_CMPT_MSG_TYPE_WRITE_CARD,
	UPPER_CMPT_MSG_TYPE_BATCH_READ_CARD, 	// ��������
	UPPER_CMPT_MSG_TYPE_BATICH_WRITE_CARD	// ����д�� 
};
/*@}*/

/*{@ trasmit terminal msg*/
enum
{
	TMNL_MSG_TYPE_REPORT_CARD_COTENT,
	TMNL_MSG_TYPE_INSET_CARD
};
/*@}*/

typedef struct tterminal_addr_list	// �ն˷����ַ
{
	uint16_t addr;
	uint16_t  tmn_type;
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
	terminal_address_list address; // �ն˵�ַ
	terminal_state tmnl_status;	 // �ն˵�״̬ 
}conference_terminal_device; 

typedef  struct tmnl_list_node		// �ն�����ڵ� 
{
	conference_terminal_device tmnl_dev;
	struct tmnl_list_node *next,*prior;
}tmnl_dblist, *tmnl_pdblist; 

typedef struct _tterminal_state_set // �ն�״̬������
{
	uint8_t sys:4;
	uint8_t auto_close:1;
	uint8_t sign_stype:1;	// ǩ����ʽ ���� ��忨
	uint8_t one_off:1; 		// �׼���ĩ����Ч
	uint8_t :1;

	uint8_t keydown:5;
	uint8_t :3;
	
	uint8_t keyup:5;
	uint8_t :3;
	
	uint8_t VoteType:5;
	uint8_t :2;
	uint8_t MicClose:1;
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
	uint8_t limit_time:6;
	uint8_t :2;
}tmnl_limit_spk_time;

typedef struct _ttmerminal_main_state_send // ��������״̬
{
	uint16_t unit;				// �����������ն�����
	
	uint8_t conference_stype:4; // ����ģʽ
	uint8_t :2;
	uint8_t chm_first:1;
	uint8_t camera_follow:1;		// ģʽ����
	
	uint8_t limit;				// ���������������
	uint8_t apply_set;			// ������������
	uint8_t spk_num;			// ��������
	uint8_t apply;				// ��������
}tmnl_main_state_send;

/*{@ lcd ������ʾ����*/
#define LCD_OPTION_DISPLAY 0
#define LCD_OPTION_CLEAR  1

enum // ������ʾ
{
	CHM_APPROVE_APPLY_INTERFACE,
	VOID_VOTE_INTERFACE,
	VOTE_INTERFACE,
	GRADE_1_INTERFACE,
	GRADE_2_INTERFACE,
	SLCT_LV_2_INTERFACE,
	SLCT_LV_3_INTERFACE,
	SLCT_LV_4_INTERFACE,
	SLCT_LV_5_INTERFACE,
	MIC_OPEN_INTERFACE,
	REPRESEND_APPLY_INTERFACE
};

typedef struct  _tterminal_send_end_lcd_display
{
	uint8_t opt;	// ����ָʾ 
	uint8_t num;  // ����
}tmnl_send_end_lcd_display;

/*@}*/

/*{@�����ն�ָʾ��*/
#define TLED_KEY1       0
#define TLED_KEY2       1
#define TLED_KEY3       2
#define TLED_KEY4       3
#define TLED_KEY5       4
#define TLED_MAX_NUM    5
#define TLED_OFF    0
#define TLED_SLOW   1
#define TLED_QUICK  2
#define TLED_ON     3

typedef struct _tterminal_led_lamp_set
{
	uint8_t data_low;
	uint8_t data_high;
}ttmnl_led_lamp;


/*@}*/

int terminal_address_list_write_file( FILE* fd,  terminal_address_list* ptmnl_addr, const uint16_t write_counts );
int terminal_address_list_read_file( FILE* fd,  terminal_address_list* ptmnl_addr );
uint16_t ternminal_send( void *buf, uint16_t length, uint64_t uint64_target_id, bool is_resp_data );
void terminal_recv_message_pro( struct terminal_deal_frame *conference_frame );
void host_reply_terminal( uint8_t cmd, uint16_t address, uint8_t *data_pay, uint16_t data_len );



#endif

