#include "data.h"
#include "terminal_pro.h"
#include "upper_computer_pro.h"
#include "camera_pro.h"
#include "terminal_system.h"

const proccess_func_items proccess_func_link_tables[MAX_FUNC_LINK_ITEMS] =
{
	{ 0xffffffff, terminal_func_allot_address, FUNC_TMNL_ALLOT_ADDRESS},// 0
	{ 0xffffffff, terminal_func_key_action, FUNC_TMNL_KEY_ACTION},
	{ 0xffffffff, terminal_func_chairman_control, FUNC_TMNL_CHAIRMAN_CONTROL},// 2
	{ 0xffffffff, terminal_func_send_main_state, FUNC_TMNL_SEND_MAIN_STATE},
	{ 0xffffffff, terminal_func_cmd_event, FUNC_TMNL_EVENT},// 4
	
	{ 0xffffffff, proccess_upper_cmpt_discussion_parameter, FUNC_CMPT_DISCUSS_PARAMETER },//5
	{ 0xffffffff, proccess_upper_cmpt_microphone_switch, FUNC_CMPT_MISCROPHONE_SWITCH},//6
	{ 0xffffffff, proccess_upper_cmpt_miscrophone_status, FUNC_CMPT_MISCROPHONE_STATUS},//7
	{ 0xffffffff, proccess_upper_cmpt_select_proposer, FUNC_CMPT_SELECT_PROPOSER},//8
	{ 0xffffffff, proccess_upper_cmpt_examine_application, FUNC_CMPT_EXAMINE_APPLICATION},
	{ 0xffffffff, proccess_upper_cmpt_conference_permission, FUNC_CMPT_CONFERENCE_PERMISSION},//10
	{ 0xffffffff, proccess_upper_cmpt_senddown_message, FUNC_CMPT_SENDDOWN_MESSAGE},
	{ 0xffffffff, proccess_upper_cmpt_tablet_stands_manager, FUNC_CMPT_TABLE_TABLET_STANDS_MANAGER},//12
	{ 0xffffffff, proccess_upper_cmpt__begin_sign, FUNC_CMPT_BEGIN_SIGN},
	{ 0xffffffff, proccess_upper_cmpt_sign_situation, FUNC_CMPT_SIGN_SITUATION},//14
	{ 0xffffffff, proccess_upper_cmpt_end_of_sign, FUNC_CMPT_END_OF_SIGN},
	{ 0xffffffff, proccess_upper_cmpt_endtation_allocation_address, FUNC_CMPT_ENDSTATION_ALLOCATION_APPLICATION_ADDRESS},//16
	{ 0xffffffff, proccess_upper_cmpt_endstation_register_status, FUNC_CMPT_ENDSTATION_REGISTER_STATUS},
	{ 0xffffffff, proccess_upper_cmpt_current_vidicon, FUNC_CMPT_CURRENT_VIDICON},//18
	{ 0xffffffff, proccess_upper_cmpt_endstation_address_undetermined_allocation, FUNC_CMPT_ENDSTATION_ADDRESS_UNDETERMINED_ALLOCATION},
	{ 0xffffffff, proccess_upper_cmpt_vidicon_control, FUNC_CMPT_VIDICON_CONTROL},//20
	{ 0xffffffff, proccess_upper_cmpt_vidicon_preration_set, FUNC_CMPT_VIDICON_PRERATION_SET},
	{ 0xffffffff, proccess_upper_cmpt_vidicon_lock, FUNC_CMPT_VIDICON_LOCK},//22
	{ 0xffffffff, proccess_upper_cmpt_vidicon_output, FUNC_CMPT_VIDICON_OUTPUT},
	{ 0xffffffff, proccess_upper_cmpt_cmpt_begin_vote, FUNC_CMPT_BEGIN_VOTE},//24
	{ 0xffffffff, proccess_upper_cmpt_pause_vote, FUNC_CMPT_PAUSE_VOTE},
	{ 0xffffffff, proccess_upper_cmpt_regain_vote, FUNC_CMPT_REGAIN_VOTE},//26
	{ 0xffffffff, proccess_upper_cmpt_end_vote, FUNC_CMPT_END_VOTE},
	{ 0xffffffff, proccess_upper_cmpt_result_vote, FUNC_CMPT_RESULT_VOTE},//28
	{ 0xffffffff, proccess_upper_cmpt_transmit_to_endstation, FUNC_CMPT_TRANSMIT_TO_ENDSTATION},
	{ 0xffffffff, proccess_upper_cmpt_report_endstation_message, FUNC_CMPT_REPORT_ENDSTATION_MESSAGE},//3 // 30
	{ 0xffffffff, proccess_upper_cmpt_hign_definition_switch_set, FUNC_CMPT_HIGH_DEFINITION_SWITCH_SET},
	
	{ 0xffffffff, terminal_mic_auto_close, FUNC_MENU_AUTO_CLOSE_CMD },// 32
	{ 0xffffffff, terminal_system_discuss_mode_set, FUNC_MENU_DISC_MODE_SET_CMD },
	{ 0xffffffff, terminal_speak_limit_num_set, FUNC_MENU_SPK_LIMIT_NUM_SET }, //34
	{ 0xffffffff, terminal_apply_limit_num_set, FUNC_MUNU_APPLY_LIMIT_NUM_SET },
	
	{ 0xffffffff, termianal_music_enable, FUNC_MENU_MUSIC_EN_SET },// 36
	{ CAMERA_PRESET, camera_preset_save, FUNC_MENU_CMR_SAVE_PRESET },
	{ CAMERA_PRESET, camera_select_num, FUNC_MENU_CMR_SEL_CMR },//38
	{ CAMERA_PRESET, camera_control_rightleft, FUNC_MENU_CMR_CTRL_LR },
	{ CAMERA_PRESET, camera_control_updown, FUNC_MENU_CMR_CTRL_UD },//40
	{ CAMERA_PRESET, camera_control_fouce, FUNC_MENU_CMR_CTRL_FOUCE },
	{ CAMERA_PRESET, camera_control_zoom, FUNC_MENU_CMR_CTRL_ZOOM },//42
	{ CAMERA_PRESET, camera_control_iris, FUNC_MENU_CMR_CTRL_APERT },
	{ CAMERA_PRESET, camera_control_align, FUNC_MENU_CMR_CTRL_ALIGN },//44
	{ 0xffffffff, camera_control_full_view, FUNC_MENU_CMR_FULL_VIEW},
	{ 0xffffffff, camera_preset_set, FUNC_MENU_CMR_CLR_PRESET },//46
	{ 0xffffffff, termianal_chairman_prior_set, FUNC_MENU_PRIOR_EN_SET },
	{ 0Xffffffff, terminal_system_register, FUNC_MENU_TERMINAL_SYS_REGISTER},// 48 // 2016/1/23 add
	
	{ 0xffffffff, camera_preset_addr_select, FUNC_SYS_PRESET_ADDR },
	{ 0xffffffff, camera_get_preset, FUNC_SYS_GET_PRESET }// 50
};

const func_link_items func_link_tables[MAX_FUNC_LINK_ITEMS] =
{
	{ TERMINAL_USE, ALLOCATION, FUNC_TMNL_ALLOT_ADDRESS, 0},
	{ TERMINAL_USE, KEYPAD_ACTION, FUNC_TMNL_KEY_ACTION, 0},
	{ TERMINAL_USE, PRESIDENT_CONTROL, FUNC_TMNL_CHAIRMAN_CONTROL, 0},
	{ TERMINAL_USE, HOST_SEND_STATUS, FUNC_TMNL_SEND_MAIN_STATE, 0},
	{ TERMINAL_USE, END_SPETHING, FUNC_TMNL_EVENT, 0},
	
	{ COMPUTER_USE, DISCUSSION_PARAMETER, FUNC_CMPT_DISCUSS_PARAMETER, 0},
	{ COMPUTER_USE, MISCROPHONE_SWITCH, FUNC_CMPT_MISCROPHONE_SWITCH, 0},
	{ COMPUTER_USE, MISCROPHONE_STATUS, FUNC_CMPT_MISCROPHONE_STATUS, 0},
	{ COMPUTER_USE, SELECT_PROPOSER, FUNC_CMPT_SELECT_PROPOSER, 0},
	{ COMPUTER_USE, EXAMINE_APPLICATION, FUNC_CMPT_EXAMINE_APPLICATION, 0},
	{ COMPUTER_USE, CONFERENCE_PERMISSION, FUNC_CMPT_CONFERENCE_PERMISSION, 0},
	{ COMPUTER_USE, SENDDOWN_MESSAGE, FUNC_CMPT_SENDDOWN_MESSAGE, 0},
	{ COMPUTER_USE, TABLE_TABLET_STANDS_MANAGER, FUNC_CMPT_TABLE_TABLET_STANDS_MANAGER, 0},
	{ COMPUTER_USE, BEGIN_SIGN, FUNC_CMPT_BEGIN_SIGN, 0},
	{ COMPUTER_USE, SIGN_SITUATION, FUNC_CMPT_SIGN_SITUATION, 0},
	{ COMPUTER_USE, END_OF_SIGN, FUNC_CMPT_END_OF_SIGN, 0},
	{ COMPUTER_USE, ENDSTATION_ALLOCATION_APPLICATION_ADDRESS, FUNC_CMPT_ENDSTATION_ALLOCATION_APPLICATION_ADDRESS, 0},
	{ COMPUTER_USE, ENDSTATION_REGISTER_STATUS, FUNC_CMPT_ENDSTATION_REGISTER_STATUS, 0},
	{ COMPUTER_USE, CURRENT_VIDICON, FUNC_CMPT_CURRENT_VIDICON, 0},
	{ COMPUTER_USE, ENDSTATION_ADDRESS_UNDETERMINED_ALLOCATION, FUNC_CMPT_ENDSTATION_ADDRESS_UNDETERMINED_ALLOCATION, 0},
	{ COMPUTER_USE, VIDICON_CONTROL, FUNC_CMPT_VIDICON_CONTROL, 0},
	{ COMPUTER_USE, VIDICON_PRERATION_SET, FUNC_CMPT_VIDICON_PRERATION_SET, 0},
	{ COMPUTER_USE, VIDICON_LOCK, FUNC_CMPT_VIDICON_LOCK, 0},
	{ COMPUTER_USE, VIDICON_OUTPUT, FUNC_CMPT_VIDICON_OUTPUT , 0},
	{ COMPUTER_USE, BEGIN_VOTE, FUNC_CMPT_BEGIN_VOTE, 0},
	{ COMPUTER_USE, PAUSE_VOTE, FUNC_CMPT_PAUSE_VOTE, 0},
	{ COMPUTER_USE, REGAIN_VOTE, FUNC_CMPT_REGAIN_VOTE, 0},
	{ COMPUTER_USE, END_VOTE, FUNC_CMPT_END_VOTE, 0},
	{ COMPUTER_USE, RESULT_VOTE, FUNC_CMPT_RESULT_VOTE, 0},
	{ COMPUTER_USE, TRANSMIT_TO_ENDSTATION, FUNC_CMPT_TRANSMIT_TO_ENDSTATION, 0},
	{ COMPUTER_USE, REPORT_ENDSTATION_MESSAGE, FUNC_CMPT_REPORT_ENDSTATION_MESSAGE, 0},
	{ COMPUTER_USE, HIGH_DEFINITION_SWITCH_SET, FUNC_CMPT_HIGH_DEFINITION_SWITCH_SET, 0},
	
	{ MENU_USE, MENU_AUTO_CLOSE_CMD, FUNC_MENU_AUTO_CLOSE_CMD, 0},
	{ MENU_USE, MENU_DISC_MODE_SET_CMD, FUNC_MENU_DISC_MODE_SET_CMD, 0},
	{ MENU_USE, MENU_SPK_LIMIT_NUM_SET, FUNC_MENU_SPK_LIMIT_NUM_SET, 0},
	{ MENU_USE, MUNU_APPLY_LIMIT_NUM_SET, FUNC_MUNU_APPLY_LIMIT_NUM_SET, 0},
	
	{ MENU_USE, MENU_MUSIC_EN_SET, FUNC_MENU_MUSIC_EN_SET, 0},
	{ MENU_USE, MENU_CMR_SAVE_PRESET, FUNC_MENU_CMR_SAVE_PRESET, 0},
	{ MENU_USE, MENU_CMR_SEL_CMR, FUNC_MENU_CMR_SEL_CMR, 0},
	{ MENU_USE, MENU_CMR_CTRL_LR, FUNC_MENU_CMR_CTRL_LR, 0},
	{ MENU_USE, MENU_CMR_CTRL_UD, FUNC_MENU_CMR_CTRL_UD, 0},
	{ MENU_USE, MENU_CMR_CTRL_FOUCE, FUNC_MENU_CMR_CTRL_FOUCE, 0},
	{ MENU_USE, MENU_CMR_CTRL_ZOOM, FUNC_MENU_CMR_CTRL_ZOOM, 0},
	{ MENU_USE, MENU_CMR_CTRL_APERT, FUNC_MENU_CMR_CTRL_APERT, 0},
	{ MENU_USE, MENU_CMR_CTRL_ALIGN, FUNC_MENU_CMR_CTRL_ALIGN, 0},
	{ MENU_USE, MENU_CMR_FULL_VIEW, FUNC_MENU_CMR_FULL_VIEW, 0},
	{ MENU_USE, MENU_CMR_CLR_PRESET, FUNC_MENU_CMR_CLR_PRESET, 0},
	{ MENU_USE, MENU_PRIOR_EN_SET, FUNC_MENU_PRIOR_EN_SET, 0},
	{ MENU_USE, MENU_TERMINAL_SYS_REGISTER, FUNC_MENU_TERMINAL_SYS_REGISTER },// 2016/1/23 add

	{ SYSTEM_USE, SYS_PRESET_ADDR, FUNC_SYS_PRESET_ADDR, 0},
	{ SYSTEM_USE, SYS_GET_PRESET, FUNC_SYS_GET_PRESET, 0}
};

