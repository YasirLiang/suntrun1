#include "util.h"
#include "jdksavdecc_util.h"
#include "system_packet_tx.h"
#include "pelco_d.h"
#include "protocal_qt.h"

const char *aem_cmds_names[] =
{
        "ACQUIRE_ENTITY",
        "LOCK_ENTITY",
        "ENTITY_AVAILABLE",
        "CONTROLLER_AVAILABLE",
        "READ_DESCRIPTOR",
        "WRITE_DESCRIPTOR",
        "SET_CONFIGURATION",
        "GET_CONFIGURATION",
        "SET_STREAM_FORMAT",
        "GET_STREAM_FORMAT",
        "SET_VIDEO_FORMAT",
        "GET_VIDEO_FORMAT",
        "SET_SENSOR_FORMAT",
        "GET_SENSOR_FORMAT",
        "SET_STREAM_INFO",
        "GET_STREAM_INFO",
        "SET_NAME",
        "GET_NAME",
        "SET_ASSOCIATION_ID",
        "GET_ASSOCIATION_ID",
        "SET_SAMPLING_RATE",
        "GET_SAMPLING_RATE",
        "SET_CLOCK_SOURCE",
        "GET_CLOCK_SOURCE",
        "SET_CONTROL",
        "GET_CONTROL",
        "INCREMENT_CONTROL",
        "DECREMENT_CONTROL",
        "SET_SIGNAL_SELECTOR",
        "GET_SIGNAL_SELECTOR",
        "SET_MIXER",
        "GET_MIXER",
        "SET_MATRIX",
        "GET_MATRIX",
        "START_STREAMING",
        "STOP_STREAMING",
        "REGISTER_UNSOLICITED_NOTIFICATION",
        "DEREGISTER_UNSOLICITED_NOTIFICATION",
        "IDENTIFY_NOTIFICATION",
        "GET_AVB_INFO",
        "GET_AS_PATH",
        "GET_COUNTERS",
        "REBOOT",
        "GET_AUDIO_MAP",
        "ADD_AUDIO_MAPPINGS",
        "REMOVE_AUDIO_MAPPINGS",
        "GET_VIDEO_MAP",
        "ADD_VIDEO_MAPPINGS",
        "REMOVE_VIDEO_MAPPINGS",
        "GET_SENSOR_MAP",
        "ADD_SENSOR_MAPPINGS",
        "REMOVE_SENSOR_MAPPINGS",
        "START_OPERATION",
        "ABORT_OPERATION",
        "OPERATION_STATUS",
        "AUTH_ADD_KEY",
        "AUTH_DELETE_KEY",
        "AUTH_GET_KEY_LIST",
        "AUTH_GET_KEY",
        "AUTH_ADD_KEY_TO_CHAIN",
        "AUTH_DELETE_KEY_FROM_CHAIN",
        "AUTH_GET_KEYCHAIN_LIST",
        "AUTH_GET_IDENTITY",
        "AUTH_ADD_TOKEN",
        "AUTH_DELETE_TOKEN",
        "AUTHENTICATE",
        "DEAUTHENTICATE",
        "ENABLE_TRANSPORT_SECURITY",
        "DISABLE_TRANSPORT_SECURITY",
        "ENABLE_STREAM_ENCRYPTION",
        "DISABLE_STREAM_ENCRYPTION",
        "SET_MEMORY_OBJECT_LENGTH",
        "GET_MEMORY_OBJECT_LENGTH",
        "SET_STREAM_BACKUP",
        "GET_STREAM_BACKUP"
};

const char *aem_descs_names[] =
{
        "ENTITY",
   	"CONFIGURATION",
        "AUDIO_UNIT",
        "VIDEO_UNIT",
        "SENSOR_UNIT",
        "STREAM_INPUT",
        "STREAM_OUTPUT",
        "JACK_INPUT",
        "JACK_OUTPUT",
        "AVB_INTERFACE",
        "CLOCK_SOURCE",
        "MEMORY_OBJECT",
        "LOCALE",
        "STRINGS",
        "STREAM_PORT_INPUT",
        "STREAM_PORT_OUTPUT",
        "EXTERNAL_PORT_INPUT",
        "EXTERNAL_PORT_OUTPUT",
        "INTERNAL_PORT_INPUT",
        "INTERNAL_PORT_OUTPUT",
        "AUDIO_CLUSTER",
        "VIDEO_CLUSTER",
        "SENSOR_CLUSTER",
        "AUDIO_MAP",
        "VIDEO_MAP",
        "SENSOR_MAP",
        "CONTROL",
        "SIGNAL_SELECTOR",
        "MIXER",
        "MATRIX",
        "MATRIX_SIGNAL",
        "SIGNAL_SPLITTER",
        "SIGNAL_COMBINER",
        "SIGNAL_DEMULTIPLEXER",
        "SIGNAL_MULTIPLEXER",
        "SIGNAL_TRANSCODER",
        "CLOCK_DOMAIN",
        "CONTROL_BLOCK"
};

const char *aem_cmds_status_names[] =
{
	"SUCCESS", // AEM_STATUS_SUCCESS
	"NOT_IMPLEMENTED", // AEM_STATUS_NOT_IMPLEMENTED
	"NO_SUCH_DESCRIPTOR", // AEM_STATUS_NO_SUCH_DESCRIPTOR
	"ENTITY_LOCKED", // AEM_STATUS_ENTITY_LOCKED
	"ENTITY_ACQUIRED", // AEM_STATUS_ENTITY_ACQUIRED
	"NOT_AUTHENTICATED", // AEM_STATUS_NOT_AUTHENTICATED
	"AUTHENTICATION_DISABLED ", // AEM_STATUS_AUTHENTICATION_DISABLED
	"BAD_ARGUMENTS", // AEM_STATUS_BAD_ARGUMENTS
	"STATUS_NO_RESOURCES", // STATUS_NO_RESOURCES
	"IN_PROGRESS", // AEM_STATUS_IN_PROGRESS
	"ENTITY_MISBEHAVING", // AEM_STATUS_ENTITY_MISBEHAVING
	"NOT_SUPPORTED", // AEM_STATUS_NOT_SUPPORTED
	"STREAM_IS_RUNNING", // AEM_STATUS_STREAM_IS_RUNNING
};

const char *acmp_cmds_names[] =
{
	"CONNECT_TX_COMMAND",
	"CONNECT_TX_RESPONSE",
	"DISCONNECT_TX_COMMAND",
	"DISCONNECT_TX_RESPONSE",
	"GET_TX_STATE_COMMAND",
	"GET_TX_STATE_RESPONSE",
	"CONNECT_RX_COMMAND",
	"CONNECT_RX_RESPONSE",
	"DISCONNECT_RX_COMMAND",
	"DISCONNECT_RX_RESPONSE",
	"GET_RX_STATE_COMMAND",
	"GET_RX_STATE_RESPONSE",
	"GET_TX_CONNECTION_COMMAND",
	"GET_TX_CONNECTION_RESPONSE"
};

const char *acmp_cmds_status_names[] =
{
	"SUCCESS", // ACMP_STATUS_SUCCESS
	"LISTENER_UNKNOWN_ID", // ACMP_STATUS_LISTENER_UNKNOWN_ID
	"TALKER_UNKNOWN_ID", // ACMP_STATUS_TALKER_UNKNOWN_ID
	"TALKER_DEST_MAC_FAIL", // ACMP_STATUS_TALKER_DEST_MAC_FAIL
	"TALKER_NO_STREAM_INDEX", // ACMP_STATUS_TALKER_NO_STREAM_INDEX
	"TALKER_NO_BANDWIDTH", // ACMP_STATUS_TALKER_NO_BANDWIDTH
	"TALKER_EXCLUSIVE", // ACMP_STATUS_TALKER_EXCLUSIVE
	"LISTENER_TALKER_TIMEOUT", // ACMP_STATUS_LISTENER_TALKER_TIMEOUT
	"LISTENER_EXCLUSIVE", // ACMP_STATUS_LISTENER_EXCLUSIVE
	"STATE_UNAVAILABLE", // ACMP_STATUS_STATE_UNAVAILABLE
	"NOT_CONNECTED", // ACMP_STATUS_NOT_CONNECTED
	"NO_SUCH_CONNECTION", // ACMP_STATUS_NO_SUCH_CONNECTION
	"COULD_NOT_SEND_MESSAGE", // ACMP_STATUS_COULD_NOT_SEND_MESSAGE
	"TALKER_MISBEHAVING", // ACMP_STATUS_TALKER_MISBEHAVING
	"LISTENER_MISBEHAVING", // ACMP_STATUS_LISTENER_MISBEHAVING
	"RESERVED", // ACMP_STATUS_RESERVED
	"CONTROLLER_NOT_AUTHORIZED", // ACMP_STATUS_CONTROLLER_NOT_AUTHORIZED
	"INCOMPATIBLE_REQUEST", // ACMP_STATUS_INCOMPATIBLE_REQUEST
	"LISTENER_INVALID_CONNECTION" // ACMP_STATUS_LISTENER_INVALID_CONNECTION
};

const char *notification_names[] =
{
	"NO_MATCH_FOUND",
	"END_STATION_CONNECTED",
	"END_STATION_DISCONNECTED",
	"COMMAND_TIMEOUT",
	"RESPONSE_RECEIVED",
	"END_STATION_READ_COMPLETED"
};

const char *logging_level_names[] =
{
	"ERROR", // LOGGING_LEVEL_ERROR
	"WARNING", // LOGGING_LEVEL_WARNING
	"NOTICE", // LOGGING_LEVEL_NOTICE
	"INFO", // LOGGING_LEVEL_INFO
	"DEBUG", // LOGGING_LEVEL_DEBUG
	"VERBOSE" // LOGGING_LEVEL_VERBOSE
};

struct ieee1722_format
{
	uint64_t fmt;
	const char *str;
};

struct ieee1722_format ieee1722_format_table[] =
{
	{UINT64_C(0x00a0020140000100), "IEC...48KHZ_1CH"}, // IEC61883_AM824_MBLA_48KHZ_1CH
	{UINT64_C(0x00a0020240000200), "IEC...48KHZ_2CH"}, // IEC61883_AM824_MBLA_48KHZ_2CH
	{UINT64_C(0x00a0020440000400), "IEC...48KHZ_4CH"}, // IEC61883_AM824_MBLA_48KHZ_4CH
	{UINT64_C(0x00a0020840000800), "IEC...48KHZ_8CH"}, // IEC61883_AM824_MBLA_48KHZ_8CH
	{UINT64_C(0x00a0040140000100), "IEC...96KHZ_1CH"}, // IEC61883_AM824_MBLA_96KHZ_1CH
	{UINT64_C(0x00a0040240000200), "IEC...96KHZ_2CH"}, // IEC61883_AM824_MBLA_96KHZ_2CH
	{UINT64_C(0x00a0040440000400), "IEC...96KHZ_4CH"}, // IEC61883_AM824_MBLA_96KHZ_4CH
	{UINT64_C(0x00a0040840000800), "IEC...96KHZ_8CH"}, // IEC61883_AM824_MBLA_96KHZ_8CH
	{UINT64_C(0x0000000000000000), "UNKNOWN"}
};

struct jdksavdecc_uint16_name jdksavdecc_adpdu_message_type[] = 
{
       {ADP_MESSAGE_TYPE_ENTITY_AVAILABLE,"ENTITY_AVAILABLE"},
       {ADP_MESSAGE_TYPE_ENTITY_DEPARTING,"ENTITY_DEPARTING"},
       {ADP_MESSAGE_TYPE_ENTITY_DISCOVER,  "ENTITY_DISCOVER"},
       {ADP_ERROR_MESSAGE_TYEP, "NULL"}
};

bool get_adpdu_msg_type_value_with_str(const char* msg_str, uint16_t *msg_code )
{
	struct jdksavdecc_uint16_name *p = &jdksavdecc_adpdu_message_type[0];
	while( p->value != ADP_ERROR_MESSAGE_TYEP )
	{
		if( strcmp( msg_str, p->name ) == 0)
		{
			*msg_code = p->value;
			return true;
		}
		p++;
	}

	return false;
}

typedef struct TCmdCorrespString {
    uint32_t cmd;
    const char * string_valus;
}TCmdCorrespString;

typedef struct TCmdCorrespTimeout {
    uint32_t cmd;
    uint32_t timeout_ms;    
}TCmdCorrespTmt;

TCmdCorrespString const host_and_end_string_values[] = {
    {QUERY_END , "QUERY_END"},
    {ALLOCATION , "ALLOCATION"},
    {SET_END_STATUS, "SET_END_STATUS"},
    {REALLOCATION, "REALLOCATION"},
    {KEYPAD_ACTION, "KEYPAD_ACTION"},
    {SET_ENDLIGHT , "SET_ENDLIGHT"},
    {NEW_ALLOCATION, "NEW_ALLOCATION"},
    {END_ASSIGN,"END_ASSIGN"},
    {SET_END_LCD, "SET_END_LCD"},
    {COMMON_SEND_END_LCD, "COMMON_SEND_END_LCD"},
    {SET_END_LED, "SET_END_LED"},
    {COMMON_SEND_END_LED, "COMMON_SEND_END_LED"},
    {PRESIDENT_CONTROL, "PRESIDENT_CONTROL"},
    {SEND_VOTE_RESULT, "SEND_VOTE_RESULT"},
    {TALKTIME_LEN , "TALKTIME_LEN"},
    {HOST_SEND_STATUS, "HOST_SEND_STATUS"},
    {SEND_END_LCD_DISPLAY , "SEND_END_LCD_DISPLAY"},
    {OPTITION_END, "OPTITION_END"},
    {SET_MIS_STATUS, "SET_MIS_STATUS"},
    {END_SPETHING, "END_SPETHING"},
    {CHECK_END_RESULT , "CHECK_END_RESULT"},
    {TRANSIT_HOST_MSG, "TRANSIT_HOST_MSG"},
    {TRANSIT_END_MSG, "TRANSIT_END_MSG"},
    {CONFERENCE_HOST_AND_END_CMD_ERROR, "UNKNOW"}
};

const char * get_host_and_end_conference_string_value(uint8_t cmd) {
    TCmdCorrespString const *p = &host_and_end_string_values[0];
    while (p->cmd != CONFERENCE_HOST_AND_END_CMD_ERROR) {
        if (p->cmd == cmd) {
        	return p->string_valus;
        }
        p++;
    }

    return "UNKNOW";
}

TCmdCorrespTmt host_and_endstation_commands_timeout_tables[]=
{
	{ QUERY_END, QUERY_END_TIMEOUTS },
	{ ALLOCATION, ALLOCATION_TIMEOUTS},
	{ SET_END_STATUS, SET_END_STATUS_TIMEOUTS},
	{ REALLOCATION, REALLOCATION_TIMEOUTS},
	{ KEYPAD_ACTION, KEYPAD_ACTION_TIMEOUTS},
	{ SET_ENDLIGHT, SET_ENDLIGHT_TIMEOUTS},
	{ NEW_ALLOCATION, NEW_ALLOCATION_TIMEOUTS},
	{ END_ASSIGN, END_ASSIGN_TIMEOUTS},
	{ SET_END_LCD, SET_END_LCD_TIMEOUTS},
	{ COMMON_SEND_END_LCD, COMMON_SEND_END_LCD_TIMEOUTS},
	{ SET_END_LED, SET_END_LED_TIMEOUTS},
	{ COMMON_SEND_END_LED, COMMON_SEND_END_LED_TIMEOUTS},
	{ PRESIDENT_CONTROL, PRESIDENT_CONTROL_TIMEOUTS},
	{ SEND_VOTE_RESULT, SEND_VOTE_RESULT_TIMEOUTS},
	{TALKTIME_LEN, TALKTIME_LEN_TIMEOUTS},
	{ HOST_SEND_STATUS, HOST_SEND_STATUS_TIMEOUTS},
	{ SEND_END_LCD_DISPLAY, SEND_END_LCD_DISPLAY_TIMEOUTS},
	{ OPTITION_END, OPTITION_END_TIMEOUTS},
	{ SET_MIS_STATUS, SET_MIS_STATUS_TIMEOUTS},
	{ END_SPETHING, END_SPETHING_TIMEOUTS},
	{ CHECK_END_RESULT, CHECK_END_RESULT_TIMEOUTS},
	{ TRANSIT_HOST_MSG, TRANSIT_HOST_MSG_TIMEOUTS},
	{ TRANSIT_END_MSG, TRANSIT_END_MSG_TIMEOUTS},
	{ CONFERENCE_HOST_AND_END_CMD_ERROR, 0xffff}
};

uint32_t get_host_endstation_command_timeout( uint32_t cmd_value )
{
	TCmdCorrespTmt *p = &host_and_endstation_commands_timeout_tables[0];

	while( p->cmd != CONFERENCE_HOST_AND_END_CMD_ERROR )
	{
		if( p->cmd == cmd_value )
		{
			return p->timeout_ms;
		}

		p++;
	}

	return (uint32_t)0xffff;
}

TCmdCorrespTmt udp_upper_cmpt_command_and_timeout_table[] =
{
	{DISCUSSION_PARAMETER, CONFERENCE_DISCUSSION_PARAMETER_TIMEOUTS},
	{MISCROPHONE_SWITCH,MISCROPHONE_SWITCH_TIMEOUTS},
	{MISCROPHONE_STATUS,MISCROPHONE_STATUS_TIMEOUTS},
	{SELECT_PROPOSER,SELECT_PROPOSER_TIMEOUTS},
	{EXAMINE_APPLICATION,EXAMINE_APPLICATION_TIMEOUTS},
	{CONFERENCE_PERMISSION,CONFERENCE_PERMISSION_TIMEOUTS},
	{SENDDOWN_MESSAGE,SENDDOWN_MESSAGE_TIMEOUTS},
	{BEGIN_SIGN,BEGIN_SIGN_TIMEOUTS},
	{SIGN_SITUATION,SIGN_SITUATION_TIMEOUTS},
	{END_OF_SIGN,END_OF_SIGN_TIMEOUTS},
	{ENDSTATION_ALLOCATION_APPLICATION_ADDRESS,ENDSTATION_ALLOCATION_APPLICATION_ADDRESS_TIMEOUTS},
	{ENDSTATION_REGISTER_STATUS,ENDSTATION_REGISTER_STATUS_TIMEOUTS},
	{CURRENT_VIDICON,CURRENT_VIDICON_TIMEOUTS},
	{ENDSTATION_ADDRESS_UNDETERMINED_ALLOCATION,ENDSTATION_ADDRESS_UNDETERMINED_ALLOCATION_TIMEOUTS},
	{VIDICON_CONTROL,VIDICON_CONTROL_TIMEOUTS},
	{VIDICON_PRERATION_SET,VIDICON_PRERATION_SET_TIMEOUTS},
	{VIDICON_LOCK,VIDICON_LOCK_TIMEOUTS},
	{VIDICON_OUTPUT,VIDICON_OUTPUT_TIMEOUTS},
	{BEGIN_VOTE,BEGIN_VOTE_TIMEOUTS},
	{PAUSE_VOTE,PAUSE_VOTE_TIMEOUTS},
	{REGAIN_VOTE,REGAIN_VOTE_TIMEOUTS},
	{END_VOTE,END_VOTE_TIMEOUTS},
	{RESULT_VOTE,RESULT_VOTE_TIMEOUTS},
	{TRANSMIT_TO_ENDSTATION,TRANSMIT_TO_ENDSTATION_TIMEOUTS},
	{REPORT_ENDSTATION_MESSAGE,REPORT_ENDSTATION_MESSAGE_TIMEOUTS},
	{HIGH_DEFINITION_SWITCH_SET,HIGH_DEFINITION_SWITCH_SET_TIMEOUTS},
	{HOST_AND_UPPER_CMPT_CMD_ERROR,0xffff}
};

TCmdCorrespString const udp_upper_cmpt_command_table[] =
{
	{ DISCUSSION_PARAMETER, "DISCUSSION_PARAMETER" },
	{ MISCROPHONE_SWITCH, "MISCROPHONE_SWITCH"},
	{ MISCROPHONE_STATUS, "MISCROPHONE_STATUS" },
	{ SELECT_PROPOSER, "SELECT_PROPOSER" },
	{ EXAMINE_APPLICATION, "EXAMINE_APPLICATION" },
	{ CONFERENCE_PERMISSION, "CONFERENCE_PERMISSION" },
	{ SENDDOWN_MESSAGE, "SENDDOWN_MESSAGE" },
	{ TABLE_TABLET_STANDS_MANAGER, "TABLE TABLET_STANDS MANAGER" },
	{ BEGIN_SIGN, "BEGIN_SIGN" },
	{ SIGN_SITUATION, "SIGN_SITUATION" },
	{ END_OF_SIGN, "END_OF_SIGN" },
	{ ENDSTATION_ALLOCATION_APPLICATION_ADDRESS, "ENDSTATION_ALLOCATION_APPLICATION_ADDRESS" },
	{ ENDSTATION_REGISTER_STATUS, "ENDSTATION_REGISTER_STATUS" },
	{ CURRENT_VIDICON, "CURRENT_VIDICON" },
	{ ENDSTATION_ADDRESS_UNDETERMINED_ALLOCATION, "ENDSTATION_ADDRESS_UNDETERMINED_ALLOCATION" },
	{ VIDICON_CONTROL, "VIDICON_CONTROL" },
	{ VIDICON_PRERATION_SET, "VIDICON_PRERATION_SET" },
	{ VIDICON_LOCK, "VIDICON_LOCK" },
	{ VIDICON_OUTPUT, "VIDICON_OUTPUT" },
	{ BEGIN_VOTE, "BEGIN_VOTE" },
	{ PAUSE_VOTE, "PAUSE_VOTE" },
	{ REGAIN_VOTE, "REGAIN_VOTE" },
	{ END_VOTE, "END_VOTE" },
	{ RESULT_VOTE, "RESULT_VOTE" },
	{ TRANSMIT_TO_ENDSTATION, "TRANSMIT_TO_ENDSTATION" },
	{ REPORT_ENDSTATION_MESSAGE, "REPORT_ENDSTATION_MESSAGE" },
	{ HIGH_DEFINITION_SWITCH_SET, "HIGH_DEFINITION_SWITCH_SET" },
	{ HOST_AND_UPPER_CMPT_CMD_ERROR, "UNKNOW" },
};

TCmdCorrespTmt acmp_command_and_timeout_table[] =
{
	{CONNECT_TX_COMMAND, ACMP_CONNECT_TX_COMMAND_TIMEOUT_MS},
        {DISCONNECT_TX_COMMAND, ACMP_DISCONNECT_TX_COMMAND_TIMEOUT_MS},
        {GET_TX_STATE_COMMAND, ACMP_GET_TX_STATE_COMMAND_TIMEOUT_MS},
        {CONNECT_RX_COMMAND, ACMP_CONNECT_RX_COMMAND_TIMEOUT_MS},
        {DISCONNECT_RX_COMMAND, ACMP_DISCONNECT_RX_COMMAND_TIMEOUT_MS},
        {GET_RX_STATE_COMMAND, ACMP_GET_RX_STATE_COMMAND_TIMEOUT_MS},
        {GET_TX_CONNECTION_COMMAND, ACMP_GET_TX_CONNECTION_COMMAND_TIMEOUT_MS},
        {AEM_ACMP_ERROR, 0xffff}
};

const char* get_aem_command_string( uint16_t cmd_values )
{
	if( cmd_values < TOTAL_NUM_OF_AEM_CMDS)
		return aem_cmds_names[cmd_values];
	else 
		return "UNKNOW";
}

const char* get_aem_desc_command_string( uint16_t desc_value )
{
	if( desc_value < TOTAL_NUM_OF_AEM_DESCS)
		return aem_descs_names[desc_value];
	else 
		return "UNKNOW";
}

const char * aem_cmd_value_to_name(uint16_t cmd_value)
{
	if(cmd_value < TOTAL_NUM_OF_AEM_CMDS)
	{
		return aem_cmds_names[cmd_value];
	}

	return "UNKNOWN";
}

const char *  aem_desc_value_to_name(uint16_t desc_value)
{
	if(desc_value < TOTAL_NUM_OF_AEM_DESCS)
	{
		return aem_descs_names[desc_value];
	}

	return "UNKNOWN";
}

const char * aem_cmd_status_value_to_name(uint32_t aem_cmd_status_value)
{
	if(aem_cmd_status_value < TOTAL_NUM_OF_AEM_CMDS_STATUS)
	{
		return aem_cmds_status_names[aem_cmd_status_value];
	}
	else if(aem_cmd_status_value == AVDECC_LIB_STATUS_INVALID)
	{
		return "AVDECC_LIB_STATUS_INVALID";
	}
	else if(aem_cmd_status_value == AVDECC_LIB_STATUS_TICK_TIMEOUT)
	{
		return "AVDECC_LIB_STATUS_TICK_TIMEOUT";
	}

	return "UNKNOWN";
}

const char* aecp_vendor_unique_status_value_to_name( uint32_t vendor_statue_value )
{
	if( vendor_statue_value <= JDKSAVDECC_AECP_VENDOR_STATUS_NOT_IMPLEMENTED)
		return jdksavdecc_aecp_vendor_print_status[vendor_statue_value].name;
	else 
		return "UNKNOWN";
}

const char * acmp_cmd_value_to_name( uint32_t cmd_value )
{
	if(cmd_value < TOTAL_NUM_OF_ACMP_CMDS)
	{
		return acmp_cmds_names[cmd_value];
	}

	return "UNKNOWN";
}

const char * acmp_cmd_status_value_to_name( uint32_t acmp_cmd_status_value )
{
	if(acmp_cmd_status_value < TOTAL_NUM_OF_ACMP_CMDS_STATUS)
	{
		return acmp_cmds_status_names[acmp_cmd_status_value];
	}

	return "UNKNOWN";
}

uint32_t get_udp_client_timeout_table( uint8_t msg_type )
{
	TCmdCorrespTmt *p = &udp_upper_cmpt_command_and_timeout_table[0];

	while(p->cmd != HOST_AND_UPPER_CMPT_CMD_ERROR )
	{
		if( p->cmd == msg_type )
		{
			return p->timeout_ms;
		}
			
		p++;
	}

	return (uint32_t)0xffff;
}

const char *upper_cmpt_cmd_value_to_string_name( uint8_t cmd_value )
{
	TCmdCorrespString const *p = &udp_upper_cmpt_command_table[0];

	while( p->cmd != HOST_AND_UPPER_CMPT_CMD_ERROR )
	{
		if( p->cmd == cmd_value )
			return p->string_valus;
		p++;
	}

	return "UNKNOW";
}

bool get_upper_cmpt_cmd_value_from_string_name(const char* msg_str, uint32_t *msg_code )
{
	TCmdCorrespString const *p = &udp_upper_cmpt_command_table[0];
	while( p->cmd != HOST_AND_UPPER_CMPT_CMD_ERROR )
	{
		if( strcmp( msg_str, p->string_valus) == 0)
		{
			*msg_code = p->cmd;
			return true;
		}
		
		p++;
	}

	return false;
}

uint32_t get_acmp_timeout( uint8_t msg_type )
{
	TCmdCorrespTmt *p = &acmp_command_and_timeout_table[0];

	while(p->cmd != AEM_ACMP_ERROR)
	{
		if( p->cmd == msg_type )
			return p->timeout_ms;

		p++;
	}

	return (uint32_t)0xffff;
}

static TCmdCorrespString const l_arcsCmdStrTable[] = {
    {QT_QUEUE_ID, "QT_QUEUE_ID"},
    {QT_SWITCH_MATRIX, "QT_SWITCH_MATRIX"},
    {QT_OPT_TMNL, "QT_OPT_TMNL"},
    {QT_SYS_SET, "QT_SYS_SET"},
    {QT_CMR_CTL, "QT_SYS_SET"},
    {0xffffffff, "UNKOWN"}
};

static TCmdCorrespTmt const l_arcsCmdTimeTable[] = {
    {QT_QUEUE_ID, QT_QUEUE_ID_MS},
    {QT_SWITCH_MATRIX, QT_SWITCH_MATRIX_MS},
    {QT_OPT_TMNL, QT_OPT_TMNL_MS},
    {QT_SYS_SET, QT_SYS_SET_MS},
    {QT_CMR_CTL, QT_CMR_CTL_MS},
    {0xffffffff, 0xffffffff}
};

const char *getArcsCmdString(uint32_t cmd_value) {
    TCmdCorrespString const *p = &l_arcsCmdStrTable[0];
    while (p->cmd != 0xffffffff) {
    if (p->cmd == cmd_value) {
            return p->string_valus;
        }
        p++;
    }

    return "UNKNOW";
}

uint32_t getArcsCmdTimeout(uint32_t cmd_value) {
    TCmdCorrespTmt const *p = &l_arcsCmdTimeTable[0];
    while (p->cmd != AEM_ACMP_ERROR) {
        if (p->cmd == cmd_value) {
            return p->timeout_ms;
        }
        p++;
    }

    return (uint32_t)0xffffffff;
}


uint8_t conference_command_type_read( void *base, uint16_t offerset )
{
	uint8_t *p = ((uint8_t *)base) + offerset;
	return (p[1]&0xff);// 读8位
}

uint16_t conferenc_terminal_read_address_data( void *base, uint16_t offerset )
{
	uint8_t *p = ((uint8_t *)base) + offerset;
	return ((uint16_t)((p[2]&0xff) << 0))|((uint16_t)((p[3]&0xff) << 8));	// 地址是在负载中低字节在前
}

bool is_conference_deal_data_response_type( void *base, uint16_t offset )
{
	uint8_t *p = ((uint8_t *)base) + offset;
	return (p[0]&0x80);	//位8 
}

bool is_terminal_command( void *base, uint16_t offset )
{
	uint8_t *p = ((uint8_t *)base) + offset;
	return (p[1]&0x80);	//位8 
}

bool is_terminal_response( void *base, uint16_t offset )
{
	uint8_t *p = ((uint8_t *)base) + offset;
	return (p[1]&0x40);	//位7
}

// 将48位转换为64位
void convert_eui48_to_uint64( const uint8_t value[6], uint64_t *new_value )
{
	*new_value = 0;
	uint32_t i = 0;

	assert(new_value);
	for( i = 0; i < 6; i++)
	{
		(*new_value) |= (uint64_t) value[i] << (5 - i) * 8;
	}
}

void convert_uint64_to_eui48(const uint64_t value, uint8_t new_value[6])
{
	uint32_t i = 0;
	for (i = 0; i < 6; i++)
	{
		new_value[i] = (uint8_t) (value >> ((5 - i) * 8));
	}
}

void convert_eui64_to_uint64( const uint8_t value[8], uint64_t * new_value )
{
	*new_value = 0;
	uint32_t i = 0;

	assert(new_value);
	for( i = 0; i < 8; i++)
	{
		// 注意：这里先强制类型转换
		*new_value |= (uint64_t)value[i] << (7 - i) * 8;
	}
}

void convert_uint64_to_eui64( uint8_t dest_out[8], const uint64_t in )
{
	int i = 0;
	for( ; i < 8; i++)
	{
		dest_out[i] = (uint8_t)(in >>  (7-i)*8 );
	}
}

uint64_t convert_eui64_to_uint64_return( const uint8_t value[8] )
{
	uint32_t i = 0;
	uint64_t new_value = 0;

	for( i = 0; i < 8; i++)
	{
		// 注意：这里先强制类型转换
		new_value |= (uint64_t)value[i] << (7 - i) * 8;
	}

	return new_value;
}


// convert entity id to 数组 mac address 
void convert_entity_id_to_eui48_mac_address( uint64_t entity_id, uint8_t value[6] )
{
	int i = 0;

	for( i = 0; i < 6; i++ )
	{
		if( i  < 3)
			value[i] = (uint8_t)( entity_id >> (56 - i * 8) ) ;
		else
			value[i] = (uint8_t)( entity_id >> (56 - ( i  + 2 )* 8) ) ;
	}
}

// convert entity id to mac address()
uint64_t convert_entity_id_to_uit64_mac_address( uint64_t entity_id )
{
	int i = 0;
	uint8_t value[6] = { 0 };
	uint64_t mac = 0;

	for( i = 0; i < 6; i++ )
	{
		if( i  < 3)
			value[i] = (uint8_t)( entity_id >> (56 - i * 8) ) ;
		else
			value[i] = (uint8_t)( entity_id >> (56 - ( i  + 2 )* 8) ) ;
	}

	for( i = 0; i < 6; i++ )
		mac |=  ((uint64_t)value[i]) << (40 - i * 8);

	return mac;
}

int convert_str_to_eui64( char *in, uint8_t output[8] )	// p 的格式如0x0011223344556677
{
	char *p = in + 2;
	int r = 0;
	
	r = jdksavdecc_util_parse_byte( &output[0], p[0], p[1] );
	r = jdksavdecc_util_parse_byte( &output[1], p[2], p[3] );
	r = jdksavdecc_util_parse_byte( &output[2], p[4], p[5] );
	r = jdksavdecc_util_parse_byte( &output[3], p[6], p[7] ); 
	r = jdksavdecc_util_parse_byte( &output[4], p[8], p[9] ); 
	r = jdksavdecc_util_parse_byte( &output[5], p[10], p[11] ); 
	r = jdksavdecc_util_parse_byte( &output[6], p[12], p[13] );
	r = jdksavdecc_util_parse_byte( &output[7], p[14], p[15] );

	return r;
}

void convert_str_to_eui32( char *in, uint8_t output[4] )	// p 的格式如0x00112233
{
	char *p = in + 2;
	
	jdksavdecc_util_parse_byte( &output[0], p[0], p[1] );
	jdksavdecc_util_parse_byte( &output[1], p[2], p[3] );
	jdksavdecc_util_parse_byte( &output[2], p[4], p[5] );
	jdksavdecc_util_parse_byte( &output[3], p[6], p[7] ); 
}


// compare two mac address 
bool is_mac_equal( const uint8_t mac_value1[6], const uint8_t mac_value2[6] )
{
	int i = 0;
	for( i = 0; i < 6; i++ )
	{
		if( mac_value1[i] != mac_value2[i] )
			return false;
	}

	return true;
}

struct response_data_interval_util
{
	uint32_t type; // 是管道传输的数据类型
	uint32_t command;
	uint32_t timeout;
};

struct response_data_interval_util response_data_interval_util_tables[] = 
{
	{ TRANSMIT_TYPE_AECP, QUERY_END, QUERY_END_TIMEOUTS },
	{ TRANSMIT_TYPE_AECP, ALLOCATION, ALLOCATION_TIMEOUTS },
	{ TRANSMIT_TYPE_AECP, SET_END_STATUS, SET_END_STATUS_TIMEOUTS },
	{ TRANSMIT_TYPE_AECP, REALLOCATION, REALLOCATION_TIMEOUTS },
	{ TRANSMIT_TYPE_AECP, KEYPAD_ACTION, KEYPAD_ACTION_TIMEOUTS },
	{ TRANSMIT_TYPE_AECP, SET_ENDLIGHT, SET_ENDLIGHT_TIMEOUTS },
	{ TRANSMIT_TYPE_AECP, NEW_ALLOCATION, NEW_ALLOCATION_TIMEOUTS },
	{ TRANSMIT_TYPE_AECP, END_ASSIGN, END_ASSIGN_TIMEOUTS },
	{ TRANSMIT_TYPE_AECP, SET_END_LCD, SET_END_LCD_TIMEOUTS },
	{ TRANSMIT_TYPE_AECP, COMMON_SEND_END_LCD, COMMON_SEND_END_LCD_TIMEOUTS },
	{ TRANSMIT_TYPE_AECP, SET_END_LED, SET_END_LED_TIMEOUTS },
	{ TRANSMIT_TYPE_AECP, COMMON_SEND_END_LED, COMMON_SEND_END_LED_TIMEOUTS },
	{ TRANSMIT_TYPE_AECP, PRESIDENT_CONTROL, PRESIDENT_CONTROL_TIMEOUTS },
	{ TRANSMIT_TYPE_AECP, SEND_VOTE_RESULT, SEND_VOTE_RESULT_TIMEOUTS },
	{ TRANSMIT_TYPE_AECP, TALKTIME_LEN, TALKTIME_LEN_TIMEOUTS },
	{ TRANSMIT_TYPE_AECP, HOST_SEND_STATUS, HOST_SEND_STATUS_TIMEOUTS },
	{ TRANSMIT_TYPE_AECP, SEND_END_LCD_DISPLAY, SEND_END_LCD_DISPLAY_TIMEOUTS },
	{ TRANSMIT_TYPE_AECP, OPTITION_END, OPTITION_END_TIMEOUTS },
	{ TRANSMIT_TYPE_AECP, SET_MIS_STATUS, SET_MIS_STATUS_TIMEOUTS },
	{ TRANSMIT_TYPE_AECP, END_SPETHING, END_SPETHING_TIMEOUTS },
	{ TRANSMIT_TYPE_AECP, CHECK_END_RESULT, CHECK_END_RESULT_TIMEOUTS },
	{ TRANSMIT_TYPE_AECP, TRANSIT_HOST_MSG, TRANSIT_HOST_MSG_TIMEOUTS },
	{ TRANSMIT_TYPE_AECP, TRANSIT_END_MSG, TRANSIT_END_MSG_TIMEOUTS },
	{ TRANSMIT_TYPE_UDP_CLT, DISCUSSION_PARAMETER, CONFERENCE_DISCUSSION_PARAMETER_TIMEOUTS },
	{ TRANSMIT_TYPE_UDP_CLT, MISCROPHONE_SWITCH, MISCROPHONE_SWITCH_TIMEOUTS },
	{ TRANSMIT_TYPE_UDP_CLT, MISCROPHONE_STATUS, MISCROPHONE_STATUS_TIMEOUTS },
	{ TRANSMIT_TYPE_UDP_CLT, SELECT_PROPOSER, SELECT_PROPOSER_TIMEOUTS },
	{ TRANSMIT_TYPE_UDP_CLT, EXAMINE_APPLICATION, EXAMINE_APPLICATION_TIMEOUTS },
	{ TRANSMIT_TYPE_UDP_CLT, CONFERENCE_PERMISSION, CONFERENCE_PERMISSION_TIMEOUTS },
	{ TRANSMIT_TYPE_UDP_CLT, SENDDOWN_MESSAGE, SENDDOWN_MESSAGE_TIMEOUTS },
	{ TRANSMIT_TYPE_UDP_CLT, BEGIN_SIGN, BEGIN_SIGN_TIMEOUTS },
	{ TRANSMIT_TYPE_UDP_CLT, SIGN_SITUATION, SIGN_SITUATION_TIMEOUTS },
	{ TRANSMIT_TYPE_UDP_CLT, END_OF_SIGN, END_OF_SIGN_TIMEOUTS },
	{ TRANSMIT_TYPE_UDP_CLT, ENDSTATION_ALLOCATION_APPLICATION_ADDRESS, ENDSTATION_ALLOCATION_APPLICATION_ADDRESS_TIMEOUTS },
	{ TRANSMIT_TYPE_UDP_CLT, ENDSTATION_REGISTER_STATUS, ENDSTATION_REGISTER_STATUS_TIMEOUTS },
	{ TRANSMIT_TYPE_UDP_CLT, CURRENT_VIDICON, CURRENT_VIDICON_TIMEOUTS },
	{ TRANSMIT_TYPE_UDP_CLT, ENDSTATION_ADDRESS_UNDETERMINED_ALLOCATION, ENDSTATION_ADDRESS_UNDETERMINED_ALLOCATION_TIMEOUTS },
	{ TRANSMIT_TYPE_UDP_CLT, VIDICON_CONTROL, VIDICON_CONTROL_TIMEOUTS },
	{ TRANSMIT_TYPE_UDP_CLT, VIDICON_PRERATION_SET, VIDICON_PRERATION_SET_TIMEOUTS },
	{ TRANSMIT_TYPE_UDP_CLT, VIDICON_LOCK, VIDICON_LOCK_TIMEOUTS },
	{ TRANSMIT_TYPE_UDP_CLT, VIDICON_OUTPUT, VIDICON_OUTPUT_TIMEOUTS },
	{ TRANSMIT_TYPE_UDP_CLT, BEGIN_VOTE, BEGIN_VOTE_TIMEOUTS },
	{ TRANSMIT_TYPE_UDP_CLT, PAUSE_VOTE, PAUSE_VOTE_TIMEOUTS },
	{ TRANSMIT_TYPE_UDP_CLT, REGAIN_VOTE, REGAIN_VOTE_TIMEOUTS },
	{ TRANSMIT_TYPE_UDP_CLT, END_VOTE, END_VOTE_TIMEOUTS },
	{ TRANSMIT_TYPE_UDP_CLT, RESULT_VOTE, RESULT_VOTE_TIMEOUTS },
	{ TRANSMIT_TYPE_UDP_CLT, TRANSMIT_TO_ENDSTATION, TRANSMIT_TO_ENDSTATION_TIMEOUTS },
	{ TRANSMIT_TYPE_UDP_CLT, REPORT_ENDSTATION_MESSAGE, REPORT_ENDSTATION_MESSAGE_TIMEOUTS },
	{ TRANSMIT_TYPE_UDP_CLT, HIGH_DEFINITION_SWITCH_SET, HIGH_DEFINITION_SWITCH_SET_TIMEOUTS },
	
	{ TRANSMIT_TYPE_CAMERA_UART_CTRL, CAMERA_CTRL_STOP, CAMERA_CTRL_STOP_INTERVAL_TIME },
	{ TRANSMIT_TYPE_CAMERA_UART_CTRL, CAMERA_CTRL_AUTO_SCAN, CAMERA_CTRL_AUTO_SCAN_INTERVAL_TIME  },
	{ TRANSMIT_TYPE_CAMERA_UART_CTRL, CAMERA_CTRL_IRIS_CLOSE, CAMERA_CTRL_IRIS_CLOSE_INTERVAL_TIME },
	{ TRANSMIT_TYPE_CAMERA_UART_CTRL, CAMERA_CTRL_IRIS_OPEN, CAMERA_CTRL_IRIS_OPEN_INTERVAL_TIME },
	{ TRANSMIT_TYPE_CAMERA_UART_CTRL, CAMERA_CTRL_FOCUCS_NEAR, CAMERA_CTRL_FOCUCS_NEAR_INTERVAL_TIME },
	{ TRANSMIT_TYPE_CAMERA_UART_CTRL, CAMERA_CTRL_FOCUCS_FAR, CAMERA_CTRL_FOCUCS_FAR_INTERVAL_TIME },
	{ TRANSMIT_TYPE_CAMERA_UART_CTRL, CAMERA_CTRL_ZOOM_WIDE, CAMERA_CTRL_ZOOM_WIDE_INTERVAL_TIME },
	{ TRANSMIT_TYPE_CAMERA_UART_CTRL, CAMERA_CTRL_ZOOM_TELE, CAMERA_CTRL_ZOOM_TELE_INTERVAL_TIME },
	{ TRANSMIT_TYPE_CAMERA_UART_CTRL, CAMERA_CTRL_DOWN, CAMERA_CTRL_DOWN_INTERVAL_TIME },
	{ TRANSMIT_TYPE_CAMERA_UART_CTRL, CAMERA_CTRL_UP, CAMERA_CTRL_UP_INTERVAL_TIME },
	{ TRANSMIT_TYPE_CAMERA_UART_CTRL, CAMERA_CTRL_LEFT, CAMERA_CTRL_LEFT_INTERVAL_TIME },
	{ TRANSMIT_TYPE_CAMERA_UART_CTRL, CAMERA_CTRL_RIGHT, CAMERA_CTRL_RIGHT_INTERVAL_TIME },
	{ TRANSMIT_TYPE_CAMERA_UART_CTRL, CAMERA_CTRL_PRESET_SET, CAMERA_CTRL_PRESET_SET_INTERVAL_TIME },
	{ TRANSMIT_TYPE_CAMERA_UART_CTRL, CAMERA_CTRL_PRESET_CALL, CAMERA_CTRL_PRESET_CALL_INTERVAL_TIME },
	
	{ TRANSMIT_TYPE_MATRIX_UART_CTRL, 0xffff, 100}
};

uint32_t get_send_respons_interval_timeout( uint8_t data_type, uint32_t command )
{
	uint32_t index = 0, i = 0;
	uint16_t interval_table_num = 0;
	interval_table_num	= GET_ARRAY_ELEM_NUM( response_data_interval_util_tables, struct response_data_interval_util );
	struct response_data_interval_util *p = &response_data_interval_util_tables[0];
	
	for( i = 0; i < interval_table_num; i++ )
	{
		if( p[i].type == data_type && p[i].command == command )
		{
			index = i;
			break;
		}
	}

	return (index < interval_table_num)? p[index].timeout:0;
}

// 交换两个的数值
bool swap_valtile_uint16( volatile uint16_t *p_sw1, volatile uint16_t *p_sw2 )
{
	assert( p_sw1 && p_sw2 );
	if( p_sw1 == NULL || p_sw2 == NULL )
		return false;

	if( *p_sw1 == *p_sw2 || (p_sw1 == p_sw2))
	{// 相等不用交换
		return true;
	}
	
	*p_sw1 ^= *p_sw2;
	*p_sw2 ^= *p_sw1;
	*p_sw1 ^= *p_sw2;

	return true;
}

const char* logging_level_string_get( int32_t level_value )
{
	if( level_value < TOTAL_NUM_OF_LOGGING_LEVELS )
	{
		return logging_level_names[level_value];
	}

	return "UNKNOWN";
}

// 转换流id到tarker id
uint64_t convert_stream_id_to_tarker_id( const uint64_t listern_stream_id )
{
	struct jdksavdecc_eui64 array;
	uint64_t id = 0;

	array.value[0] = (uint8_t)(listern_stream_id>>(7*8));
	array.value[1] = (uint8_t)(listern_stream_id>>(6*8));
	array.value[2] = (uint8_t)(listern_stream_id>>(5*8));
	array.value[3] = 0xff;
	array.value[4] = 0xfe;
	array.value[5] = (uint8_t)(listern_stream_id>>(4*8));
	array.value[6] = (uint8_t)(listern_stream_id>>(3*8));
	array.value[7] = (uint8_t)(listern_stream_id>>(2*8));

	convert_eui64_to_uint64( array.value, &id );

	return id;
}

struct conference_command_resend_count
{
	uint8_t cmd;
	uint8_t resend_cnt;
};

struct conference_command_resend_count conference_command_resend_count_tables[] = 
{
	{ QUERY_END, 2 },
	{ ALLOCATION, 2 },
	{ SET_END_STATUS, 2 },
	{ REALLOCATION, 2 },
	{ KEYPAD_ACTION, 2 },
	{ SET_ENDLIGHT, 2 },
	{ NEW_ALLOCATION, 2 },
	{ END_ASSIGN, 2 },
	{ SET_END_LCD, 2 },
	{ COMMON_SEND_END_LCD, 2 },
	{ SET_END_LED, 2 },
	{ COMMON_SEND_END_LED, 2 },
	{ PRESIDENT_CONTROL, 2 },
	{ SEND_VOTE_RESULT, 2 },
	{ TALKTIME_LEN, 2 },
	{ HOST_SEND_STATUS, 2 },
	{ SEND_END_LCD_DISPLAY, 2 },
	{ OPTITION_END, 2 },
	{ SET_MIS_STATUS, 2 },
	{ END_SPETHING, 2 },
	{ CHECK_END_RESULT, 2 },
	{ TRANSIT_HOST_MSG, 2 },
	{ TRANSIT_END_MSG, 2 },
	{ 0xff, 2 }
};

uint8_t get_conference_command_resend_count(const uint8_t cmd)
{
	struct conference_command_resend_count * p = &conference_command_resend_count_tables[0];

	while ( p != NULL && p->cmd != 0xff)
	{
		if (p->cmd == cmd)
			break;

		p++;
	}

	return p->resend_cnt;
}

