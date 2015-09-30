#include "controller_command.h"
#include "acmp_controller_machine.h"
#include "adp_controller_machine.h"
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>

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
	uint64_t entity_entity_id = 0;
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
	
	MSGINFO( "\nEnd Station    |  Entity ID\t\t|  Mac\t\n-------------------------------------------------------------------------------" );
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
		
		MSGINFO( "%c\t%d      |  0x%016llx\t | %012llx", connect_ch, end_num, end_id, end_mac );
	}

	MSGINFO( "\r\nC  End Station connect\nD  End Stastion disconnect\r\n\r\n" );
}


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
		else if((strncmp(cmd_buf, "q", 1) == 0) || (strncmp( cmd_buf, "quit", 4) == 0))
		{
			exit(1);
		}
		else if(!isspace(cmd_buf[0]))
		{
			MSGINFO( "adp\nclear\nconnect\ndisconnect\nlist\nupdate\nq\nquit\nshow\n");
			continue;
		}

		free(cmd_buf); // free command
	}
}


