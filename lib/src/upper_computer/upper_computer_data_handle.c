#include "upper_computer_data_handle.h"

int handle_command_from_udp_client(uint8_t type_cmd, uint16_t discuss_param_data_len, const void*discuss_param_data )
{
	uint8_t cmd = type_cmd;
	uint16_t cmd_data_len = discuss_param_data_len;
	uint8_t *cmd_data = ( uint8_t* )discuss_param_data;

	switch( cmd )
	{
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_CONFERENCE_DISCUSSION_PARAMETER:// 会讨参数	
		{
			upper_cmpt_command_discussion_parameter_excute( cmd, cmd_data_len, cmd_data ); 
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_MISCROPHONE_SWITCH:// 麦克风开关
		{
			upper_cmpt_command_microphone_switch_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_MISCROPHONE_STATUS:// 麦克风状态
		{
			upper_cmpt_command_miscrophone_status_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_SELECT_PROPOSER:// 选择申请人
		{
			upper_cmpt_command_select_proposer_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_EXAMINE_APPLICATION:// 审批申请
		{
			upper_cmpt_command_examine_application_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_CONFERENCE_PERMISSION:// 会议权限
		{
			upper_cmpt_command_conference_permission_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_SENDDOWN_MESSAGE:// 下发短信息
		{
			upper_cmpt_command_senddown_message_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_TABLE_TABLET_STANDS_MANAGER:// 桌牌管理
		{
			upper_cmpt_command_table_tablet_stands_manager_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_BEGIN_SIGN:// 开始签到
		{
			upper_cmpt_command_begin_sign_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_SIGN_SITUATION:// 签到情况
		{
			upper_cmpt_command_sign_situation__excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_END_OF_SIGN:// 签到结束
		{
			upper_cmpt_command_end_of_sign_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_ENDSTATION_ALLOCATION_APPLICATION_ADDRESS:// 终端分配应用地址 
		{
			upper_cmpt_command_endtation_allocation_address_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_ENDSTATION_REGISTER_STATUS:// 终端报到情况
		{
			upper_cmpt_command_endstation_register_status_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_CURRENT_VIDICON:// 当前摄像机
		{
			upper_cmpt_command_current_vidicon_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_ENDSTATION_ADDRESS_UNDETERMINED_ALLOCATION:// 待定位终端应用地址	
		{
			upper_cmpt_command_endstation_address_undetermined_allocation_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_VIDICON_CONTROL:// 摄像机控制
		{
			upper_cmpt_command_vidicon_control_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_VIDICON_PRERATION_SET:// 摄像机预置操作
		{
			upper_cmpt_command_vidicon_preration_set_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_VIDICON_LOCK:// 摄像机锁定
		{
			upper_cmpt_command_vidicon_lock_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_VIDICON_OUTPUT:// 摄像机输出
		{
			upper_cmpt_command_vidicon_output_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_BEGIN_VOTE:// 开始表决
		{
			upper_cmpt_command_begin_vote_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_PAUSE_VOTE:// 暂定表决
		{
			upper_cmpt_command_pause_vote_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_REGAIN_VOTE:// 恢复表决
		{
			upper_cmpt_command_regain_vote_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_END_VOTE:// 结束表决
		{
			upper_cmpt_command_end_vote_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_RESULT_VOTE:// 表决结果
		{
			upper_cmpt_command_result_vote_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_TRANSMIT_TO_ENDSTATION:// 转发给终端
		{
			upper_cmpt_command_transmit_to_endstation_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_REPORT_ENDSTATION_MESSAGE:// 上报终端短信息
		{
			upper_cmpt_command_report_endstation_message_excute( cmd, cmd_data_len, cmd_data );
		}
		break;
		case HOST_UPPER_COMPUTER_COMMAND_TYPE_HIGH_DEFINITION_SWITCH_SET:// 设置高清摄像头切换命令
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
		// 是否为上位机主动发出, 是则要响应客户机后，清除相应的inflight命令，这里依据数据类型与协议的命令类型
		if( isupper_active_send( sub_type ) )
		{
			handle_command_from_udp_client( cmd_type, data_len, data_payload );
		}
		else	// 上位机响应，主机暂时不做处理，直接返回
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

	// 检查校验
	if( check_crc( cpy_frame.payload, cpy_frame.payload_len ) )
	{
		// 解包
		unpack_payload_from_udp_client( &deal_unpack, cpy_frame.payload, cpy_frame.payload_len, 0 );

		// 处理接收到的数据
		handle_pack_event( &deal_unpack );
	}
	else	// 不响应客户端
	{
		return -1;
	}
	
	return 0;
}


