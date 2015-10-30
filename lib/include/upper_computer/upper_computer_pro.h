#ifndef __UPPER_COMPUTER_PRO_H__
#define __UPPER_COMPUTER_PRO_H__

#include "jdksavdecc_world.h"
#include "upper_computer_common.h"

#define CMPT_VOTE_KEY_MARK	0x1F
#define CMPT_VOTE_EN         		0x80
#define CMPT_VOTE_SET_FLAG   	0x40
#define CMPT_WAIT_VOTE_FLAG  	0x20

int profile_system_file_dis_param_save( FILE* fd, tcmpt_discuss_parameter *set_dis_para );

/*{@÷’∂À√¸¡Ó∫Ø ˝*/
int proccess_upper_cmpt_discussion_parameter( uint16_t protocol_type, void *data, uint32_t data_len );

/*@÷’∂À√¸¡Ó∫Ø ˝}*/

#endif

