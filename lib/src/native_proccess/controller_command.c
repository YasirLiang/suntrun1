#include "controller_command.h"
#include "acmp_controller_machine.h"
#include "adp_controller_machine.h"
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "terminal_pro.h"
#include "terminal_command.h"
#include "descriptor.h"

static solid_pdblist end_list_guard = NULL;

/*====================²¹È«ÃüÁî===============*/
#define COMMANDS_MAX_NUM 64
static char *commands_list[COMMANDS_MAX_NUM] = 
{
	"adp",
	"connect",
	"clear",
	"disconnect",
	"list",
	"ls",
	"update" ,
	"q",
	"quit",
	"show",
	// ¿ªÊ¼-²âÊÔÖÕ¶ËÃüÁîµÄÃüÁî
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
	// ½áÊø-²âÊÔÖÕ¶ËÃüÁîµÄÃüÁî
	
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
*¿ªÊ¼-ÖÕ¶Ë·¢ËÍÃüÁî²âÊÔº¯Êý
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
	tmnl_state_set tmnl_state; // ÖÕ¶Ë×´Ì¬
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
	uint8_t mic_status;

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
	
	DEBUG_INFO( "query entity id = 0x%016llx addr = %d, mic status = %02x" ,convert_eui64_to_uint64_return(entity_id.value), addr, mic_status);
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
	terminal_set_indicator_lamp(lamp_status, addr, convert_eui64_to_uint64_return(entity_id.value));
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
			memcpy( &led_stype, &set_led,2); // ´ÓµÍÎ»¿ªÊ¼¿½±´£¬¿½±´µ½¶ÔÓ¦µÄµÍÎ»
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
	
	DEBUG_INFO( "query entity id = 0x%016llx addr = %d, spk_time = %d" ,convert_eui64_to_uint64_return(entity_id.value), addr, spk_time_set.limit_time);
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
			state_send.unit = (uint8_t)atoi(&first[0]); // ½ÓÈëÖÕ¶ËµÄ×ÜÊý
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
	         if ( strlen(cmd_buf) == 0 )
	            continue;
		else
		{
			add_history( cmd_buf );
		}

		if( strncmp( cmd_buf, "list", 4 ) == 0 || strncmp( cmd_buf, "ls", 2 ) == 0)
		{
			cmd_list_proccess();
			continue;
		}
		else if((strncmp(cmd_buf, "query", 5 ) != 0)&&((strncmp(cmd_buf, "q", 1) == 0 ) || (strncmp( cmd_buf, "quit", 4) == 0)))
		{
			break;
		}
		else if( strncmp( cmd_buf, "clear", 5 ) == 0 )
		{
			system("clear");
			continue;
		}
		else if( strncmp(cmd_buf, "query", 5 ) == 0 )
		{
			terminal_cmd_query_proccess(&cmd_buf[6]);
			continue;
		}
		else if( strncmp(cmd_buf, "allot", 5 ) == 0 )
		{
			terminal_cmd_allot_proccess();
			continue;
		}
		else if( strncmp(cmd_buf, "reAllot", 7) == 0 )
		{
			terminal_cmd_reallot_proccess();
			continue;
		}
		else if( strncmp(cmd_buf, "setState", 8) == 0 )
		{
			terminal_cmd_set_state_proccess(&cmd_buf[9]);
			continue;
		}
		else if( strncmp(cmd_buf, "setMic", 6) == 0 )
		{
			terminal_cmd_set_mic_proccess(&cmd_buf[7]);
			continue;
		}
		else if( strncmp(cmd_buf, "setIndicator", 12) == 0 )
		{
			terminal_cmd_set_indicator_proccess(&cmd_buf[13]);
			continue;
		}
		else if( strncmp(cmd_buf, "newAllot", 8) == 0 )
		{
			terminal_cmd_new_allot_proccess(&cmd_buf[9]);
			continue;
		}
		else if( strncmp(cmd_buf, "setLcd", 6) == 0 )
		{
			terminal_cmd_set_lcd_proccess(&cmd_buf[7]);
			continue;
		}
		else if( strncmp(cmd_buf, "setLed", 6) == 0 )
		{
			terminal_cmd_set_led_proccess(&cmd_buf[7]);
			continue;
		}
		else if( strncmp(cmd_buf, "chairManControl", 15 ) == 0 )
		{
			terminal_cmd_chairman_control_proccess(&cmd_buf[16]);
			continue;
		}
		else if( strncmp(cmd_buf, "voteResult", 10) == 0 )
		{
			terminal_cmd_query_vote_result_proccess(&cmd_buf[11]);
			continue;
		}
		else if( strncmp(cmd_buf, "limitSpkTime", 12) == 0 )
		{
			terminal_cmd_limit_spk_time_proccess(&cmd_buf[13]);
			continue;
		}
		else if( strncmp(cmd_buf, "hostSendState", 13) == 0 )
		{
			terminal_cmd_host_send_state_proccess(&cmd_buf[14]);
			continue;
		}
		else if( strncmp(cmd_buf, "sendEndLcd", 15) == 0 )
		{
			terminal_cmd_send_end_lcd_proccess(&cmd_buf[16]);
			continue;
		}
		else if( strncmp(cmd_buf, "optionEndpoint", 14) == 0 )
		{
			terminal_cmd_option_endpoint_proccess(&cmd_buf[15]);
			continue;
		}
		else if( strncmp(cmd_buf, "specialEventReply", 17) == 0 )
		{
			terminal_cmd_special_event_reply_proccess(&cmd_buf[18]);
			continue;
		}
		else if( strncmp(cmd_buf, "tnmtCmptMsg", 11) == 0 )
		{
			terminal_cmd_tnmt_cmpt_msg_proccess(&cmd_buf[12]);
			continue;
		}
		else if( strncmp(cmd_buf, "replyEndMessage", 15) == 0 )
		{
			terminal_cmd_reply_end_message_proccess(&cmd_buf[16]);
			continue;
		}
		else if( strncmp(cmd_buf, "queryVoteSign", 13) == 0 )
		{
			terminal_cmd_query_vote_sign_proccess(&cmd_buf[14]);
			continue;
		}
		else if(!isspace(cmd_buf[0]))
		{
			MSGINFO( "clear\nlist\nls\nq\nquit\nquery\nallot\nreAllot\nsetState\nsetMic\nsetIndicator\nnewAllot\nsetLcd\nsetled\nchairManControl\nvoteResult\nlimitSpkTime\nhostSendState\nsendEndLcd\noptionEndpoint\nspecialEventReply\ntnmtCmptMsg\nreplyEndMessage\nqueryVoteSign");
			continue;
		}
		
		 free(cmd_buf);
	}
}
/*===================================
*½áÊø-ÖÕ¶Ë·¢ËÍÃüÁî²âÊÔº¯Êý
*====================================*/

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
	            continue;
		else
		{
			add_history( cmd_buf );
		}
		
		if( strncmp( cmd_buf, "list", 4 ) == 0 || strncmp( cmd_buf, "ls", 2 ) == 0)
		{
			cmd_list_proccess();
			continue;
		}
		else if( strncmp( cmd_buf, "adp", 3 ) == 0 )
		{
			cmd_adp_proccess( cmd_buf ); // fmt adp [ msg type] [0x----------------]
			continue;
		}
		else if( strncmp( cmd_buf, "clear", 3 ) == 0 )
		{
			system("clear");
			continue;
		}
		else if( strncmp( cmd_buf, "connect", 7 ) == 0 )
		{
			cmd_connect_and_disconnect_proccess( &cmd_buf[8], true ); // Ìø¹ý¿Õ¸ñ 
			continue;
		}
		else if( strncmp( cmd_buf, "disconnect", 10 ) == 0 )
		{
			cmd_connect_and_disconnect_proccess( &cmd_buf[11], false );
			continue;
		}
		else if(strncmp( cmd_buf, "show", 4 ) == 0)	// show endstations connections status
		{
			cmd_show_proccess();
			continue;
		}
		else if(strncmp( cmd_buf, "update", 6 ) == 0)// update system endstations connections
		{
			cmd_update_proccess();
			continue;
		}
		else if( strncmp( cmd_buf, "terminal", 8 ) == 0) // endstations commnad test process(²âÊÔ·¢ËÍÖÕ¶ËÃüÁî)
		{
			cmd_terminal_proccess(cmd_buf);
			continue;
		}
		else if(((strncmp(cmd_buf, "query", 5 ) != 0)&&(strncmp(cmd_buf, "queryVoteSign", 13) != 0))&&((strncmp(cmd_buf, "q", 1) == 0 ) || (strncmp( cmd_buf, "quit", 4) == 0)))
		{
			exit(1);
		}
		else if(!isspace(cmd_buf[0]))
		{
			MSGINFO( "adp\nclear\nconnect\ndisconnect\nlist\nupdate\nq\nquit\nshow\nterminal\n");
			continue;
		}

		free(cmd_buf); // free command
	}
}


