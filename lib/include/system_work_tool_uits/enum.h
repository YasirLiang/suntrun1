#ifndef __ENUM_H__
#define __ENUM_H__

enum host_and_endstation_commands // commands between host controller and endstations
{
	QUERY_END = 0x1,
	ALLOCATION = 0x02,
	SET_END_STATUS = 0x03,
	REALLOCATION = 0x04,
	KEYPAD_ACTION = 0x05,
	SET_ENDLIGHT = 0x06,
	NEW_ALLOCATION = 0x07,
	END_ASSIGN = 0x08,
	SET_END_LCD  = 0x09,
	COMMON_SEND_END_LCD = 0x0a,
	SET_END_LED = 0x0b,
	COMMON_SEND_END_LED = 0x0c,
	PRESIDENT_CONTROL =  0x0d,
	SEND_VOTE_RESULT =0x0e,
	TALKTIME_LEN = 0x0f,	
	HOST_SEND_STATUS = 0x10,	
	SEND_END_LCD_DISPLAY = 0x11,
	OPTITION_END = 0x12,
	SET_MIS_STATUS = 0x13,
	END_SPETHING = 0x14,
	CHECK_END_RESULT = 0x16,
	TRANSIT_HOST_MSG = 0x1e,
	TRANSIT_END_MSG = 0x1f,
        CONFERENCE_HOST_AND_END_CMD_ERROR = 0xffff
};

enum host_and_upper_computer_commands // commands between host controller and upper computer
{
	DISCUSSION_PARAMETER = 0x1,
	MISCROPHONE_SWITCH = 0x02,
	MISCROPHONE_STATUS = 0x03,
	SELECT_PROPOSER = 0x05,
	EXAMINE_APPLICATION = 0x06,
	CONFERENCE_PERMISSION = 0x07,
	SENDDOWN_MESSAGE = 0x08,
	TABLE_TABLET_STANDS_MANAGER = 0x09,
	BEGIN_SIGN  = 0x0a,
	SIGN_SITUATION = 0x0b,
	END_OF_SIGN = 0x0c,
	ENDSTATION_ALLOCATION_APPLICATION_ADDRESS = 0x0e,
	ENDSTATION_REGISTER_STATUS =  0x0f,
	CURRENT_VIDICON =0x10,
	ENDSTATION_ADDRESS_UNDETERMINED_ALLOCATION = 0x11,	
	VIDICON_CONTROL = 0x12,	
	VIDICON_PRERATION_SET = 0x13,
	VIDICON_LOCK = 0x14,
	VIDICON_OUTPUT = 0x15,
	BEGIN_VOTE = 0x17,
	PAUSE_VOTE = 0x18,
	REGAIN_VOTE = 0x19,
	END_VOTE = 0x20,
	RESULT_VOTE = 0x21,
	TRANSMIT_TO_ENDSTATION = 0x22,
	REPORT_ENDSTATION_MESSAGE = 0x23,
	HIGH_DEFINITION_SWITCH_SET = 0x24,
        HOST_AND_UPPER_CMPT_CMD_ERROR = 0xffff
};

enum host_and_endstation_command_timeouts // commands TIMEOUTS between host controller and endstations
{
	QUERY_END_TIMEOUTS = 200,
	ALLOCATION_TIMEOUTS = 200,
	SET_END_STATUS_TIMEOUTS = 200,
	REALLOCATION_TIMEOUTS = 200,
	KEYPAD_ACTION_TIMEOUTS = 200,
	SET_ENDLIGHT_TIMEOUTS = 200,
	NEW_ALLOCATION_TIMEOUTS = 200,
	END_ASSIGN_TIMEOUTS = 200,
	SET_END_LCD_TIMEOUTS  = 200,
	COMMON_SEND_END_LCD_TIMEOUTS = 200,
	SET_END_LED_TIMEOUTS = 200,
	COMMON_SEND_END_LED_TIMEOUTS = 200,
	PRESIDENT_CONTROL_TIMEOUTS =  200,
	SEND_VOTE_RESULT_TIMEOUTS = 200,
	TALKTIME_LEN_TIMEOUTS = 200,	
	HOST_SEND_STATUS_TIMEOUTS = 200,	
	SEND_END_LCD_DISPLAY_TIMEOUTS = 200,
	OPTITION_END_TIMEOUTS = 200,
	SET_MIS_STATUS_TIMEOUTS = 200,
	END_SPETHING_TIMEOUTS = 200,
	CHECK_END_RESULT_TIMEOUTS = 200,
	TRANSIT_HOST_MSG_TIMEOUTS = 200,
	TRANSIT_END_MSG_TIMEOUTS = 200,
};

enum udp_upper_cmd_timeout //  upper and host deal timouts
{
	CONFERENCE_DISCUSSION_PARAMETER_TIMEOUTS = 1000,
	MISCROPHONE_SWITCH_TIMEOUTS = 200,
	MISCROPHONE_STATUS_TIMEOUTS = 200,
	SELECT_PROPOSER_TIMEOUTS = 200,
	EXAMINE_APPLICATION_TIMEOUTS = 200, 
	CONFERENCE_PERMISSION_TIMEOUTS = 200,
	SENDDOWN_MESSAGE_TIMEOUTS = 200,
	BEGIN_SIGN_TIMEOUTS = 200, 
	SIGN_SITUATION_TIMEOUTS = 200,
	END_OF_SIGN_TIMEOUTS = 200,
	ENDSTATION_ALLOCATION_APPLICATION_ADDRESS_TIMEOUTS = 200,
	ENDSTATION_REGISTER_STATUS_TIMEOUTS = 200, 
	CURRENT_VIDICON_TIMEOUTS = 200,
	ENDSTATION_ADDRESS_UNDETERMINED_ALLOCATION_TIMEOUTS = 200,
	VIDICON_CONTROL_TIMEOUTS = 200,
	VIDICON_PRERATION_SET_TIMEOUTS = 200,
	VIDICON_LOCK_TIMEOUTS = 200,
	VIDICON_OUTPUT_TIMEOUTS = 200,
	BEGIN_VOTE_TIMEOUTS = 200,
	PAUSE_VOTE_TIMEOUTS = 200,
	REGAIN_VOTE_TIMEOUTS = 200, 
	END_VOTE_TIMEOUTS = 200,
	RESULT_VOTE_TIMEOUTS = 200,
	TRANSMIT_TO_ENDSTATION_TIMEOUTS = 200,
	REPORT_ENDSTATION_MESSAGE_TIMEOUTS = 200,
	HIGH_DEFINITION_SWITCH_SET_TIMEOUTS = 200, 
};

enum adp_msg_type_values
{
	ADP_MESSAGE_TYPE_ENTITY_AVAILABLE,
	ADP_MESSAGE_TYPE_ENTITY_DEPARTING,
	ADP_MESSAGE_TYPE_ENTITY_DISCOVER,
	ADP_ERROR_MESSAGE_TYEP
};

enum aem_cmds_values /// The command codes values for AEM commands
{
        AEM_CMD_ACQUIRE_ENTITY,
        AEM_CMD_LOCK_ENTITY,
        AEM_CMD_ENTITY_AVAILABLE,
        AEM_CMD_CONTROLLER_AVAILABLE,
        AEM_CMD_READ_DESCRIPTOR,
        AEM_CMD_WRITE_DESCRIPTOR,
        AEM_CMD_SET_CONFIGURATION,
        AEM_CMD_GET_CONFIGURATION,
        AEM_CMD_SET_STREAM_FORMAT,
        AEM_CMD_GET_STREAM_FORMAT,
        AEM_CMD_SET_VIDEO_FORMAT,
        AEM_CMD_GET_VIDEO_FORMAT,
        AEM_CMD_SET_SENSOR_FORMAT,
        AEM_CMD_GET_SENSOR_FORMAT,
        AEM_CMD_SET_STREAM_INFO,
        AEM_CMD_GET_STREAM_INFO,
        AEM_CMD_SET_NAME,
        AEM_CMD_GET_NAME,
        AEM_CMD_SET_ASSOCIATION_ID,
        AEM_CMD_GET_ASSOCIATION_ID,
        AEM_CMD_SET_SAMPLING_RATE,
        AEM_CMD_GET_SAMPLING_RATE,
        AEM_CMD_SET_CLOCK_SOURCE,
        AEM_CMD_GET_CLOCK_SOURCE,
        AEM_CMD_SET_CONTROL,
        AEM_CMD_GET_CONTROL,
        AEM_CMD_INCREMENT_CONTROL,
        AEM_CMD_DECREMENT_CONTROL,
        AEM_CMD_SET_SIGNAL_SELECTOR,
        AEM_CMD_GET_SIGNAL_SELECTOR,
        AEM_CMD_SET_MIXER,
        AEM_CMD_GET_MIXER,
        AEM_CMD_SET_MATRIX,
        AEM_CMD_GET_MATRIX,
        AEM_CMD_START_STREAMING,
        AEM_CMD_STOP_STREAMING,
        AEM_CMD_REGISTER_UNSOLICITED_NOTIFICATION,
        AEM_CMD_DEREGISTER_UNSOLICITED_NOTIFICATION,
        AEM_CMD_IDENTIFY_NOTIFICATION,
        AEM_CMD_GET_AVB_INFO,
        AEM_CMD_GET_AS_PATH,
        AEM_CMD_GET_COUNTERS,
        AEM_CMD_REBOOT,
        AEM_CMD_GET_AUDIO_MAP,
        AEM_CMD_ADD_AUDIO_MAPPINGS,
        AEM_CMD_REMOVE_AUDIO_MAPPINGS,
        AEM_CMD_GET_VIDEO_MAP,
        AEM_CMD_ADD_VIDEO_MAPPINGS,
        AEM_CMD_REMOVE_VIDEO_MAPPINGS,
        AEM_CMD_GET_SENSOR_MAP,
        AEM_CMD_ADD_SENSOR_MAPPINGS,
        AEM_CMD_REMOVE_SENSOR_MAPPINGS,
        AEM_CMD_START_OPERATION,
        AEM_CMD_ABORT_OPERATION,
        AEM_CMD_OPERATION_STATUS,
        AEM_CMD_AUTH_ADD_KEY,
        AEM_CMD_AUTH_DELETE_KEY,
        AEM_CMD_AUTH_GET_KEY_LIST,
        AEM_CMD_AUTH_GET_KEY,
        AEM_CMD_AUTH_ADD_KEY_TO_CHAIN,
        AEM_CMD_AUTH_DELETE_KEY_FROM_CHAIN,
        AEM_CMD_AUTH_GET_KEYCHAIN_LIST,
        AEM_CMD_AUTH_GET_IDENTITY,
        AEM_CMD_AUTH_ADD_TOKEN,
        AEM_CMD_AUTH_DELETE_TOKEN,
        AEM_CMD_AUTHENTICATE,
        AEM_CMD_DEAUTHENTICATE,
        AEM_CMD_ENABLE_TRANSPORT_SECURITY,
        AEM_CMD_DISABLE_TRANSPORT_SECURITY,
        AEM_CMD_ENABLE_STREAM_ENCRYPTION,
        AEM_CMD_DISABLE_STREAM_ENCRYPTION,
        AEM_CMD_SET_MEMORY_OBJECT_LENGTH,
        AEM_CMD_GET_MEMORY_OBJECT_LENGTH,
        AEM_CMD_SET_STREAM_BACKUP,
        AEM_CMD_GET_STREAM_BACKUP,
        TOTAL_NUM_OF_AEM_CMDS = 75, ///< The total number of AEM commands currently supported in the 1722.1 specification
        AEM_CMD_ERROR = 0xffff
};

enum aem_descs_values /// The descriptor types values for AEM descriptors
{
        AEM_DESC_ENTITY,
        AEM_DESC_CONFIGURATION,
        AEM_DESC_AUDIO_UNIT,
        AEM_DESC_VIDEO_UNIT,
        AEM_DESC_SENSOR_UNIT,
        AEM_DESC_STREAM_INPUT,
        AEM_DESC_STREAM_OUTPUT,
        AEM_DESC_JACK_INPUT,
        AEM_DESC_JACK_OUTPUT,
        AEM_DESC_AVB_INTERFACE,
        AEM_DESC_CLOCK_SOURCE,
        AEM_DESC_MEMORY_OBJECT,
        AEM_DESC_LOCALE,
        AEM_DESC_STRINGS,
        AEM_DESC_STREAM_PORT_INPUT,
        AEM_DESC_STREAM_PORT_OUTPUT,
        AEM_DESC_EXTERNAL_PORT_INPUT,
        AEM_DESC_EXTERNAL_PORT_OUTPUT,
        AEM_DESC_INTERNAL_PORT_INPUT,
        AEM_DESC_INTERNAL_PORT_OUTPUT,
        AEM_DESC_AUDIO_CLUSTER,
        AEM_DESC_VIDEO_CLUSTER,
        AEM_DESC_SENSOR_CLUSTER,
        AEM_DESC_AUDIO_MAP,
        AEM_DESC_VIDEO_MAP,
        AEM_DESC_SENSOR_MAP,
        AEM_DESC_CONTROL,
        AEM_DESC_SIGNAL_SELECTOR,
        AEM_DESC_MIXER,
        AEM_DESC_MATRIX,
        AEM_DESC_MATRIX_SIGNAL,
        AEM_DESC_SIGNAL_SPLITTER,
        AEM_DESC_SIGNAL_COMBINER,
        AEM_DESC_SIGNAL_DEMULTIPLEXER,
        AEM_DESC_SIGNAL_MULTIPLEXER,
        AEM_DESC_SIGNAL_TRANSCODER,
        AEM_DESC_CLOCK_DOMAIN,
        AEM_DESC_CONTROL_BLOCK,
        TOTAL_NUM_OF_AEM_DESCS = 38, ///< The total number of AEM descriptors currently supported in the 1722.1 specification
        AEM_DESC_ERROR = 0xffff
};

enum aem_cmds_status
{
        AEM_STATUS_SUCCESS = 0, ///< The AVDECC Entity successfully performed the command and has valid results
        AEM_STATUS_NOT_IMPLEMENTED = 1, ///< The AVDECC Entity does not support the command type
        AEM_STATUS_NO_SUCH_DESCRIPTOR = 2, ///< A descriptor with the descriptor type and index does not exist
        AEM_STATUS_ENTITY_LOCKED = 3, ///< The AVDECC Entity has been locked by another AVDECC Controller
        AEM_STATUS_ENTITY_ACQUIRED = 4, ///< The AVDECC Entity has been acquired by another AVDECC Controller
        AEM_STATUS_NOT_AUTHENTICATED = 5, ///< The AVDECC Controller is not authenticated with the AVDECC Entity
        AEM_STATUS_AUTHENTICATION_DISABLED = 6, ///< The AVDECC Controller is trying to use an authentication command when authentication is not enabled on the AVDECC Entity
        AEM_STATUS_BAD_ARGUMENTS = 7, ///< One or more of the values in the fields of the frame were deemed to be bad by the AVDECC Entity
        AEM_STATUS_NO_RESOURCES = 8, ///< The AVDECC Entity cannot complete the command because it does not have the resources to support it
        AEM_STATUS_IN_PROGRESS = 9, ///< The AVDECC Entity is processing the command and will send a second response at a later time with the result of the command
        AEM_STATUS_ENTITY_MISBEHAVING = 10, ///< The AVDECC Entity is generating an internal error while trying to process the command
        AEM_STATUS_NOT_SUPPORTED = 11, ///< The command is implemented, but the target of the command is not supported
        AEM_STATUS_STREAM_IS_RUNNING = 12, ///< The stream is currently streaming and the command is one which cannot be executed on a streaming stream
        TOTAL_NUM_OF_AEM_CMDS_STATUS = 13,  ///< The total number of AEM commands status currently supported in the 1722.1 specification
        AVDECC_LIB_STATUS_INVALID = 1023, ///< AVDECC library specific status, not part of the 1722.1 specification
                                          ///< The response received has a subtype different from the subtype of the command sent
        AVDECC_LIB_STATUS_TICK_TIMEOUT = 1024 ///< AVDECC library specific status, not part of the 1722.1 specification
                                              ///< The response is not received within the timeout period after re-sending a command
};

enum acmp_cmds_values /// The command codes values for ACMP commands
{
        CONNECT_TX_COMMAND,
        CONNECT_TX_RESPONSE,
        DISCONNECT_TX_COMMAND,
        DISCONNECT_TX_RESPONSE,
        GET_TX_STATE_COMMAND,
        GET_TX_STATE_RESPONSE,
        CONNECT_RX_COMMAND,
        CONNECT_RX_RESPONSE,
        DISCONNECT_RX_COMMAND,
        DISCONNECT_RX_RESPONSE,
        GET_RX_STATE_COMMAND,
        GET_RX_STATE_RESPONSE,
        GET_TX_CONNECTION_COMMAND,
        GET_TX_CONNECTION_RESPONSE,
        TOTAL_NUM_OF_ACMP_CMDS = 14, ///< The total number of ACMP commands currently supported in the 1722.1 specification
        AEM_ACMP_ERROR = 0xffff
};

enum acmp_cmds_status
{
	ACMP_STATUS_SUCCESS = 0, ///< Command executed successfully
	ACMP_STATUS_LISTENER_UNKNOWN_ID = 1, ///< Listener does not have the specified unique identifier
	ACMP_STATUS_TALKER_UNKNOWN_ID = 2, ///< Talker does not have the specified unique identifier
	ACMP_STATUS_TALKER_DEST_MAC_FAIL = 3, ///< Talker could not allocate a destination MAC for the steam
	ACMP_STATUS_TALKER_NO_STREAM_INDEX = 4, ///< Talker does not have an available stream index for the stream
	ACMP_STATUS_TALKER_NO_BANDWIDTH = 5, ///< Talker could not allocate bandwidth for the stream
	ACMP_STATUS_TALKER_EXCLUSIVE = 6, ///< Talker already has an established stream and only supports one listener 
	ACMP_STATUS_LISTENER_TALKER_TIMEOUT = 7, ///< Listener had timeout for all retries when trying to send command to Talker
	ACMP_STATUS_LISTENER_EXCLUSIVE = 8, ///< Listener already has an established connection to a stream
	ACMP_STATUS_STATE_UNAVAILABLE = 9, ///< Could not get the state from the AVDECC Entity
	ACMP_STATUS_NOT_CONNECTED  = 10, ///< Trying to disconnect when not connected or not connected to the Talker specified
	ACMP_STATUS_NO_SUCH_CONNECTION = 11, ///< Trying to obtain connection info for an Talker connection which does not exist
	ACMP_STATUS_COULD_NOT_SEND_MESSAGE = 12, ///< Listener failed to send the message to the Talker
	ACMP_STATUS_TALKER_MISBEHAVING = 13, ///< Talker was unable to complete the command because an internal error occurred
	ACMP_STATUS_LISTENER_MISBEHAVING = 14, ///< Listener was unable to complete the command because an internal error occurred
	ACMP_STATUS_RESERVED = 15, ///< Reserved field for new status
	ACMP_STATUS_CONTROLLER_NOT_AUTHORIZED = 16, ///< The AVDECC Controller with the specified Entity ID is not authorized to change stream connections
	ACMP_STATUS_INCOMPATIBLE_REQUEST = 17, ///< Listener is trying to connect a Talker that is already streaming with a different traffic class, etc, or
	                                       ///< does not support the request traffic class
	ACMP_STATUS_LISTENER_INVALID_CONNECTION = 18,
	TOTAL_NUM_OF_ACMP_CMDS_STATUS = 19,  ///< The total number of AEM commands status currently supported in the 1722.1 specification
};

enum aem_cmd_waiting
{
	CMD_WITHOUT_NOTIFICATION = 0, ///< All internal commands are sent without notification ids
	CMD_WITH_NOTIFICATION = 1, ///< All user commands are sent with unique notification ids
};

enum ether_hdr_info
{
	SRC_MAC_SIZE = 6,
	DEST_MAC_SIZE = 6,
	ETHER_PROTOCOL_SIZE = 2,
	ETHER_HDR_SIZE = SRC_MAC_SIZE + DEST_MAC_SIZE + ETHER_PROTOCOL_SIZE, ///<  The header size of the Ethernet frame = dest mac + src mac + ether protocol sizes
};

enum protocol_hdr_info
{
    	PROTOCOL_HDR_SIZE = 4 ///<  The Protocol Header size
};

enum frame_lengths
{
	ADP_FRAME_LEN = 82, ///< Length of ADP packet is 82 bytes
	ACMP_FRAME_LEN = 70, ///< Length of ACMP packet is 70 bytes
	AECP_FRAME_LEN = 64 ///< Length of AECP packet is 64 bytes
};

enum timeouts
{
	NETIF_READ_TIMEOUT_MS = 100, ///< The network interface has a 100 milliseconds timeout in capturing ADP packets
	AVDECC_MSG_TIMEOUT_MS = 250,  ///< AVDECC messages have a 250 milliseconds timeout
	ACMP_CONNECT_TX_COMMAND_TIMEOUT_MS = 2000,
	ACMP_DISCONNECT_TX_COMMAND_TIMEOUT_MS = 200,
	ACMP_GET_TX_STATE_COMMAND_TIMEOUT_MS = 200,
	ACMP_CONNECT_RX_COMMAND_TIMEOUT_MS = 4500,
	ACMP_DISCONNECT_RX_COMMAND_TIMEOUT_MS = 500,
	ACMP_GET_RX_STATE_COMMAND_TIMEOUT_MS = 200,
	ACMP_GET_TX_CONNECTION_COMMAND_TIMEOUT_MS = 200
};

enum flags
{
    	CMD_LOOKUP = 1024 ///<  Check if the command is an ACMP or AEM command
};

enum notifications /// Notifications for the AVDECC library implementation, not part of the 1722.1 specification
{
	NO_MATCH_FOUND = 0, ///< A command or response is not implemented
	END_STATION_CONNECTED = 1, ///< An AVDECC End Station is discovered and connected
	END_STATION_DISCONNECTED = 2, ///< An AVDECC End Station is disconnected
	COMMAND_TIMEOUT = 3, ///< A command is sent, but the response is not received within a timeout period
	RESPONSE_RECEIVED = 4, ///< A response is received after sending a command
	END_STATION_READ_COMPLETED = 5, ///< An AVDECC End Station has finished internal READ_DESCRIPTOR processing for all top level descriptors
	TOTAL_NUM_OF_NOTIFICATIONS = 6
};

enum logging_levels
{
	LOGGING_LEVEL_ERROR   = 0,
	LOGGING_LEVEL_WARNING = 1,
	LOGGING_LEVEL_NOTICE  = 2,
	LOGGING_LEVEL_INFO    = 3,
	LOGGING_LEVEL_DEBUG   = 4,
	LOGGING_LEVEL_VERBOSE = 5,
	TOTAL_NUM_OF_LOGGING_LEVELS = 6
};
    
enum counter_labels ///Counter Labels for GET_COUNTERS command
{
	AVB_INTERFACE_LINK_UP,
	AVB_INTERFACE_LINK_DOWN,
	AVB_INTERFACE_FRAMES_TX,
	AVB_INTERFACE_FRAMES_RX,
	AVB_INTERFACE_RX_CRC_ERROR,
	AVB_GPTP_GM_CHANGED,
	AVB_INTERFACE_ENTITY_SPECIFIC_1,
	AVB_INTERFACE_ENTITY_SPECIFIC_2,
	AVB_INTERFACE_ENTITY_SPECIFIC_3,
	AVB_INTERFACE_ENTITY_SPECIFIC_4,
	AVB_INTERFACE_ENTITY_SPECIFIC_5,
	AVB_INTERFACE_ENTITY_SPECIFIC_6,
	AVB_INTERFACE_ENTITY_SPECIFIC_7,
	AVB_INTERFACE_ENTITY_SPECIFIC_8,
	CLOCK_DOMAIN_LOCKED,
	CLOCK_DOMAIN_UNLOCKED,
	CLOCK_DOMAIN_ENTITY_SPECIFIC_1,
	CLOCK_DOMAIN_ENTITY_SPECIFIC_2,
	CLOCK_DOMAIN_ENTITY_SPECIFIC_3,
	CLOCK_DOMAIN_ENTITY_SPECIFIC_4,
	CLOCK_DOMAIN_ENTITY_SPECIFIC_5,
	CLOCK_DOMAIN_ENTITY_SPECIFIC_6,
	CLOCK_DOMAIN_ENTITY_SPECIFIC_7,
	CLOCK_DOMAIN_ENTITY_SPECIFIC_8,
	STREAM_INPUT_MEDIA_LOCKED,
	STREAM_INPUT_MEDIA_UNLOCKED,
	STREAM_INPUT_STREAM_RESET,
	STREAM_INPUT_SEQ_NUM_MISMATCH,
	STREAM_INPUT_MEDIA_RESET,
	STREAM_INPUT_TIMESTAMP_UNCERTAIN,
	STREAM_INPUT_TIMESTAMP_VALID,
	STREAM_INPUT_TIMESTAMP_NOT_VALID,
	STREAM_INPUT_UNSUPPORTED_FORMAT,
	STREAM_INPUT_LATE_TIMESTAMP,
	STREAM_INPUT_EARLY_TIMESTAMP,
	STREAM_INPUT_FRAMES_RX,
	STREAM_INPUT_FRAMES_TX,
	STREAM_INPUT_ENTITY_SPECIFIC_1,
	STREAM_INPUT_ENTITY_SPECIFIC_2,
	STREAM_INPUT_ENTITY_SPECIFIC_3,
	STREAM_INPUT_ENTITY_SPECIFIC_4,
	STREAM_INPUT_ENTITY_SPECIFIC_5,
	STREAM_INPUT_ENTITY_SPECIFIC_6,
	STREAM_INPUT_ENTITY_SPECIFIC_7,
	STREAM_INPUT_ENTITY_SPECIFIC_8
};

#endif

