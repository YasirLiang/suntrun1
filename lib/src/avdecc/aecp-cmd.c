/*
Copyright (c) 2014, J.D. Koftinoff Software, Ltd.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "avdecc-cmd.h"
#include "aecp-cmd.h"
#include "aecp.h"
#include "jdksavdecc_aecp_print.h"
#include "jdksavdecc_aem_print.h"

void aecp_aem_print( FILE *s, const struct jdksavdecc_frame *frame, const struct jdksavdecc_aecpdu_aem *aemdu )
{
    struct jdksavdecc_printer p;
    char buf[10240];
    jdksavdecc_printer_init( &p, buf, sizeof( buf ) );

    avdecc_cmd_print_frame_header( &p, frame );
    jdksavdecc_aecp_print( &p, &aemdu->aecpdu_header, frame->payload, 0, frame->length );
    fprintf( s, "%s", buf );
}

int aecp_aem_process( const void *request_, struct raw_context *net, const struct jdksavdecc_frame *frame )
{
    struct jdksavdecc_aecpdu_aem *request = (struct jdksavdecc_aecpdu_aem *)request_;
    struct jdksavdecc_aecpdu_aem response;
    bzero( &response, sizeof( response ) );
    (void)net;
    if ( aecp_aem_check( frame,
                         &response,
                         request->aecpdu_header.controller_entity_id,
                         request->aecpdu_header.header.target_entity_id,
                         request->aecpdu_header.sequence_id ) == 0 )
    {
        fprintf( stdout, "Response: \n" );
        aecp_aem_print( stdout, frame, &response );
    }

    return 0;
}
int aecp_my_process( const void *request_, struct raw_context *net, const struct jdksavdecc_frame *frame )
{
    struct jdksavdecc_printer p;
    char buf[10240];
    struct jdksavdecc_aecpdu_common common_header;
    bzero( &common_header, sizeof(common_header) );
    
    jdksavdecc_aecpdu_common_read(&common_header, frame->payload, 0, frame->length);


    struct jdksavdecc_aecpdu_common_control_header *h = &common_header.header;
    if (h->subtype != JDKSAVDECC_SUBTYPE_AECP) return 0;

    
    jdksavdecc_printer_init( &p, buf, sizeof( buf ) );
    jdksavdecc_aecp_print( &p, &common_header, frame->payload, 0, frame->length );

    fprintf( stdout, "%s", buf );
    return 0;


}

int aecp_aem( struct raw_context *net, struct jdksavdecc_frame *frame, uint16_t message_type, int argc, char **argv )
{
    int r = 1;
    struct jdksavdecc_aecpdu_aem aemdu;
    uint16_t command_code;
    uint16_t sequence_id;
    struct jdksavdecc_eui48 destination_mac;
    struct jdksavdecc_eui64 target_entity_id;
    uint8_t command_payload[640];
    size_t command_payload_len = 0;
    int arg = 3;

    bzero( &aemdu, sizeof( aemdu ) );

    if ( argc > ++arg )
    {
        arg_command = argv[arg];
    }

    if ( !jdksavdecc_get_uint16_value_for_name( jdksavdecc_aem_print_command, arg_command, &command_code ) )
    {
        errno = 0;
        char *end = (char *)arg_command;
        if ( arg_command )
        {
            command_code = (uint16_t)strtol( arg_command, &end, 0 );
        }
        if ( !arg_command || errno == ERANGE || *end )
        {
            struct jdksavdecc_uint16_name *name = jdksavdecc_aem_print_command;
            fprintf( stderr, "Invalid AECP AEM command. Options are:\n" );
            while ( name->name )
            {
                fprintf( stdout, "\t0x%04x %s\n", name->value, name->name );
                name++;
            }
            return 1;
        }
    }

    if ( argc > ++arg )
    {
        arg_destination_mac = argv[arg];
        if ( *arg_destination_mac == 0 )
        {
            arg_destination_mac = 0;
        }
    }

    if ( arg_destination_mac )
    {
        jdksavdecc_eui48_init_from_cstr( &destination_mac, arg_destination_mac );
    }

    if ( argc > ++arg )
    {
        arg_target_entity_id = argv[arg];
        if ( *arg_target_entity_id == 0 )
        {
            arg_target_entity_id = 0;
        }
    }

    if ( arg_target_entity_id )
    {
        jdksavdecc_eui64_init_from_cstr( &target_entity_id, arg_target_entity_id );
    }

    if ( argc > ++arg )
    {
        arg_sequence_id = argv[arg];
        sequence_id = (uint16_t)strtol( arg_sequence_id, 0, 0 );
    }
    else
    {
        sequence_id = 0;
    }

    if ( argc > ++arg )
    {
        /* Parse payload */
        size_t len = strlen( argv[arg] );
        const char *p = argv[arg];
        size_t i;
        command_payload_len = len / 2;
        if ( (size_t)command_payload_len > sizeof( command_payload ) )
        {
            fprintf( stderr, "Bad payload ascii form length\n" );
            return 1;
        }
        for ( i = 0; i < command_payload_len; ++i )
        {
            if ( !jdksavdecc_util_parse_byte( &command_payload[i], p[0], p[1] ) )
            {
                fprintf( stderr, "Bad payload octets ascii form\n" );
                return 1;
            }
            p += 2;
        }
    }

    if ( aecp_aem_form_msg( frame,
                            &aemdu,
                            message_type,
                            command_code,
                            sequence_id,
                            destination_mac,
                            target_entity_id,
                            command_payload,
                            command_payload_len ) == 0 )
    {
        if ( raw_send( net, frame->dest_address.value, frame->payload, frame->length ) > 0 )
        {
            if ( arg_verbose > 0 )
            {
                fprintf( stdout, "Sent:\n" );
                aecp_aem_print( stdout, frame, &aemdu );

                if ( arg_verbose > 1 )
                {
                    avdecc_cmd_print_frame_payload( stdout, frame );
                }
            }
            r = 0;
        }
        avdecc_cmd_process_incoming_raw( &aemdu, net, arg_time_in_ms_to_wait, aecp_aem_process );
    }
    else
    {
        fprintf( stderr, "avdecc: unable to form aem message\n" );
    }

    return r;
}

int aecp_aa( struct raw_context *net, struct jdksavdecc_frame *frame, uint16_t message_type, int argc, char **argv )
{
    (void)net;
    (void)frame;
    (void)message_type;
    (void)argc;
    (void)argv;
    fprintf( stderr, "TODO: aecp aa\n" );
    return 1;
}

int aecp_avc( struct raw_context *net, struct jdksavdecc_frame *frame, uint16_t message_type, int argc, char **argv )
{
    (void)net;
    (void)frame;
    (void)message_type;
    (void)argc;
    (void)argv;
    fprintf( stderr, "TODO: aecp avc\n" );
    return 1;
}

int aecp_hdcp_apm( struct raw_context *net, struct jdksavdecc_frame *frame, uint16_t message_type, int argc, char **argv )
{
    (void)net;
    (void)frame;
    (void)message_type;
    (void)argc;
    (void)argv;
    fprintf( stderr, "TODO: aecp hdcp apm\n" );
    return 1;
}

int aecp_vendor( struct raw_context *net, struct jdksavdecc_frame *frame, uint16_t message_type, int argc, char **argv )
{
    (void)net;
    (void)frame;
    (void)message_type;
    (void)argc;
    (void)argv;
    fprintf( stderr, "TODO: aecp vendor\n" );
    return 1;
}

int aecp_extended( struct raw_context *net, struct jdksavdecc_frame *frame, uint16_t message_type, int argc, char **argv )
{
    (void)net;
    (void)frame;
    (void)message_type;
    (void)argc;
    (void)argv;
    fprintf( stderr, "TODO: aecp extended\n" );
    return 1;
}

int aecp( struct raw_context *net, struct jdksavdecc_frame *frame, int argc, char **argv )
{
    int r = 1;
    int parsed = 0;

    if ( arg_message_type )
    {
        uint16_t message_type_code = 0;
        if ( jdksavdecc_get_uint16_value_for_name( jdksavdecc_aecp_print_message_type, arg_message_type, &message_type_code ) )
        {
            switch ( message_type_code )
            {
            case JDKSAVDECC_AECP_MESSAGE_TYPE_ADDRESS_ACCESS_COMMAND:
            case JDKSAVDECC_AECP_MESSAGE_TYPE_ADDRESS_ACCESS_RESPONSE:
                parsed = 1;
                r = aecp_aa( net, frame, message_type_code, argc, argv );
                break;
            case JDKSAVDECC_AECP_MESSAGE_TYPE_AEM_COMMAND:
            case JDKSAVDECC_AECP_MESSAGE_TYPE_AEM_RESPONSE:
                parsed = 1;
                r = aecp_aem( net, frame, message_type_code, argc, argv );
                break;
            case JDKSAVDECC_AECP_MESSAGE_TYPE_AVC_COMMAND:
            case JDKSAVDECC_AECP_MESSAGE_TYPE_AVC_RESPONSE:
                parsed = 1;
                r = aecp_avc( net, frame, message_type_code, argc, argv );
                break;
            case JDKSAVDECC_AECP_MESSAGE_TYPE_HDCP_APM_COMMAND:
            case JDKSAVDECC_AECP_MESSAGE_TYPE_HDCP_APM_RESPONSE:
                parsed = 1;
                r = aecp_hdcp_apm( net, frame, message_type_code, argc, argv );
                break;
            case JDKSAVDECC_AECP_MESSAGE_TYPE_VENDOR_UNIQUE_COMMAND:
            case JDKSAVDECC_AECP_MESSAGE_TYPE_VENDOR_UNIQUE_RESPONSE:
                parsed = 1;
                r = aecp_vendor( net, frame, message_type_code, argc, argv );
                break;
            case JDKSAVDECC_AECP_MESSAGE_TYPE_EXTENDED_COMMAND:
            case JDKSAVDECC_AECP_MESSAGE_TYPE_EXTENDED_RESPONSE:
                parsed = 1;
                r = aecp_extended( net, frame, message_type_code, argc, argv );
                break;
            }
        }
    }

    if ( !parsed )
    {
        struct jdksavdecc_uint16_name *name = jdksavdecc_aecp_print_message_type;
        fprintf( stdout, "aecpdu message type options:\n" );
        while ( name->name )
        {
            fprintf( stdout, "\t0x%04x %s\n", name->value, name->name );
            name++;
        }
    }
    return r;
}
