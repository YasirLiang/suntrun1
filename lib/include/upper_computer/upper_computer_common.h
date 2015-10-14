#ifndef __UPPER_COMPUTER_COMMON_H__
#define __UPPER_COMPUTER_COMMON_H__

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
	uint8_tapply_flag;
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
typedef struct _type_upper_computer_data_meeting_authority
{
	struct application_common_address addr;
	uint8_t msg_buf[UPPER_MSG_MAX_LENGTH];//身份代表
}tcmpt_data_meeting_authority;
/*@}*/

/*{@*//*@}*/
/*{@*//*@}*/
/*{@*//*@}*/
/*{@*//*@}*/
/*{@*//*@}*/
/*{@*//*@}*/
/*{@*//*@}*/
/*{@*//*@}*/
/*{@*//*@}*/
/*{@*//*@}*/
/*{@*//*@}*/
/*{@*//*@}*/
/*{@*//*@}*/
/*{@*//*@}*/
/*{@*//*@}*/
/*{@*//*@}*/
/*{@*//*@}*/
/*{@*//*@}*/
/*{@*//*@}*/
/*{@*//*@}*/
/*{@*//*@}*/
/*{@*//*@}*/
/*{@*//*@}*/
/*{@*//*@}*/
/*{@*//*@}*/

#endif

