#ifndef __UPPER_COMPUTER_COMMAND_EXCUTE_H__
#define __UPPER_COMPUTER_COMMAND_EXCUTE_H__


#include "jdksavdecc_world.h"
#include "host_controller_debug.h"

int upper_cmpt_command_discussion_parameter_excute(uint8_t tp_cmd, uint16_t tp_cmd_data_len,const uint8_t *cmd_data);
int upper_cmpt_command_microphone_switch_excute(uint8_t tp_cmd, uint16_t tp_cmd_data_len,const uint8_t *cmd_data);
int upper_cmpt_command_miscrophone_status_excute(uint8_t tp_cmd, uint16_t tp_cmd_data_len,const uint8_t *cmd_data);
int upper_cmpt_command_select_proposer_excute(uint8_t tp_cmd, uint16_t tp_cmd_data_len,const uint8_t *cmd_data);
int upper_cmpt_command_examine_application_excute(uint8_t tp_cmd, uint16_t tp_cmd_data_len,const uint8_t *cmd_data);
int upper_cmpt_command_conference_permission_excute(uint8_t tp_cmd, uint16_t tp_cmd_data_len,const uint8_t *cmd_data);
int upper_cmpt_command_senddown_message_excute(uint8_t tp_cmd, uint16_t tp_cmd_data_len,const uint8_t *cmd_data);
int upper_cmpt_command_table_tablet_stands_manager_excute(uint8_t tp_cmd, uint16_t tp_cmd_data_len,const uint8_t *cmd_data);
int upper_cmpt_command_begin_sign_excute(uint8_t tp_cmd, uint16_t tp_cmd_data_len,const uint8_t *cmd_data);
int upper_cmpt_command_sign_situation__excute(uint8_t tp_cmd, uint16_t tp_cmd_data_len,const uint8_t *cmd_data);
int upper_cmpt_command_end_of_sign_excute(uint8_t tp_cmd, uint16_t tp_cmd_data_len,const uint8_t *cmd_data);
int upper_cmpt_command_endtation_allocation_address_excute(uint8_t tp_cmd, uint16_t tp_cmd_data_len,const uint8_t *cmd_data);
int upper_cmpt_command_endstation_register_status_excute(uint8_t tp_cmd, uint16_t tp_cmd_data_len,const uint8_t *cmd_data);
int upper_cmpt_command_current_vidicon_excute(uint8_t tp_cmd, uint16_t tp_cmd_data_len,const uint8_t *cmd_data);
int upper_cmpt_command_endstation_address_undetermined_allocation_excute(uint8_t tp_cmd, uint16_t tp_cmd_data_len,const uint8_t *cmd_data);
int upper_cmpt_command_vidicon_control_excute(uint8_t tp_cmd, uint16_t tp_cmd_data_len,const uint8_t *cmd_data);
int upper_cmpt_command_vidicon_preration_set_excute(uint8_t tp_cmd, uint16_t tp_cmd_data_len,const uint8_t *cmd_data);
int upper_cmpt_command_vidicon_lock_excute(uint8_t tp_cmd, uint16_t tp_cmd_data_len,const uint8_t *cmd_data);
int upper_cmpt_command_vidicon_output_excute(uint8_t tp_cmd, uint16_t tp_cmd_data_len,const uint8_t *cmd_data);
int upper_cmpt_command_begin_vote_excute(uint8_t tp_cmd, uint16_t tp_cmd_data_len,const uint8_t *cmd_data);
int upper_cmpt_command_pause_vote_excute(uint8_t tp_cmd, uint16_t tp_cmd_data_len,const uint8_t *cmd_data);
int upper_cmpt_command_regain_vote_excute(uint8_t tp_cmd, uint16_t tp_cmd_data_len,const uint8_t *cmd_data);
int upper_cmpt_command_end_vote_excute(uint8_t tp_cmd, uint16_t tp_cmd_data_len,const uint8_t *cmd_data);
int upper_cmpt_command_result_vote_excute(uint8_t tp_cmd, uint16_t tp_cmd_data_len,const uint8_t *cmd_data);
int upper_cmpt_command_transmit_to_endstation_excute(uint8_t tp_cmd, uint16_t tp_cmd_data_len,const uint8_t *cmd_data);
int upper_cmpt_command_report_endstation_message_excute(uint8_t tp_cmd, uint16_t tp_cmd_data_len,const uint8_t *cmd_data);
int upper_cmpt_command_hign_definition_switch_set_excute(uint8_t tp_cmd, uint16_t tp_cmd_data_len,const uint8_t *cmd_data);

#endif

