/**
*terminal_common.h
*
*terminal system status define
*/

#ifndef __TERMINAL_COMMON_H__
#define __TERMINAL_COMMON_H__

#include "jdksavdecc_world.h"
#include "conference.h"

#define ADDRESS_FILE "address.dat"		// 终端地址信息存放的文件

#define COMMAND_TMN_MASK	 	0x1F	//命令掩码,命令内容在低5位
#define COMMAND_FROM_TMN	 	0x80	//命令方向，
#define COMMAND_TMN_REPLY	 	0x40	//响应报文标志
#define COMMAND_TMN_CHAIRMAN	0x20	//主席单元标注 

#define CONFERENCE_BROADCAST_ADDRESS 0x8000
#define TMN_ADDR_MASK   0x0FFF
#define BROADCAST_FLAG  0x8000
#define BRCT_RANGE_MARK 0xFE00
#define BRDCST_ALL        0x8000
#define BRDCST_MEM      0x8200
#define BRDCST_VIP      0x8400
#define BRDCST_CHM      0x8800
#define BRDCST_EXE      0x9000

/*{@ set terminal status*/
#define MIC_CLOSE 1


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
	TMNL_TYPE_CHM_EXCUTE
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
	LEC_PAGE_LEFT_ROLL,
	LEC_PAGE_RIGHT_ROLL,
	LEC_PAGE_UP_ROLL,
	LEC_PAGE_DOWN_ROLL,
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
typedef enum _tterminal_discuss_mode 	// 会讨模式
{
	FREE_MODE,
	PPT_MODE,
	LIMIT_MODE,
	FIFO_MODE,
	APPLY_MODE		
}ttmnl_discuss_mode;
/*@}*/

/*{@*/
enum // 界面显示
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
/*@}*/

/*{@ operate  terminal*/
enum
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
};
/*@}*/

/*{@ trasmit upper computer msg*/
enum
{
	UPPER_CMPT_MSG_TYPE_LED_DISPLAY,
	UPPER_CMPT_MSG_TYPE_LCD_DISPLAY,
	UPPER_CMPT_MSG_TYPE_READ_CARD,
	UPPER_CMPT_MSG_TYPE_WRITE_CARD,
	UPPER_CMPT_MSG_TYPE_BATCH_READ_CARD, 	// 批量读卡
	UPPER_CMPT_MSG_TYPE_BATICH_WRITE_CARD	// 批量写卡 
};
/*@}*/

/*{@ trasmit terminal msg*/
enum
{
	TMNL_MSG_TYPE_REPORT_CARD_COTENT,
	TMNL_MSG_TYPE_INSET_CARD
};
/*@}*/

#endif

