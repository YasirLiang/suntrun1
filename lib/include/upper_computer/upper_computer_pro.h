#ifndef __UPPER_COMPUTER_PRO_H__
#define __UPPER_COMPUTER_PRO_H__

#include "jdksavdecc_world.h"
#include "upper_computer_common.h"

int profile_system_file_dis_param_save( FILE* fd, tcmpt_discuss_parameter *set_dis_para );

/*{@终端命令函数*/
int proccess_upper_cmpt_discussion_parameter( uint16_t protocol_type, void *data, uint32_t data_len );
int proccess_upper_cmpt_microphone_switch( uint16_t protocal_type, void *data, uint32_t data_len );
int proccess_upper_cmpt_miscrophone_status( uint16_t protocal_type, void *data, uint32_t data_len );// 后期可能需要修改10/30。
int proccess_upper_cmpt_select_proposer( uint16_t protocal_type, void *data, uint32_t data_len );//函数处理流程未完成10/30。
int proccess_upper_cmpt_examine_application( uint16_t protocal_type, void *data, uint32_t data_len );//函数处理流程未完成10/30。
int proccess_upper_cmpt_conference_permission( uint16_t protocal_type, void *data, uint32_t data_len );//函数处理流程未完成10/30。
int proccess_upper_cmpt_senddown_message( uint16_t protocal_type, void *data, uint32_t data_len );//函数处理流程未完成10/30。
int proccess_upper_cmpt_tablet_stands_manager( uint16_t protocal_type, void *data, uint32_t data_len ); // 函数处理流程未完成10/30。
int proccess_upper_cmpt__begin_sign( uint16_t protocal_type, void *data, uint32_t data_len ); // 函数处理流程未完成10/30。
int proccess_upper_cmpt_sign_situation( uint16_t protocal_type, void *data, uint32_t data_len );//后期可能需要修改10/30。
int proccess_upper_cmpt_end_of_sign( uint16_t protocal_type, void *data, uint32_t data_len );// 函数处理流程未完成10/30。
int proccess_upper_cmpt_endtation_allocation_address( uint16_t protocal_type, void *data, uint32_t data_len );//后期可能需要修改11/2。
int proccess_upper_cmpt_endstation_register_status( uint16_t protocal_type, void *data, uint32_t data_len );//后期可能需要修改11/2。
int proccess_upper_cmpt_current_vidicon( uint16_t protocal_type, void *data, uint32_t data_len );// 函数处理流程未完成11/2。
int proccess_upper_cmpt_endstation_address_undetermined_allocation( uint16_t protocal_type, void *data, uint32_t data_len );// 函数处理流程未完成11/2。
int proccess_upper_cmpt_vidicon_control( uint16_t protocal_type, void *data, uint32_t data_len );// 函数处理流程未完成11/2。
int proccess_upper_cmpt_vidicon_preration_set( uint16_t protocal_type, void *data, uint32_t data_len );// 函数处理流程未完成11/2。
int proccess_upper_cmpt_vidicon_lock( uint16_t protocal_type, void *data, uint32_t data_len );// 函数处理流程未完成11/2。
int proccess_upper_cmpt_vidicon_output( uint16_t protocal_type, void *data, uint32_t data_len );// 函数处理流程未完成11/2。
int proccess_upper_cmpt_cmpt_begin_vote( uint16_t protocal_type, void *data, uint32_t data_len );// 函数处理流程未完成11/2。
int proccess_upper_cmpt_pause_vote( uint16_t protocal_type, void *data, uint32_t data_len );
int proccess_upper_cmpt_regain_vote( uint16_t protocal_type, void *data, uint32_t data_len );
int proccess_upper_cmpt_end_vote( uint16_t protocal_type, void *data, uint32_t data_len );
int proccess_upper_cmpt_result_vote( uint16_t protocal_type, void *data, uint32_t data_len );//后期可能需要修改10/30。
int proccess_upper_cmpt_transmit_to_endstation( uint16_t protocal_type, void *data, uint32_t data_len );// 函数处理流程未完成11/2。
int proccess_upper_cmpt_report_endstation_message( uint16_t protocal_type, void *data, uint32_t data_len );//后期可能需要修改11/2。
int proccess_upper_cmpt_hign_definition_switch_set( uint16_t protocal_type, void *data, uint32_t data_len );// 函数处理流程未完成11/2。


/*@终端命令函数}*/

/*{@上位机处理函数流程}*/
int upper_cmpt_report_mic_state( uint8_t mic_status, uint16_t addr );
int cmpt_miscrophone_status_list( void );
int upper_cmpt_report_sign_in_state( uint8_t sign_status, uint16_t addr );
int upper_cmpt_sign_situation_report( uint8_t vote_rlst, uint16_t addr );

/*@上位机处理函数流程}*/


#endif

