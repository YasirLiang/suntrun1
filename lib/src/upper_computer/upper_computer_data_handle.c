#include "upper_computer_data_handle.h"

int handle_command_from_udp_client(uint8_t type_cmd, uint16_t discuss_param_data_len, const void*discuss_param_data )
{
	uint8_t cmd = type_cmd;
	uint16_t cmd_data_len = discuss_param_data_len;
	uint8_t *cmd_data = ( uint8_t* )discuss_param_data;

	switch( cmd )
	{
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_CONFERENCE_DISCUSSION_PARAMETER:// ���ֲ���	
		{
			upper_cmpt_command_discussion_parameter_excute( cmd, cmd_data_len, cmd_data ); 
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_MISCROPHONE_SWITCH:// ��˷翪��
		{
			upper_cmpt_command_microphone_switch_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_MISCROPHONE_STATUS:// ��˷�״̬
		{
			upper_cmpt_command_miscrophone_status_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_SELECT_PROPOSER:// ѡ��������
		{
			upper_cmpt_command_select_proposer_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_EXAMINE_APPLICATION:// ��������
		{
			upper_cmpt_command_examine_application_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_CONFERENCE_PERMISSION:// ����Ȩ��
		{
			upper_cmpt_command_conference_permission_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_SENDDOWN_MESSAGE:// �·�����Ϣ
		{
			upper_cmpt_command_senddown_message_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_TABLE_TABLET_STANDS_MANAGER:// ���ƹ���
		{
			upper_cmpt_command_table_tablet_stands_manager_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_BEGIN_SIGN:// ��ʼǩ��
		{
			upper_cmpt_command_begin_sign_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_SIGN_SITUATION:// ǩ�����
		{
			upper_cmpt_command_sign_situation__excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_END_OF_SIGN:// ǩ������
		{
			upper_cmpt_command_end_of_sign_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_ENDSTATION_ALLOCATION_APPLICATION_ADDRESS:// �ն˷���Ӧ�õ�ַ 
		{
			upper_cmpt_command_endtation_allocation_address_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_ENDSTATION_REGISTER_STATUS:// �ն˱������
		{
			upper_cmpt_command_endstation_register_status_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_CURRENT_VIDICON:// ��ǰ�����
		{
			upper_cmpt_command_current_vidicon_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_ENDSTATION_ADDRESS_UNDETERMINED_ALLOCATION:// ����λ�ն�Ӧ�õ�ַ	
		{
			upper_cmpt_command_endstation_address_undetermined_allocation_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_VIDICON_CONTROL:// ���������
		{
			upper_cmpt_command_vidicon_control_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_VIDICON_PRERATION_SET:// �����Ԥ�ò���
		{
			upper_cmpt_command_vidicon_preration_set_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_VIDICON_LOCK:// ���������
		{
			upper_cmpt_command_vidicon_lock_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_VIDICON_OUTPUT:// ��������
		{
			upper_cmpt_command_vidicon_output_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_BEGIN_VOTE:// ��ʼ���
		{
			upper_cmpt_command_begin_vote_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_PAUSE_VOTE:// �ݶ����
		{
			upper_cmpt_command_pause_vote_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_REGAIN_VOTE:// �ָ����
		{
			upper_cmpt_command_regain_vote_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_END_VOTE:// �������
		{
			upper_cmpt_command_end_vote_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_RESULT_VOTE:// ������
		{
			upper_cmpt_command_result_vote_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_TRANSMIT_TO_ENDSTATION:// ת�����ն�
		{
			upper_cmpt_command_transmit_to_endstation_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_REPORT_ENDSTATION_MESSAGE:// �ϱ��ն˶���Ϣ
		{
			upper_cmpt_command_report_endstation_message_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_HIGH_DEFINITION_SWITCH_SET:// ���ø�������ͷ�л�����
		{
			upper_cmpt_command_hign_definition_switch_set_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		default:
		{
			DEBUG_INFO( "bad conference upper computer command\n" );
		}
		break;
	}
	
	return 0;
}

int handle_pack_event( struct host_upper_cmpt *cnfrnc_pack)
{
	uint8_t loader_hdr = cnfrnc_pack->common_header.state_loader;
	uint8_t sub_type = cnfrnc_pack->common_header.deal_type;
	uint8_t cmd_type = cnfrnc_pack->common_header.command;
	uint16_t data_len = cnfrnc_pack->common_header.data_len;
	uint8_t data_payload[DATA_PAYLOAD_LEN_MAX];
	memcpy( data_payload, cnfrnc_pack->data_payload, data_len );

	DEBUG_INFO("loader_hdr = %02x sub_type = %02x cmd_type = %02x data_len = %02x", \
					loader_hdr, sub_type,cmd_type,data_len);
	if( loader_hdr == UPPER_COMPUTER_DATA_LOADER )
	{	
		// �Ƿ�Ϊ��λ����������, ����Ҫ��Ӧ�ͻ����������Ӧ��inflight���������������������Э�����������
		if( isupper_active_send( sub_type ) )
		{
			handle_command_from_udp_client( cmd_type, data_len, data_payload );
		}
		else	// ��λ����Ӧ��������ʱ��������ֱ�ӷ���
		{
			DEBUG_INFO("There is nothing to do for response from upper computer");
		}
	}
	else
	{
		DEBUG_INFO( "wrong UDP state loader\n" );
		return -1;
	}

	return 0;
}

int handle_upper_computer_conference_data( struct host_upper_cmpt_frame * pframe )
{
	struct host_upper_cmpt_frame cpy_frame;
	struct host_upper_cmpt deal_unpack;
	
	memset( &cpy_frame, 0, sizeof( struct host_upper_cmpt_frame ));
	strcpy( (char*)cpy_frame.dest_address, (char*)pframe->dest_address );
	cpy_frame.dest_port = pframe->dest_port;
	memcpy( cpy_frame.payload, pframe->payload, pframe->payload_len );

	// ���У��
	if( check_crc( cpy_frame.payload, cpy_frame.payload_len ) )
	{
		// ���
		unpack_payload_from_udp_client( &deal_unpack, cpy_frame.payload, cpy_frame.payload_len, 0 );

		// ������յ�������
		handle_pack_event( &deal_unpack );
	}
	else	// ����Ӧ�ͻ���
	{
		return -1;
	}
	
	return 0;
}


