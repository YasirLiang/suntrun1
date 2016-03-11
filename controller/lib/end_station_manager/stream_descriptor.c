#include "stream_descriptor.h"
#include "jdksavdecc_frame.h"
#include "jdksavdecc_acmp.h"
#include "avdecc_net.h"
#include "acmp_controller_machine.h"
#include "host_controller_debug.h"

static bool rx_state_resp = false;	// 接收到rx_state 命令标志
static bool tx_state_resp = false;	// 接收到tx_state 命令标志

bool get_stream_output_tx_state_resp( void )
{
	return tx_state_resp;
}

void set_stream_output_tx_state_resp( bool status )
{
	tx_state_resp = status;
}

bool get_stream_input_rx_state_resp( void )
{
	return rx_state_resp;
}

void set_stream_input_rx_state_resp( bool status )
{
	rx_state_resp = status;
}

int proc_get_rx_state_resp( const uint8_t *frame, size_t frame_len, uint16_t desc_index, int *status)
{
	struct jdksavdecc_frame cmd_frame;
	ssize_t acmp_cmd_get_rx_state_resp_returned;
	struct jdksavdecc_acmpdu acmp_cmd_get_rx_state_resp; // Store the response received after sending a GET_TX_STATE command.

	memset(&acmp_cmd_get_rx_state_resp, 0, sizeof(struct jdksavdecc_acmpdu));
	memcpy(cmd_frame.payload, frame, frame_len);
	acmp_cmd_get_rx_state_resp_returned = jdksavdecc_acmpdu_read(&acmp_cmd_get_rx_state_resp, frame, ZERO_OFFSET_IN_PAYLOAD, frame_len);
	if( acmp_cmd_get_rx_state_resp_returned < 0 )
	{
		DEBUG_INFO( "LOGGING_LEVEL_ERROR:acmp_cmd_get_rx_state_read error");
		assert(acmp_cmd_get_rx_state_resp_returned >= 0);
		return -1;
	}

	*status = acmp_cmd_get_rx_state_resp.header.status;
	acmp_update_input_stream_descriptor( desc_index, &acmp_cmd_get_rx_state_resp);
	set_stream_input_rx_state_resp( true );
        acmp_proc_state_resp( &cmd_frame );

	return 0;
}


int proc_get_tx_state_resp( const uint8_t *frame, size_t frame_len,uint16_t desc_index, int *status)
{
        struct jdksavdecc_frame cmd_frame;
        ssize_t acmp_cmd_get_tx_state_resp_returned;
	struct jdksavdecc_acmpdu acmp_cmd_get_tx_state_resp;

	memset(&acmp_cmd_get_tx_state_resp, 0, sizeof(struct jdksavdecc_acmpdu));
        memcpy(cmd_frame.payload, frame, frame_len);
        acmp_cmd_get_tx_state_resp_returned = jdksavdecc_acmpdu_read(&acmp_cmd_get_tx_state_resp, frame, ZERO_OFFSET_IN_PAYLOAD, frame_len );
        if(acmp_cmd_get_tx_state_resp_returned < 0)
        {
        	DEBUG_INFO( "LOGGING_LEVEL_ERROR:acmp_cmd_get_tx_state_read error" );
            	assert(acmp_cmd_get_tx_state_resp_returned >= 0);
            	return -1;
        }

        *status = acmp_cmd_get_tx_state_resp.header.status;
	acmp_update_output_stream_descriptor( desc_index, &acmp_cmd_get_tx_state_resp);
	set_stream_output_tx_state_resp( true );
        acmp_proc_state_resp( &cmd_frame );

        return 0;
}

int proc_connect_rx_resp( const uint8_t *frame, size_t frame_len, int *status)
{
	struct jdksavdecc_frame cmd_frame;
	ssize_t acmp_cmd_connect_rx_resp_returned;
	struct jdksavdecc_acmpdu acmp_cmd_connect_rx_resp; // Store the response received after sending a CONNECT_RX command.

	memset(&acmp_cmd_connect_rx_resp, 0, sizeof(struct jdksavdecc_acmpdu));
	memcpy(cmd_frame.payload, frame, frame_len);
	acmp_cmd_connect_rx_resp_returned = jdksavdecc_acmpdu_read(&acmp_cmd_connect_rx_resp, frame, ZERO_OFFSET_IN_PAYLOAD, frame_len);
	if(acmp_cmd_connect_rx_resp_returned < 0)
	{
		DEBUG_INFO( "LOGGING_LEVEL_ERROR:acmp_cmd_connect_rx_read error" );
		assert(acmp_cmd_connect_rx_resp_returned >= 0);
		return -1;
	}

	*status = acmp_cmd_connect_rx_resp.header.status;
        acmp_proc_state_resp( &cmd_frame );

	return 0;
 }

int proc_disconnect_rx_resp( const uint8_t *frame, size_t frame_len, int *status)
{
        struct jdksavdecc_frame cmd_frame;
        ssize_t acmp_cmd_disconnect_rx_resp_returned;
	struct jdksavdecc_acmpdu acmp_cmd_disconnect_rx_resp; // Store the response received after sending a DISCONNECT_RX command.

	memset(&acmp_cmd_disconnect_rx_resp, 0, sizeof(struct jdksavdecc_acmpdu));
        memcpy(cmd_frame.payload, frame, frame_len);
        acmp_cmd_disconnect_rx_resp_returned = jdksavdecc_acmpdu_read(&acmp_cmd_disconnect_rx_resp, frame, ZERO_OFFSET_IN_PAYLOAD, frame_len);
        if(acmp_cmd_disconnect_rx_resp_returned < 0)
        {
            DEBUG_INFO( "LOGGING_LEVEL_ERROR:acmp_cmd_disconnect_rx_read error");
            assert(acmp_cmd_disconnect_rx_resp_returned >= 0);
            return -1;
        }

        *status = acmp_cmd_disconnect_rx_resp.header.status;

        acmp_proc_state_resp( &cmd_frame );

        return 0;
}

int proc_get_tx_connection_resp( const uint8_t *frame, size_t frame_len, int *status)
    {
        struct jdksavdecc_frame cmd_frame;
        ssize_t acmp_cmd_get_tx_connection_resp_returned;
	struct jdksavdecc_acmpdu acmp_cmd_get_tx_connection_resp; // Store the response received after sending a GET_TX_CONNECTION command.

	memset(&acmp_cmd_get_tx_connection_resp, 0, sizeof(struct jdksavdecc_acmpdu));
        memcpy(cmd_frame.payload, frame, frame_len);
        acmp_cmd_get_tx_connection_resp_returned = jdksavdecc_acmpdu_read(&acmp_cmd_get_tx_connection_resp, frame, ZERO_OFFSET_IN_PAYLOAD, frame_len);
        if(acmp_cmd_get_tx_connection_resp_returned < 0)
        {		
		DEBUG_INFO( "LOGGING_LEVEL_ERROR:acmp_cmd_get_tx_connection_read error");
           	assert(acmp_cmd_get_tx_connection_resp_returned >= 0);
            	return -1;
        }
		
        *status = acmp_cmd_get_tx_connection_resp.header.status;
        acmp_proc_state_resp( &cmd_frame );

        return 0;
}

// store input stream to system desc list
void store_stream_input_desc( const uint8_t *frame, int pos, size_t frame_len, desc_pdblist  desc_info_node )
{
	struct jdksavdecc_descriptor_stream stream_input_desc; // Structure containing the stream_output_desc fields
	ssize_t ret = jdksavdecc_descriptor_stream_read(&stream_input_desc, frame, pos, frame_len);
        if (ret < 0)
        {
        	DEBUG_INFO( "avdecc_read_descriptor_error: stream_input_desc_read error" );
		return ;
        }

	uint8_t num = ++desc_info_node->endpoint_desc.input_stream.num;
	assert( desc_info_node && num <=  MAX_STREAM_NUM );
	desc_info_node->endpoint_desc.input_stream.desc[num-1].descriptor_type = stream_input_desc.descriptor_type;
	desc_info_node->endpoint_desc.input_stream.desc[num-1].descriptor_index = stream_input_desc.descriptor_index;
	desc_info_node->endpoint_desc.input_stream.desc[num-1].current_format = stream_input_desc.current_format;
	desc_info_node->endpoint_desc.is_input_stream_desc_exist = true;
}

// store output stream to system desc list
void store_stream_output_desc( const uint8_t *frame, int pos, size_t frame_len, desc_pdblist  desc_info_node )
{
	struct jdksavdecc_descriptor_stream stream_output_desc; // Structure containing the stream_output_desc fields
	ssize_t ret = jdksavdecc_descriptor_stream_read( &stream_output_desc, frame, pos, frame_len );
        if (ret < 0)
        {
        	DEBUG_INFO( "avdecc_read_descriptor_error:stream_output_desc_read error" );
		return;
        }

	uint8_t num = ++desc_info_node->endpoint_desc.output_stream.num;
	assert( desc_info_node && num <=  MAX_STREAM_NUM );
	desc_info_node->endpoint_desc.output_stream.desc[num-1].descriptor_type = stream_output_desc.descriptor_type;
	desc_info_node->endpoint_desc.output_stream.desc[num-1].descriptor_index = stream_output_desc.descriptor_index;
	desc_info_node->endpoint_desc.output_stream.desc[num-1].current_format = stream_output_desc.current_format;
	desc_info_node->endpoint_desc.is_output_stream_desc_exist = true;
}

int background_read_descriptor_input_output_stream( void )
{
	return acmp_update_endstation_connections_networks();
}


