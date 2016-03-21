#include "controller_command.h"
#include "acmp_controller_machine.h"
#include "adp_controller_machine.h"
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "terminal_pro.h"
#include "terminal_command.h"
#include "descriptor.h"
#include "jdksavdecc_util.h"
#include "upper_computer_command.h"
#include "terminal_system_func.h"
#include "terminal_common.h"
#include "system.h"
#include "control_matrix_pro.h"
#include "matrix_output_input.h"
#include "menu_cli_func.h"

static solid_pdblist end_list_guard = NULL;

/*====================补全命令===============*/
#define COMMANDS_MAX_NUM 128
static const char *commands_list[COMMANDS_MAX_NUM] = 
{
	"adp",
	"connect",
	"clear",
	"disconnect",
	"list",
	"ls",
	"update" ,
	"udpClient",
	"q",
	"quit",
	"show",
	// 开始-测试终端命令的命令
	"terminal",
	"query",
	"allot",
	"reAllot",
	"setState",
	"setMic",
	"setIndicator",
	"newAllot",
	"setLcd",
	"setLed",
	"chairManControl",
	"voteResult",
	"limitSpkTime",
	"hostSendState",
	"sendEndLcd",
	"optionEndpoint",
	"specialEventReply",
	"tnmtCmptMsg",
	"replyEndMessage",
	"queryVoteSign",
	// 结束-测试终端命令的命令
	
	// 开始-系统功能
	"hostFunc",
	"reAllot",
	"matrixControl",// 矩阵控制
	// // 菜单功能测试 ////////////////
	"MenuTest",// 菜单功能测试
	"ModeSet",
	"TempClose",
	"ChairmanHint",
	"CameraTrack",
	"AutoClose",
	"SpeakLimit",
	"ApplyNumSet",
	"ReAllot",
	"NewAllot",
	"SetFinish",
	"CameraCtl",
	"CameraCtlLeftRight",
	"CameraCtlUpDown",
	"CameraCtlFouce",
	"CameraCtlIris",
	"CameraCtlZoom",
	"DistanceCtl",
	"PresetSave",
	"SwitchCmr",
	"ClearPreset",
	"SelectPresetAddr",
	"EnterEscPreset",
	"help",// 菜单功能测试 ////////////////
	// 结束-系统功能
	
	NULL
};

static char* command_generator(const char *text, int state)
{
	const char *name;
	static int list_index, len;

	if ( !state )
	{
		list_index = 0;
		len = strlen (text);
	}

	while ((name = commands_list[list_index]) != NULL )
	{
		list_index++;

		if (strncmp (name, text, len) == 0)
			return strdup(name);
	}

	return ((char *)NULL);
}

char** command_completion (const char *text, int start, int end)
{
	char **matches = NULL;

	if (start == 0)
	matches = rl_completion_matches (text, command_generator);

	return (matches);
}

void initialize_readline( void )
{
	rl_readline_name = "readline";
	rl_attempted_completion_function = command_completion;

	return ;
}
/*============================================*/

// update system endstation connections by sending acmp comand
void cmd_update_proccess( void )
{
	acmp_update_endstation_connections_networks();
}

void cmd_show_proccess( void )
{
	// look for connections in system descriptor list after one seconds
	descriptor_looking_for_connections( descptor_guard );
}

void cmd_adp_proccess( const char *opt )
{
	const char *p = opt + 4;	// the num of 'adp' and one blank character
	char entity_str_id[32] = {0};
	//uint64_t entity_entity_id = 0;
	char entity_str_msg[32] = {0};
	uint16_t msg_type = 0;
	struct jdksavdecc_eui64 discover_entity_id;
	int input_flag = 0;
	memset( &discover_entity_id, 0, sizeof discover_entity_id );
	const char *first = p;
	
	while( *p != '\0' )
	{
		int copy_num = 0;
		first = p;
		for( copy_num = 0; (!isspace(*p)) &&  (*p != 0); p++)
			copy_num++;	

		if( input_flag == 0 )
		{
			memcpy( entity_str_msg, first, copy_num );
			if( !get_adpdu_msg_type_value_with_str(entity_str_msg, &msg_type))
			{
				MSGINFO( " Please check type is ENTITY_AVAILABLE or ENTITY_DEPARTING or ENTITY_DISCOVER" );
				return; 
			}
			
			p++;
			input_flag++;
		}
		else if( input_flag == 1 )
		{
			memcpy( entity_str_id, first, copy_num );
			convert_str_to_eui64( entity_str_id, discover_entity_id.value );
			input_flag++;
		}

		MSGINFO( "input_flag = %d", input_flag);
	}

	if( (input_flag == 2))
	{
		if(input_flag == 0)
			msg_type = 0;
		
		if( (msg_type == 0) || (msg_type == 1)  || (msg_type == 2) )
		{
			MSGINFO( "ADP: Entity ID = 0x%016llx \n", convert_eui64_to_uint64_return(discover_entity_id.value));
			adp_entity_avail( discover_entity_id, msg_type );
		}
	}
}

void cmd_connect_and_disconnect_proccess( const char *opt, bool isconnect )
{
	char output_id[32] = {0};
	uint16_t output_index = 0;
	struct jdksavdecc_eui64 talker_entity_id;
	
	char input_id[32] = {0};
	uint16_t input_index = 0;
	struct jdksavdecc_eui64 listerner_entity_id;	
	uint16_t connect_cnt = 0;
	uint16_t sequence_id = 0;

	const char *p = opt;
	const char *first = p;	
	int input_flag = 0;
	while( *p != '\0' )
	{
		int copy_num = 0;
		first = p;
		for( ; (!isspace(*p)) &&  (*p != 0); p++ )
			copy_num++;
		
		if( input_flag == 0 )
		{
			memcpy( output_id, first, copy_num );
			convert_str_to_eui64( output_id, talker_entity_id.value );
			p++;
			input_flag++;
		}
		else if( input_flag == 1 )
		{
			output_index = (uint8_t)atoi(&first[0]);
			p++;
			input_flag++;
		}
		else if( input_flag == 2 )
		{
			memcpy( input_id, first, copy_num );
			convert_str_to_eui64( input_id, listerner_entity_id.value );
			p++;
			input_flag++;
		}
		else if( input_flag == 3 )
		{
			input_index = (uint8_t)atoi(&first[0]);
			p++;
			input_flag++;
		}
		else if( input_flag == 4 )
		{
			connect_cnt = (uint8_t)atoi(&first[0]);
			p++;
			input_flag++;
		}
		else if( input_flag == 5 )
		{
			sequence_id = (uint8_t)atoi(&first[0]);
			break;
		}
	}

	if( input_flag >= 4)
	{
		if( isconnect )
		{
			DEBUG_INFO( "[ %016llx %d %016llx %d %d %d ]", \
				convert_eui64_to_uint64_return(talker_entity_id.value), output_index, \
				convert_eui64_to_uint64_return(listerner_entity_id.value), input_index, connect_cnt, sequence_id );
			acmp_connect_avail( talker_entity_id.value, output_index, listerner_entity_id.value, input_index, connect_cnt, sequence_id);
		}
		else
		{
			DEBUG_INFO( "[ %016llx %d %016llx %d %d %d ]",\
				convert_eui64_to_uint64_return(talker_entity_id.value), output_index,\
				convert_eui64_to_uint64_return(listerner_entity_id.value), input_index, connect_cnt, sequence_id );
			acmp_disconnect_avail( talker_entity_id.value, output_index, listerner_entity_id.value, input_index, connect_cnt, sequence_id);
		}
	}
}

void cmd_list_proccess( void )
{
	uint16_t end_num = 0;
	uint64_t end_id = 0;
	uint64_t end_mac = 0;
	bool connect_flag = false;
	solid_pdblist end_list_node = end_list_guard->next;
	desc_pdblist desc_entity_node = NULL;
	uint8_t firmware[64] = "UNKNOWN";
	uint8_t entity_name[64] = "UNKNOWN";
	
	MSGINFO( "\nEnd Station    |  Name\t\t  |  Entity ID\t      |  Firmware    |  Mac\t\n--------------------------------------------------------------------------------------" );
	for( ; end_list_node != end_list_guard; end_list_node = end_list_node->next )
	{
		char connect_ch = 'D';
		end_num = end_list_node->solid.entity_index;
		end_id = end_list_node->solid.entity_id;
		connect_flag = end_list_node->solid.connect_flag;
		end_mac = convert_entity_id_to_uit64_mac_address( end_id );
		if( connect_flag )
			connect_ch = 'C';
		else 
			connect_ch = 'D';

		desc_entity_node = search_desc_dblist_node_no_printf_info( end_id, descptor_guard);
		if( NULL != desc_entity_node )
		{
			memcpy( firmware, desc_entity_node->endpoint_desc.firmware_version.value, sizeof(struct jdksavdecc_string));
			memcpy( entity_name, desc_entity_node->endpoint_desc.entity_name.value, sizeof(struct jdksavdecc_string));
		}
			
		MSGINFO( "%c\t%d      |  %s    |  0x%016llx    |  %s\t     |  %012llx", connect_ch,  end_num, entity_name, end_id, firmware, end_mac );
	}

	MSGINFO( "\r\nC  End Station connect\nD  End Stastion disconnect\r\n\r\n" );
}

/*===================================
*
*开始-终端发送命令测试函数
*====================================*/
void terminal_cmd_query_proccess( const char*opt )
{
	uint16_t addr = 0;
	char entity_id_str[32] = {0};
	struct jdksavdecc_eui64 entity_id;

	const char *p = opt;
	const char *first = opt;
	int input_flag = 0;
	while( *p != '\0')
	{
		int copy_num = 0;
		first = p;
		for( ; (!isspace(*p)) &&  (*p != 0); p++ )
			copy_num++;
		
		if( input_flag == 0 )
		{
			memcpy( entity_id_str, first, copy_num );
			convert_str_to_eui64( entity_id_str, entity_id.value );
			p++;
			input_flag++;
		}
		else if( input_flag == 1 )
		{
			addr = (uint16_t)atoi(&first[0]);
			p++;
			input_flag++;
		}
	}
	
	DEBUG_INFO( "query entity id = 0x%016llx addr = %04x" ,convert_eui64_to_uint64_return(entity_id.value), addr);
	terminal_query_endstation( addr, convert_eui64_to_uint64_return(entity_id.value));
}

void terminal_cmd_allot_proccess( void )
{
	terminal_allot_address();
}

void terminal_cmd_reallot_proccess( void )
{
	terminal_reallot_address();
}

void terminal_cmd_set_state_proccess( const char*opt )
{
	tmnl_state_set tmnl_state; // 终端状态
	uint8_t array_state[4] = {0};
	uint16_t addr = 0;
	char entity_id_str[32] = {0};
	char tmnl_state_str[32] = {0};
	struct jdksavdecc_eui64 entity_id;

	const char *p = opt;
	const char *first = opt;
	int input_flag = 0;
	while( *p != '\0')
	{
		int copy_num = 0;
		first = p;
		for( ; (!isspace(*p)) &&  (*p != 0); p++ )
			copy_num++;
		
		if( input_flag == 0 )
		{
			memcpy( entity_id_str, first, copy_num );
			convert_str_to_eui64( entity_id_str, entity_id.value );
			p++;
			input_flag++;
		}
		else if( input_flag == 1 )
		{
			addr = (uint16_t)atoi(&first[0]);
			p++;
			input_flag++;
		}
		else if(input_flag == 2 )
		{
			memcpy( tmnl_state_str, first, copy_num );
			convert_str_to_eui64( tmnl_state_str, array_state );
			memcpy(&tmnl_state, array_state, 4 );
			p++;
			input_flag++;
		}
	}
	
	DEBUG_INFO( "query entity id = 0x%016llx addr = %d " , convert_eui64_to_uint64_return(entity_id.value), addr );
	terminal_state_set(tmnl_state, addr, convert_eui64_to_uint64_return(entity_id.value));
}

void terminal_cmd_set_mic_proccess( const char*opt )
{
	uint16_t addr = 0;
	char entity_id_str[32] = {0};
	struct jdksavdecc_eui64 entity_id;
	uint8_t mic_status = 0;

	const char *p = opt;
	const char *first = opt;
	int input_flag = 0;
	while( *p != '\0')
	{
		int copy_num = 0;
		first = p;
		for( ; (!isspace(*p)) &&  (*p != 0); p++ )
			copy_num++;
		
		if( input_flag == 0 )
		{
			memcpy( entity_id_str, first, copy_num );
			convert_str_to_eui64( entity_id_str, entity_id.value );
			p++;
			input_flag++;
		}
		else if( input_flag == 1 )
		{
			addr = (uint16_t)atoi(&first[0]);
			p++;
			input_flag++;
		}
		else if( input_flag == 2 )
		{
			mic_status = (uint8_t)atoi(&first[0]);
			p++;
			input_flag++;
		}
	}
	
	DEBUG_INFO( "query entity id = 0x%016llx addr = %d, mic status = %02x" ,convert_eui64_to_uint64_return(entity_id.value), addr, mic_status );
	terminal_set_mic_status( mic_status, addr, convert_eui64_to_uint64_return(entity_id.value));
}

void terminal_cmd_set_indicator_proccess(const char*opt)
{
	uint16_t addr = 0;
	char entity_id_str[32] = {0};
	struct jdksavdecc_eui64 entity_id;
	uint16_t lamp_status=0;

	const char *p = opt;
	const char *first = opt;
	int input_flag = 0;
	while( *p != '\0')
	{
		int copy_num = 0;
		first = p;
		for( ; (!isspace(*p)) &&  (*p != 0); p++ )
			copy_num++;
		
		if( input_flag == 0 )
		{
			memcpy( entity_id_str, first, copy_num );
			convert_str_to_eui64( entity_id_str, entity_id.value );
			p++;
			input_flag++;
		}
		else if( input_flag == 1 )
		{
			addr = (uint16_t)atoi(&first[0]);
			p++;
			input_flag++;
		}
		else if( input_flag == 2 )
		{
			lamp_status = (uint16_t)atoi(&first[0]);
			p++;
			input_flag++;
		}
	}
	
	DEBUG_INFO( "query entity id = 0x%016llx addr = %d, lamp_status = %04x" ,convert_eui64_to_uint64_return(entity_id.value), addr, lamp_status);
	ttmnl_led_lamp lamp;
	lamp.data_low = (uint8_t)(lamp_status&0xff00 >> 8);// 高八位在地址
	lamp.data_high = (uint8_t)(lamp_status&0x00ff >> 0);
	
	terminal_set_indicator_lamp(lamp, addr, convert_eui64_to_uint64_return(entity_id.value));
}

void terminal_cmd_new_allot_proccess(const char*opt)
{
	char entity_id_str[32] = {0};
	struct jdksavdecc_eui64 entity_id;

	const char *p = opt;
	const char *first = opt;
	int input_flag = 0;
	while( *p != '\0')
	{
		int copy_num = 0;
		first = p;
		for( ; (!isspace(*p)) &&  (*p != 0); p++ )
			copy_num++;
		
		if( input_flag == 0 )
		{
			memcpy( entity_id_str, first, copy_num );
			convert_str_to_eui64( entity_id_str, entity_id.value );
			p++;
			input_flag++;
			break;
		}
	}
	
	DEBUG_INFO( "query entity id = 0x%016llx " ,convert_eui64_to_uint64_return(entity_id.value));
	terminal_new_endstation_allot_address( convert_eui64_to_uint64_return(entity_id.value));
}

void terminal_cmd_set_lcd_proccess(const char*opt)
{
	uint16_t addr = 0;
	char entity_id_str[32] = {0};
	struct jdksavdecc_eui64 entity_id;
	uint8_t lcd_stype = 0;

	const char *p = opt;
	const char *first = opt;
	int input_flag = 0;
	while( *p != '\0')
	{
		int copy_num = 0;
		first = p;
		for( ; (!isspace(*p)) &&  (*p != 0); p++ )
			copy_num++;
		
		if( input_flag == 0 )
		{
			memcpy( entity_id_str, first, copy_num );
			convert_str_to_eui64( entity_id_str, entity_id.value );
			p++;
			input_flag++;
		}
		else if( input_flag == 1 )
		{
			addr = (uint16_t)atoi(&first[0]);
			p++;
			input_flag++;
		}
		else if( input_flag == 2 )
		{
			lcd_stype = (uint8_t)atoi(&first[0]);
			p++;
			input_flag++;
		}
	}
	
	DEBUG_INFO( "query entity id = 0x%016llx addr = %d, lcd_stype = %02x" ,convert_eui64_to_uint64_return(entity_id.value), addr, lcd_stype);
	terminal_set_lcd_play_stype(  convert_eui64_to_uint64_return(entity_id.value), addr, lcd_stype);
}

void terminal_cmd_set_led_proccess(const char*opt)
{
	uint16_t addr = 0;
	char entity_id_str[32] = {0};
	struct jdksavdecc_eui64 entity_id;
	tmnl_led_state_show_set led_stype;
	uint16_t set_led = 0;
	
	const char *p = opt;
	const char *first = opt;
	int input_flag = 0;
	while( *p != '\0')
	{
		int copy_num = 0;
		first = p;
		for( ; (!isspace(*p)) &&  (*p != 0); p++ )
			copy_num++;
		
		if( input_flag == 0 )
		{
			memcpy( entity_id_str, first, copy_num );
			convert_str_to_eui64( entity_id_str, entity_id.value );
			p++;
			input_flag++;
		}
		else if( input_flag == 1 )
		{
			addr = (uint16_t)atoi(&first[0]);
			p++;
			input_flag++;
		}
		else if( input_flag == 2 )
		{
			set_led = (uint16_t)atoi(&first[0]);
			memcpy( &led_stype, &set_led,2); // 从低位开始拷贝，拷贝到对应的低位
			p++;
			input_flag++;
		}
	}
	
	DEBUG_INFO( "query entity id = 0x%016llx addr = %d, [led_stype = %04x ( stop_time = %02x, speed_roll = %02x, page_show_state = %02x, bright_lv = %02x, blink = %02x )]" ,convert_eui64_to_uint64_return(entity_id.value), addr, set_led, led_stype.stop_time,led_stype.speed_roll,led_stype.page_show_state,led_stype.bright_lv,led_stype.blink);
	terminal_set_led_play_stype( convert_eui64_to_uint64_return(entity_id.value), addr, led_stype);
}

void terminal_cmd_chairman_control_proccess( const char*opt )
{
	uint16_t addr = 0;
	char entity_id_str[32] = {0};
	struct jdksavdecc_eui64 entity_id;
	uint8_t meeting_set = 0;
	
	const char *p = opt;
	const char *first = opt;
	int input_flag = 0;
	while( *p != '\0')
	{
		int copy_num = 0;
		first = p;
		for( ; (!isspace(*p)) &&  (*p != 0); p++ )
			copy_num++;
		
		if( input_flag == 0 )
		{
			memcpy( entity_id_str, first, copy_num );
			convert_str_to_eui64( entity_id_str, entity_id.value );
			p++;
			input_flag++;
		}
		else if( input_flag == 1 )
		{
			addr = (uint16_t)atoi(&first[0]);
			p++;
			input_flag++;
		}
		else if( input_flag == 2 )
		{
			meeting_set = (uint8_t)atoi(&first[0]);
			p++;
			input_flag++;
		}
	}
	
	DEBUG_INFO( "query entity id = 0x%016llx addr = %d, meeting_set = %02x" ,convert_eui64_to_uint64_return(entity_id.value), addr, meeting_set);
	terminal_chairman_control_meeting( convert_eui64_to_uint64_return(entity_id.value), addr, meeting_set);
}

void terminal_cmd_query_vote_result_proccess( const char*opt )
{
	uint16_t addr = 0;
	char entity_id_str[32] = {0};
	struct jdksavdecc_eui64 entity_id;

	char vote_re_tmp_str[32] = {0};
	tmnl_vote_result vote_rslt;
	uint8_t vote_re_tmp[8]  = {0};
	
	const char *p = opt;
	const char *first = opt;
	int input_flag = 0;
	while( *p != '\0')
	{
		int copy_num = 0;
		first = p;
		for( ; (!isspace(*p)) &&  (*p != 0); p++ )
			copy_num++;
		
		if( input_flag == 0 )
		{
			memcpy( entity_id_str, first, copy_num );
			convert_str_to_eui64( entity_id_str, entity_id.value );
			p++;
			input_flag++;
		}
		else if( input_flag == 1 )
		{
			addr = (uint16_t)atoi(&first[0]);
			p++;
			input_flag++;
		}
		else if( input_flag == 2 )
		{
			memcpy( vote_re_tmp_str, first, copy_num );
			convert_str_to_eui64( vote_re_tmp_str, vote_re_tmp );
			vote_rslt.total = (((uint16_t)vote_re_tmp[0]) << 8) |(((uint16_t)vote_re_tmp[1]) << 0);
			vote_rslt.abs = (((uint16_t)vote_re_tmp[2]) << 8) |(((uint16_t)vote_re_tmp[3]) << 0);
			vote_rslt.neg = (((uint16_t)vote_re_tmp[4]) << 8) |(((uint16_t)vote_re_tmp[5]) << 0);
			vote_rslt.aff = (((uint16_t)vote_re_tmp[6]) << 8) |(((uint16_t)vote_re_tmp[7]) << 0);
			p++;
			input_flag++;
		}
	}
	
	DEBUG_INFO( "query entity id = 0x%016llx addr = %d, [ vote result (total = %04x, abs = %04x, neg = %04x,aff = %04x) ]" ,convert_eui64_to_uint64_return(entity_id.value), addr, vote_rslt.total, vote_rslt.abs,vote_rslt.neg,vote_rslt.aff);
	terminal_send_vote_result( convert_eui64_to_uint64_return(entity_id.value), addr, vote_rslt);
}

void terminal_cmd_limit_spk_time_proccess(const char *opt)
{
	uint16_t addr = 0;
	char entity_id_str[32] = {0};
	struct jdksavdecc_eui64 entity_id;
	uint8_t spk_time  = 0;
	tmnl_limit_spk_time spk_time_set;
	
	const char *p = opt;
	const char *first = opt;
	int input_flag = 0;
	while( *p != '\0')
	{
		int copy_num = 0;
		first = p;
		for( ; (!isspace(*p)) &&  (*p != 0); p++ )
			copy_num++;
		
		if( input_flag == 0 )
		{
			memcpy( entity_id_str, first, copy_num );
			convert_str_to_eui64( entity_id_str, entity_id.value );
			p++;
			input_flag++;
		}
		else if( input_flag == 1 )
		{
			addr = (uint16_t)atoi(&first[0]);
			p++;
			input_flag++;
		}
		else if( input_flag == 2 )
		{
			spk_time = (uint8_t)atoi(&first[0]);
			spk_time_set.limit_time = spk_time;
			p++;
			input_flag++;
		}
	}
	
	DEBUG_INFO( "query entity id = 0x%016llx addr = %02x, spk_time = %d" ,convert_eui64_to_uint64_return(entity_id.value), addr, spk_time_set.limit_time);
	terminal_limit_spk_time( convert_eui64_to_uint64_return(entity_id.value), addr, spk_time_set);
}

void terminal_cmd_host_send_state_proccess(const char *opt)
{
	char entity_id_str[32] = {0};
	struct jdksavdecc_eui64 entity_id;
	
	tmnl_main_state_send state_send;
	memset( &state_send, 0, sizeof(tmnl_main_state_send));
	state_send.apply = 10;
	state_send.apply_set = 10;
	state_send.spk_num = 4;
	state_send.limit = 10;
	state_send.conference_stype = 3;
	
	const char *p = opt;
	const char *first = opt;
	int input_flag = 0;
	while( *p != '\0')
	{
		int copy_num = 0;
		first = p;
		for( ; (!isspace(*p)) &&  (*p != 0); p++ )
			copy_num++;
		
		if( input_flag == 0 )
		{
			memcpy( entity_id_str, first, copy_num );
			convert_str_to_eui64( entity_id_str, entity_id.value );
			p++;
			input_flag++;
		}
		else if( input_flag == 1 )
		{
			state_send.unit = (uint8_t)atoi(&first[0]); // 接入终端的总数
			p++;
			input_flag++;
		}
	}
	
	DEBUG_INFO( "query entity id = 0x%016llx  conference_stype = %d" , convert_eui64_to_uint64_return(entity_id.value), state_send.conference_stype );
	terminal_host_send_state( convert_eui64_to_uint64_return(entity_id.value), state_send);
}

void terminal_cmd_send_end_lcd_proccess(const char *opt)
{
	uint16_t addr = 0;
	char entity_id_str[32] = {0};
	struct jdksavdecc_eui64 entity_id;
	tmnl_send_end_lcd_display lcd_dis;
	uint8_t lcd_opt = 0;
	uint8_t lcd_num = 0;
	
	const char *p = opt;
	const char *first = opt;
	int input_flag = 0;
	while( *p != '\0')
	{
		int copy_num = 0;
		first = p;
		for( ; (!isspace(*p)) &&  (*p != 0); p++ )
			copy_num++;
		
		if( input_flag == 0 )
		{
			memcpy( entity_id_str, first, copy_num );
			convert_str_to_eui64( entity_id_str, entity_id.value );
			p++;
			input_flag++;
		}
		else if( input_flag == 1 )
		{
			addr = (uint16_t)atoi(&first[0]);
			p++;
			input_flag++;
		}
		else if( input_flag == 2 )
		{
			lcd_opt = (uint8_t)atoi(&first[0]);
			lcd_dis.opt = lcd_opt;
			p++;
			input_flag++;
		}
		else if( input_flag == 3 )
		{
			lcd_num = (uint8_t)atoi(&first[0]);
			lcd_dis.num = lcd_num;
			p++;
			input_flag++;
		}
	}
	
	DEBUG_INFO( "query entity id = 0x%016llx addr = %d, lcd_num = %d" ,convert_eui64_to_uint64_return(entity_id.value), addr, lcd_dis.num);
	terminal_send_end_lcd_display( convert_eui64_to_uint64_return(entity_id.value), addr, lcd_dis);
}

void terminal_cmd_option_endpoint_proccess( const char *opt )
{
	uint16_t addr = 0;
	char entity_id_str[32] = {0};
	struct jdksavdecc_eui64 entity_id;
	uint8_t end_opt = 0;
	
	const char *p = opt;
	const char *first = opt;
	int input_flag = 0;
	while( *p != '\0')
	{
		int copy_num = 0;
		first = p;
		for( ; (!isspace(*p)) &&  (*p != 0); p++ )
			copy_num++;
		
		if( input_flag == 0 )
		{
			memcpy( entity_id_str, first, copy_num );
			convert_str_to_eui64( entity_id_str, entity_id.value );
			p++;
			input_flag++;
		}
		else if( input_flag == 1 )
		{
			addr = (uint16_t)atoi(&first[0]);
			p++;
			input_flag++;
		}
		else if( input_flag == 2 )
		{
			end_opt = (uint8_t)atoi(&first[0]);
			p++;
			input_flag++;
		}
	}
	
	DEBUG_INFO( "query entity id = 0x%016llx addr = %d, end_opt = %d" ,convert_eui64_to_uint64_return(entity_id.value), addr, end_opt);
	terminal_option_endpoint( convert_eui64_to_uint64_return(entity_id.value), addr, end_opt);
}

void terminal_cmd_special_event_reply_proccess( const char *opt )
{
	uint16_t addr = 0;
	char entity_id_str[32] = {0};
	struct jdksavdecc_eui64 entity_id;
	
	const char *p = opt;
	const char *first = opt;
	int input_flag = 0;
	while( *p != '\0')
	{
		int copy_num = 0;
		first = p;
		for( ; (!isspace(*p)) &&  (*p != 0); p++ )
			copy_num++;
		
		if( input_flag == 0 )
		{
			memcpy( entity_id_str, first, copy_num );
			convert_str_to_eui64( entity_id_str, entity_id.value );
			p++;
			input_flag++;
		} 
		else if( input_flag == 1 )
		{
			addr = (uint16_t)atoi(&first[0]);
			p++;
			input_flag++;
		}
	}

	DEBUG_INFO( "query entity id = 0x%016llx addr = %d" ,convert_eui64_to_uint64_return(entity_id.value), addr);
	terminal_endstation_special_event_reply( convert_eui64_to_uint64_return(entity_id.value), addr );
}

void terminal_cmd_tnmt_cmpt_msg_proccess( const char *opt )
{
	uint16_t addr = 0;
	char entity_id_str[32] = {0};
	struct jdksavdecc_eui64 entity_id;
	uint8_t cmpt_msg[8] = {1, 3, 3, 3, 3, 3, 3, 3 };
	uint16_t msg_len = 8;
	
	const char *p = opt;
	const char *first = opt;
	int input_flag = 0;
	while( *p != '\0')
	{
		int copy_num = 0;
		first = p;
		for( ; (!isspace(*p)) &&  (*p != 0); p++ )
			copy_num++;
		
		if( input_flag == 0 )
		{
			memcpy( entity_id_str, first, copy_num );
			convert_str_to_eui64( entity_id_str, entity_id.value );
			p++;
			input_flag++;
		}
		else if( input_flag == 1 )
		{
			addr = (uint16_t)atoi(&first[0]);
			p++;
			input_flag++;
		}
	}
	
	DEBUG_INFO( "query entity id = 0x%016llx addr = %d, cmpt_msg type = %d" ,convert_eui64_to_uint64_return(entity_id.value), addr, cmpt_msg[0]);
	terminal_transmit_upper_cmpt_message( convert_eui64_to_uint64_return(entity_id.value), addr, cmpt_msg, msg_len );
}

void terminal_cmd_reply_end_message_proccess(const char *opt )
{
	uint16_t addr = 0;
	char entity_id_str[32] = {0};
	struct jdksavdecc_eui64 entity_id;
	
	const char *p = opt;
	const char *first = opt;
	int input_flag = 0;
	while( *p != '\0')
	{
		int copy_num = 0;
		first = p;
		for( ; (!isspace(*p)) &&  (*p != 0); p++ )
			copy_num++;
		
		if( input_flag == 0 )
		{
			memcpy( entity_id_str, first, copy_num );
			convert_str_to_eui64( entity_id_str, entity_id.value );
			p++;
			input_flag++;
		}
		else if( input_flag == 1 )
		{
			addr = (uint16_t)atoi(&first[0]);
			p++;
			input_flag++;
		}
	}
	
	DEBUG_INFO( "query entity id = 0x%016llx addr = %d" ,convert_eui64_to_uint64_return(entity_id.value), addr);
	terminal_reply_end_message_command( convert_eui64_to_uint64_return(entity_id.value), addr);
}

void terminal_cmd_query_vote_sign_proccess(const char *opt)
{
	uint16_t addr = 0;
	char entity_id_str[32] = {0};
	struct jdksavdecc_eui64 entity_id;
	
	const char *p = opt;
	const char *first = opt;
	int input_flag = 0;
	while( *p != '\0')
	{
		int copy_num = 0;
		first = p;
		for( ; (!isspace(*p)) &&  (*p != 0); p++ )
			copy_num++;
		
		if( input_flag == 0 )
		{
			memcpy( entity_id_str, first, copy_num );
			convert_str_to_eui64( entity_id_str, entity_id.value );
			p++;
			input_flag++;
		}
		else if( input_flag == 1 )
		{
			addr = (uint16_t)atoi(&first[0]);
			p++;
			input_flag++;
		}
	}
	
	DEBUG_INFO( "query entity id = 0x%016llx addr = %d " ,convert_eui64_to_uint64_return(entity_id.value), addr );
	terminal_query_vote_sign_result( convert_eui64_to_uint64_return(entity_id.value), addr );
}

void cmd_terminal_proccess( const char *opt )
{
	while(1)
	{
		 char* cmd_buf = readline( "$ " );
		 if ( !cmd_buf )
	            break;
		 
	         if( strlen(cmd_buf) == 0 )
		{
			free(cmd_buf);
			continue;
		}
		else
		{
			add_history( cmd_buf );
		}

		if( strncmp( cmd_buf, "list", 4 ) == 0 || strncmp( cmd_buf, "ls", 2 ) == 0)
		{
			cmd_list_proccess();
		}
		else if((strncmp(cmd_buf, "query", 5 ) != 0)&&((strncmp(cmd_buf, "q", 1) == 0 ) || (strncmp( cmd_buf, "quit", 4) == 0)))
		{
			free(cmd_buf);
			break;
		}
		else if( strncmp( cmd_buf, "clear", 5 ) == 0 )
		{
			int ret = 0;
			ret = system("clear");
			if( ret == -1 || ret == 127 )
			{
				DEBUG_INFO( "shell exec excute failed!or system func run fork() failed!" );
				break;
			}
		}
		else if( strncmp(cmd_buf, "query", 5 ) == 0 )
		{
			terminal_cmd_query_proccess(&cmd_buf[6]);
		}
		else if( strncmp(cmd_buf, "allot", 5 ) == 0 )
		{
			terminal_cmd_allot_proccess();
		}
		else if( strncmp(cmd_buf, "reAllot", 7) == 0 )
		{
			terminal_cmd_reallot_proccess();
		}
		else if( strncmp(cmd_buf, "setState", 8) == 0 )
		{
			terminal_cmd_set_state_proccess(&cmd_buf[9]);
		}
		else if( strncmp(cmd_buf, "setMic", 6) == 0 )
		{
			terminal_cmd_set_mic_proccess(&cmd_buf[7]);
		}
		else if( strncmp(cmd_buf, "setIndicator", 12) == 0 )
		{
			terminal_cmd_set_indicator_proccess(&cmd_buf[13]);
		}
		else if( strncmp(cmd_buf, "newAllot", 8) == 0 )
		{
			terminal_cmd_new_allot_proccess(&cmd_buf[9]);
		}
		else if( strncmp(cmd_buf, "setLcd", 6) == 0 )
		{
			terminal_cmd_set_lcd_proccess(&cmd_buf[7]);
		}
		else if( strncmp(cmd_buf, "setLed", 6) == 0 )
		{
			terminal_cmd_set_led_proccess(&cmd_buf[7]);
		}
		else if( strncmp(cmd_buf, "chairManControl", 15 ) == 0 )
		{
			terminal_cmd_chairman_control_proccess(&cmd_buf[16]);
		}
		else if( strncmp(cmd_buf, "voteResult", 10) == 0 )
		{
			terminal_cmd_query_vote_result_proccess(&cmd_buf[11]);
		}
		else if( strncmp(cmd_buf, "limitSpkTime", 12) == 0 )
		{
			terminal_cmd_limit_spk_time_proccess(&cmd_buf[13]);
		}
		else if( strncmp(cmd_buf, "hostSendState", 13) == 0 )
		{
			terminal_cmd_host_send_state_proccess(&cmd_buf[14]);
		}
		else if( strncmp(cmd_buf, "sendEndLcd", 10) == 0 )
		{
			terminal_cmd_send_end_lcd_proccess(&cmd_buf[11]);
		}
		else if( strncmp(cmd_buf, "optionEndpoint", 14) == 0 )
		{
			terminal_cmd_option_endpoint_proccess(&cmd_buf[15]);
		}
		else if( strncmp(cmd_buf, "specialEventReply", 17) == 0 )
		{
			terminal_cmd_special_event_reply_proccess(&cmd_buf[18]);
		}
		else if( strncmp(cmd_buf, "tnmtCmptMsg", 11) == 0 )
		{
			terminal_cmd_tnmt_cmpt_msg_proccess(&cmd_buf[12]);
		}
		else if( strncmp(cmd_buf, "replyEndMessage", 15) == 0 )
		{
			terminal_cmd_reply_end_message_proccess(&cmd_buf[16]);
		}
		else if( strncmp(cmd_buf, "queryVoteSign", 13) == 0 )
		{
			terminal_cmd_query_vote_sign_proccess(&cmd_buf[14]);
		}
		else if(!isspace(cmd_buf[0]))
		{
			MSGINFO( "clear\nlist\nls\nq\nquit\nquery\nallot\nreAllot\nsetState\nsetMic\nsetIndicator\nnewAllot\nsetLcd\nsetled\nchairManControl\nvoteResult\nlimitSpkTime\nhostSendState\nsendEndLcd\noptionEndpoint\nspecialEventReply\ntnmtCmptMsg\nreplyEndMessage\nqueryVoteSign");
		}
		
		free(cmd_buf);
	}
}
/*===================================
*结束-终端发送命令测试函数
*====================================*/

/*===================================
*begin host and udp client command interface testing
*====================================*/
void cmd_udp_client_command_avail( const char *opt ) // 4 paramt
{
	uint8_t protocol_type = 0;
	uint32_t cmd_cmd_code = 0;
	uint32_t data_len = 0;
	char udp_client_command_str[32] = { 0 };
	uint8_t udp_client_data_payload[128] = { 0 };
	bool is_cmd_right = true;
	
	const char *p = opt;
	const char *first = opt;
	int input_flag = 0;
	while( *p != '\0')
	{
		int copy_num = 0;
		first = p;
		for( ; (!isspace(*p)) &&  (*p != 0); p++ )
			copy_num++;
		
		if( input_flag == 0 )
		{
			memcpy( udp_client_command_str, first, copy_num );
			if( get_upper_cmpt_cmd_value_from_string_name( udp_client_command_str, &cmd_cmd_code) )
			{
				p++;
				input_flag++;
				is_cmd_right = false;
			}
			else
			{
				DEBUG_INFO( "Please enter right host and uppper command !");
				is_cmd_right = false;
				break;
			}
		}
		else if( input_flag == 1 )
		{
			protocol_type = (uint8_t)atoi( &first[0] );
			p++;
			input_flag++;
			is_cmd_right = false;
		}
		else if( input_flag == 2 )
		{
			data_len = (uint16_t)atoi( &first[0] );
			p++;	// p pointer to data payload
			input_flag++;
			is_cmd_right = true;
			break;
		}
	}

	// parser payload data
	int i = 0;
	for ( i = 0; i < data_len; ++i )
        {
		if ( !jdksavdecc_util_parse_byte( &udp_client_data_payload[i], p[0], p[1] ) )
		{
			DEBUG_INFO( "Bad payload octets ascii form" );
			is_cmd_right = false;
			break;
		}
		
		p += 2;
        }

	if( is_cmd_right )
	{
		if( data_len == 0 )
		{
			send_upper_computer_command( protocol_type, cmd_cmd_code, NULL, 0 );
			DEBUG_INFO( "[ cmd code = %02x, type = %02x, data len = %d ]",cmd_cmd_code, protocol_type, data_len );
		}
		else
		{
			send_upper_computer_command( protocol_type, cmd_cmd_code, udp_client_data_payload, data_len );
			DEBUG_INFO( "[ cmd = %02x, type = %02x, data len = %d ]",cmd_cmd_code, protocol_type, data_len );
		}

		
	}
}

void cmd_udp_client( void )
{
	while(1)
	{
		char* cmd_buf = readline( "@ " );
		 if ( !cmd_buf )
	            break;
	         if ( strlen(cmd_buf) == 0 )
	         {
	         	free(cmd_buf);
			continue;
	         }
		else
		{
			add_history( cmd_buf );
		}

		if( ((strncmp(cmd_buf, "q", 1) == 0 ) || (strncmp( cmd_buf, "quit", 4) == 0)) )
		{
			free(cmd_buf);
			break;
		}
		else
		{
			cmd_udp_client_command_avail( cmd_buf );
		}
		
		free(cmd_buf);
	}
}
/*===================================
*end host and udp client command interface testing
*====================================*/

/*===================================
*开始-主机功能
*====================================*/
void cmd_host_func_command_reallot( const char *opt ) // no paramt
{
	terminal_system_reallot_addr();
}

void cmd_host_func_proccess( void )
{
	while(1)
	{
		char* cmd_buf = readline( "F " );
		if ( !cmd_buf )
	            break;
	        if ( strlen(cmd_buf) == 0 )
		{
			free(cmd_buf);
			continue;
	        }
		else
		{
			add_history( cmd_buf );
		}

		if( strncmp(cmd_buf, "reAllot", 7) == 0)
		{
			cmd_host_func_command_reallot( cmd_buf );
		}
		else if( ((strncmp(cmd_buf, "q", 1) == 0 ) || (strncmp( cmd_buf, "quit", 4) == 0)) )
		{
			free(cmd_buf);
			break;
		}
		else
		{
			MSGINFO( "\nreAllot\n");
		}

		free(cmd_buf);
	}
}

void cmd_matrix_control_proccess( void )
{
	int cmd = 0;
	enum_matrix_command matrix_cmd = 0xff;
	int int_input = 0;
	int int_output_num = 0;
	int int_output[MATRIX_OUTPUT_NUM]={0};
	uint8_t uoutput[MATRIX_OUTPUT_NUM]={0};
	char sw_string_buf[256] = {0};
	
	while( 1 )
	{
		MSGINFO( "\nEnter follow num to control matrix (0 to exit control)\n" );
		MSGINFO( "\t1:av矩阵切换  2:视频矩阵切换:  3:音频矩阵切换\n" );
		MSGINFO( "\t3:锁定键盘  5:解开键盘的锁定  6:关闭蜂鸣器\n" );
		MSGINFO( "\t7:打开蜂鸣器  8:查询软件版本  9:设置兼容指令系统\n" );
		MSGINFO( "\t10:设置creator2.0指令系统  11:关闭串口  12:打开串口\n" );
		MSGINFO( "\t13:设置lcd背光时间  14:查询矩阵型号  15:修改矩阵的密码\n" );
		MSGINFO( "\t16:关闭所有输出通道  17:设置通道对应输出 18:查询输出的输入状态\n" );
		MSGINFO( "\t0:退去矩阵控制,返回主菜单\n" );

		MSGINFO( ">>" );
		if( scanf( "%d", &cmd ) != 1 )
		{
			MSGINFO( "wrong enter cmd: Will Return to Main menu" );
			return;
		}

		MSGINFO( "Your Enter command-->%d", cmd );
		switch( cmd )
		{
			case 1:
				if( cmd == 1 )
					matrix_cmd = MATRIX_AV_SWITCH;
			case 2:
				if( cmd == 2 )
					matrix_cmd = MATRIX_VIDEO_SWITCH;
			case 3:
				int_input = 0;
				int_output_num = 0;
				memset( int_output, 0, sizeof(int_output));
				
				// choose to enter control num or strings
				MSGINFO("Select 1 or 2 to choose control Stype\n");
				int ctl_num;
				scanf( "%d", &ctl_num );
				if( ctl_num == 2 )
				{
					memset( sw_string_buf, 0, sizeof( sw_string_buf));
					scanf( "%s", sw_string_buf );
					control_matrix_switch( sw_string_buf, strlen(sw_string_buf));
				}
				else
				{
					MSGINFO( "step 1: Enter input channal (1-%d)", MATRIX_INPUT_NUM );
					while( (int_input == 0) || (int_input > MATRIX_OUTPUT_NUM))
					{
						if( scanf( "%d", &int_input ) != 1 )
						{
							MSGINFO( "wrong enter param: Will Return to Main menu" );
							return;
						}
						
						if( (int_input > MATRIX_OUTPUT_NUM ) || (int_input <=0))
							MSGINFO( "input num (%d)out of range!Please enter right num",int_input );
					}

					MSGINFO( "step 2: Enter output number (1-%d) <--input = %d", MATRIX_OUTPUT_NUM, int_input );
					while( (int_output_num == 0) || (int_output_num > MATRIX_OUTPUT_NUM))
					{
						if( scanf( "%d", (int*)&int_output_num ) != 1 )
						{
							MSGINFO( "wrong enter param: Will Return to Main menu" );
							return;
						}

						if( (int_output_num > MATRIX_OUTPUT_NUM ) || (int_output_num) <=0 )
							MSGINFO( "output num (%d)out of range!Please enter right num", int_output_num );
					}
					
					MSGINFO( "step 3(end):Enter output channal (1-%d)", MATRIX_OUTPUT_NUM );
					int i = 0;
					for( i = 0; i < int_output_num; i++ )
					{
						while( (int_output[i] == 0) || (int_output[i] > MATRIX_OUTPUT_NUM))
						{
							if( scanf( "%d", (int*)&int_output[i] ) != 1 )
							{
								MSGINFO( "wrong enter param: Will Return to Main menu" );
								return;
							}
							
							if( (int_output[i] > MATRIX_OUTPUT_NUM ) || (int_output[i]) <=0 )
								MSGINFO( "output channal (%d)out of range!Please enter right num. Or have same output", int_output[i] );
						}
					}

					if( cmd == 3 )
						matrix_cmd = MATRIX_AUDIO_SWITCH;

					DEBUG_INFO( "input = %d, output_num = %d", int_input, int_output_num );
					for( i = 0; i < int_output_num; i++ )
						uoutput[i] = (int)int_output[i];
					if( -1 != control_matrix_input_output_switch( matrix_cmd, (uint8_t)int_input, uoutput, (uint8_t)int_output_num ))
					{
						MSGINFO( "switch Success" );
					}
					else
					{
						MSGINFO( "switch failed: input = %d, output_num %d", int_input, int_output_num );
					}
				}
				break;
			case 4:
				break;
			case 5:
				break;
			case 6:
				break;
			case 7:
				break;
			case 8:
				break;
			case 9:
				break;
			case 10:
				break;
			case 11:
				break;
			case 12:
				break;
			case 13:
				break;
			case 14:
				break;
			case 15:
				break;
			case 16:
				break;
			case 17:
				break;
			case 18:
				break;
			case 0:
				MSGINFO( "Return to main menu\n" );
				return;
			default:
				MSGINFO( " %d out of range comand,Please enter Again \n", cmd );
				break;
		}
	}
}

// *********************************************
// *********系统菜单功能接口的测试开始****************
// *********************************************
void	parser_comand_line( char *in_cmd, char out_cmd[][CMD_OPTION_STRING_LEN] )// 注:函数使用命令行参数最好不要使用两个空格隔开，否则会出现参数使用不正确的情况
{
	uint16_t i = 0, j = 0, num = 0, cmd_len = 0;
	uint16_t space_num = 0;

	assert( NULL != in_cmd );
	if( NULL == in_cmd )
		return;

	cmd_len = strlen( in_cmd );
	for( i = 0; i < cmd_len; i++ )
	{
		if( in_cmd[i] != ' ' )
		{// 不为空格
			out_cmd[num][j] =  in_cmd[i];
			j++;
			space_num = 0;
		}
		else if( in_cmd[i] == ' ' )
		{// 不空格
			if( space_num == 0 )
				num++;
			
			space_num++;
			j = 0;
		}
	}
}

void cmd_menu_control_proccess( void )
{
	char treated_cmd[CMD_OPTION_MAX_NUM][CMD_OPTION_STRING_LEN] = {{0}};

	MSGINFO( "Welcome to test menu func, Please enter \"help\" to see menu command Usage!(quit to exit)" );
	while( 1 )
	{
		char* cmd_buf = readline( ">>  " );
	        if ( !cmd_buf )
	            break;
	        if ( strlen(cmd_buf) == 0 )
	        {
	        	free(cmd_buf);
	        	continue;
	        }
		else
		{
			add_history( cmd_buf );
		}
		
		if( strncmp( cmd_buf, "quit", 4 ) == 0 )
		{
			if( cmd_buf != NULL )
				free( cmd_buf );
			
			return;
		}
		else if( strncmp( cmd_buf, "clear", 5 ) == 0 )
		{
			system("clear");
			if( cmd_buf != NULL )
				free( cmd_buf );
			
			continue;
		}

		memset( treated_cmd, 0, sizeof(treated_cmd) );
		parser_comand_line( cmd_buf, treated_cmd );
		if( cmd_buf != NULL )
			free( cmd_buf );
		
		menu_cmd_line_run( treated_cmd );
	}
}

// *********************************************
// *********系统菜单功能接口的测试结束****************
// *********************************************

/*===================================
*结束-主机功能
*====================================*/

extern struct threads_info threads;
void controller_proccess( void )
{
	end_list_guard = endpoint_list;
	initialize_readline(); // init command list
	
	while( 1 )
	{
	        char* cmd_buf = readline( "# " );
	        if ( !cmd_buf )
	            break;
	        if ( strlen(cmd_buf) == 0 )
	        {
	        	free(cmd_buf);
	        	continue;
	        }
		else
		{
			add_history( cmd_buf );
		}
		
		if( strncmp( cmd_buf, "list", 4 ) == 0 || strncmp( cmd_buf, "ls", 2 ) == 0 )
		{
			cmd_list_proccess();
		}
		else if( strncmp( cmd_buf, "adp", 3 ) == 0 )
		{
			cmd_adp_proccess( cmd_buf ); // fmt adp [ msg type] [0x----------------]
		}
		else if( strncmp( cmd_buf, "clear", 5 ) == 0 )
		{
			int ret = 0;
			ret = system("clear");
			if( ret == -1 || ret == 127 )
			{
				DEBUG_INFO( "shell exec excute failed!or system func run fork() failed!" );
				free(cmd_buf);
				break;
			}
		}
		else if( strncmp( cmd_buf, "connect", 7 ) == 0 )
		{
			cmd_connect_and_disconnect_proccess( &cmd_buf[8], true ); // 跳过空格 
		}
		else if( strncmp( cmd_buf, "disconnect", 10 ) == 0 )
		{
			cmd_connect_and_disconnect_proccess( &cmd_buf[11], false );
		}
		else if( strncmp( cmd_buf, "show", 4 ) == 0 )	// show endstations connections status
		{
			cmd_show_proccess();
		}
		else if( strncmp( cmd_buf, "update", 6 ) == 0 )// update system endstations connections
		{
			cmd_update_proccess();
		}
		else if( strncmp( cmd_buf, "terminal", 8 ) == 0 ) // endstations commnad test process(测试发送终端命令)
		{
			cmd_terminal_proccess( cmd_buf );
		}
		else if( strncmp( cmd_buf, "hostFunc", 8 ) == 0 ) // 控制器功能
		{
			cmd_host_func_proccess();
		}
		else if( strncmp( cmd_buf, "udpClient", 9) == 0 ) // test procces by sending host and upper computer command to udp client
		{
			cmd_udp_client();
		}
		else if( strncmp( cmd_buf, "matrixControl", 13) == 0 ) // test procces by sending host and upper computer command to udp client
		{
			cmd_matrix_control_proccess();
		}
		else if( strncmp( cmd_buf, "MenuTest", 8 ) == 0 )
		{
			cmd_menu_control_proccess();
		}
		else if( ((strncmp(cmd_buf, "query", 5 ) != 0)&&(strncmp(cmd_buf, "queryVoteSign", 13) != 0))&&((strncmp(cmd_buf, "q", 1) == 0 ) || (strncmp( cmd_buf, "quit", 4) == 0)) )
		{
			free(cmd_buf);
			system_close( &threads );
			break;
		}
		else if(!isspace(cmd_buf[0]))
		{
			MSGINFO( "adp\nclear\nconnect\ndisconnect\nhostFunc\nlist\nupdate\nudpClient\nq\nquit\nshow\nterminal\nmatrixControl\nMenuTest\n");
		}

		free(cmd_buf); // free command
	}
}


