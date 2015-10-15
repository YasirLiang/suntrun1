#include "upper_computer_command.h"
#include "host_controller_debug.h"

void upper_cmpt_command_askbuf_set( struct host_upper_cmpt *askbuf, uint8_t deal_type, uint8_t command, void *data, ssize_t data_len )
{
	assert( askbuf );
	askbuf->common_header.state_loader = UPPER_COMPUTER_DATA_LOADER;
	askbuf->common_header.deal_type = deal_type;
	askbuf->common_header.command = command;
	askbuf->common_header.data_len = data_len;
	memset( askbuf, 0, DATA_PAYLOAD_LEN_MAX);

	if( (data_len != 0) && (NULL != data))
	{
		if( data_len <= DATA_PAYLOAD_LEN_MAX )
			memcpy( askbuf->data_payload , (uint8_t *)data, data_len );// 这里是从低字节开始拷贝的，所以data必须协议数据传输的顺序一致
		else
		{	
			DEBUG_INFO( "error upper computer trasmit deal data:too length!" );
			assert( data_len <= DATA_PAYLOAD_LEN_MAX );
		}
	}
}

void upper_computer_discussion_parameter( uint8_t protocol_type, tcmpt_discuss_parameter disc_param )
{
	ssize_t data_len = sizeof( tcmpt_discuss_parameter ); // 4
	struct host_upper_cmpt askbuf;
	bool is_resp = true;				// 初始化为响应数据
	
	if( (protocol_type & CMPT_MSG_TYPE_RESPONSE) == CMPT_MSG_TYPE_QUERY )
	{
		upper_cmpt_command_askbuf_set( &askbuf, protocol_type, &disc_param, HOST_UPPER_COMPUTER_COMMAND_TYPE_CONFERENCE_DISCUSSION_PARAMETER, data_len );
	}
	else
	{
		upper_cmpt_command_askbuf_set( &askbuf, protocol_type, NULL, HOST_UPPER_COMPUTER_COMMAND_TYPE_CONFERENCE_DISCUSSION_PARAMETER, 0 ); 
	}

	upper_computer_send( &askbuf, is_resp );
}
void upper_computer_microphone_switch()
{
	
}

void upper_computer_miscrophone_status()
{
	
}

void upper_computer_select_proposer()
{
	
}
void upper_computer_examine_application()
{
	
}

void upper_computer_conference_permission()
{
	
}

void upper_computer_senddown_message()
{
	
}

void upper_computer_table_tablet_stands_manager()
{
	
}

void upper_computer_begin_sign()
{
	
}

void upper_computer_sign_situation()
{
	
}

void upper_computer_end_of_sign()
{
	
}

void upper_computer_endtation_allocation_address()
{
	
}

void upper_computer_endstation_register_status()
{
	
}

void upper_computer_current_vidicon()
{
	
}

void upper_computer_endstation_address_undetermined_allocation()
{
	
}

void upper_computer_vidicon_control()
{
	
}

void upper_computer_vidicon_preration_set()
{
	
}

void upper_computer_vidicon_lock()
{
	
}

void upper_computer_vidicon_output()
{
	
}

void uppercomputer_begin_vote()
{
	
}

void upper_computer_pause_vote()
{
	
}

void upper_computer_regain_vote()
{
	
}

void upper_computer_end_vote()
{
	
}

void upper_computer_result_vote()
{
	
}

void upper_computer_transmit_to_endstation()
{
	
}

void upper_computer_report_endstation_message()
{
	
}

int upper_computer_hign_definition_switch_set()
{
	
}