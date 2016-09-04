/*
* @file
* @brief expose to user for sending data to extern system
* @ingroup packet tx in the system
* @cond
******************************************************************************
* Last updated for version 1.0.0
* Last updated on  2016-09-04
*
*                    Moltanisk Liang
*                    ---------------------------
*                    avb auto control system
*
* Copyright (C) Moltanisk Liang, GuangZhou Suntron. All rights reserved.
******************************************************************************
* @endcond
*/
/*Including files-----------------------------------------------------------*/
#include "system_packet_tx.h"
#include "adp_controller_machine.h"
#include "acmp_controller_machine.h"
#include "aecp_controller_machine.h"
#include "udp_server_controller_machine.h"
#include "udp_client_controller_machine.h"
#include "wait_message.h"
#include "camera_uart_controller_machine.h"
#include "send_common.h" /* °üº¬SEND_DOUBLE_QUEUE_EABLE*/
#include "matrix_output_input.h"

/*Local Objects-------------------------------------------------------------*/
static uint8_t gsys_tx_buf[TRANSMIT_DATA_BUFFER_SIZE];/*system tx buffer*/
/*Local functions declaration-----------------------------------------------*/
static int system_raw_queue_tx(void *frame, uint16_t frame_len, uint8_t data_type,
                                      const uint8_t dest_mac[6], bool isresp);
static int system_udp_queue_tx(void *frame, uint16_t frame_len,
                            uint8_t data_type, const struct sockaddr_in *sin);
static int system_uart_queue_tx(void *frame, uint16_t frame_len,
                                              uint8_t data_type, bool isresp);
/*$system_raw_queue_tx-----------------------------------------------------*/
static int system_raw_queue_tx(void *frame, uint16_t frame_len, uint8_t data_type,
                                       const uint8_t dest_mac[6], bool isresp)
{
    int ret;
    assert(frame);

    if ((data_type == TRANSMIT_TYPE_ADP)
         || (data_type == TRANSMIT_TYPE_ACMP)
         || (data_type == TRANSMIT_TYPE_AECP))
    {
        tx_data tx;
        if (frame_len > TRANSMIT_DATA_BUFFER_SIZE) {
            DEBUG_INFO( "frame_len bigger than pipe transmit buffer!");
            return -1;
        }

        memset(gsys_tx_buf, 0, sizeof(gsys_tx_buf));
        memcpy(tx.raw_dest, dest_mac, sizeof(struct jdksavdecc_eui48));
        memcpy(gsys_tx_buf, (uint8_t*)frame, frame_len);
        tx.frame = gsys_tx_buf;
        tx.data_type = data_type;
        tx.frame_len = frame_len;
        tx.notification_flag = RUNINFLIGHT;
        tx.resp = isresp;
        
        ret = system_packet_save_send_queue(tx);    /* save to system queue */
    }
    else {
        ret = -1;
        DEBUG_INFO( "ERR transmit data type" );
    }
    return ret;
}

/****************************************************************************/
/**
* @description
* This function is to send a frame data to system double queue
* @param[in] dest_mac destination of sending data
*                    frame sending data
*                    frame_len sending date len
*		      notification decide data whether send
*		      data_type type of data sending 
*		      isresp the flags of respond send data
* @returns -1 err for no data type in the system
*/
int system_raw_packet_tx(const uint8_t dest_mac[6], void *frame,
        uint16_t frame_len, bool notification, uint8_t data_type, bool isresp)
{
	assert((dest_mac != NULL) && (frame != NULL));
	return system_raw_queue_tx(frame, frame_len, data_type, dest_mac, isresp);
}

/*$system_udp_queue_tx------------------------------------------------------*/
static int system_udp_queue_tx(void *frame, uint16_t frame_len,
                             uint8_t data_type, const struct sockaddr_in *sin)
{
    int ret;
    assert((sin != NULL) && (frame != NULL));
    
    if ((data_type == TRANSMIT_TYPE_UDP_SVR)
         || (data_type == TRANSMIT_TYPE_UDP_CLT))
    {
        tx_data tx;
        bool resp;
        resp = is_conference_deal_data_response_type(frame,
                                                CONFERENCE_RESPONSE_POS);
        if (frame_len > TRANSMIT_DATA_BUFFER_SIZE) {
            DEBUG_INFO("frame_len bigger than pipe transmit buffer!");
            return -1;
        }

        memset(gsys_tx_buf, 0, sizeof(gsys_tx_buf));
        memcpy(gsys_tx_buf, frame, frame_len);
        tx.frame = gsys_tx_buf;
        tx.data_type = data_type;
        tx.frame_len = frame_len;
        tx.notification_flag = RUNINFLIGHT;
        tx.resp = resp;
        memcpy(&tx.udp_sin, sin, sizeof(struct sockaddr_in));
        
        ret = system_packet_save_send_queue(tx);
    }
    else {
        ret = -1;
        DEBUG_INFO( "transmit data type not udp clt or srv" );
    }
    return ret;
}

/****************************************************************************/
/**
* @description
* @param[in] sin pointer context of destination udp
*                    frame point to frame buffer
*                    frame_len frame len
*                    notication frame send status notify or not
*                    data_type system sending data type
* @returns -1 err for no data type in the system
*/
int system_udp_packet_tx(const struct sockaddr_in *sin, void *frame,
                     uint16_t frame_len, bool notification, uint8_t data_type)
{
    assert((sin != NULL) && (frame != NULL));
    return system_udp_queue_tx(frame, frame_len, data_type, sin);
}

/*$system_uart_queue_tx-----------------------------------------------------*/
static int system_uart_queue_tx(void *frame, uint16_t frame_len,
                                               uint8_t data_type, bool isresp)
{
    int ret;
    bool resp;
    assert(frame != NULL);

    if ((data_type == TRANSMIT_TYPE_CAMERA_UART_CTRL)
         || (data_type == TRANSMIT_TYPE_MATRIX_UART_CTRL))
    {
        tx_data tx;
        resp = true; /* no need camera response data */

        if (frame_len > TRANSMIT_DATA_BUFFER_SIZE) {
            DEBUG_INFO("frame_len bigger than pipe transmit buffer!");
            return -1;
        }

        memset(gsys_tx_buf, 0, TRANSMIT_DATA_BUFFER_SIZE);
        memcpy(gsys_tx_buf, frame, frame_len);
        tx.frame = gsys_tx_buf;
        tx.data_type = data_type;
        tx.frame_len = frame_len;
        tx.notification_flag = RUNINFLIGHT;
        tx.resp = resp;

        ret = system_packet_save_send_queue(tx);
    }
    else {
        ret = -1;
        DEBUG_INFO("transmit data type not uart data!");
    }
    return ret;
}

/****************************************************************************/
/**
* @description
* @param[in] frame pointer to data buf
*                    frame_len frame len of data sended
*                    notication frame send status notify or not
*                    data_type system sending data type
*                    isresp is response or not
* @returns -1 err for no data type in the system
*/
int system_uart_packet_tx(void *frame, uint16_t frame_len,
                            bool notification, uint8_t data_type, bool isresp)
{
	assert( frame);	
	return system_uart_queue_tx(frame, frame_len, data_type, isresp);
}

/****************************************************************************/
/**
* @description
* @param[in] frame pointer to data buf
*                    frame_len frame len of data sended
*                    notication frame send status notify or not
*                    data_type system sending data type
*                    isresp is response or not
*                    dest_mac 1722 protocal destination
*                    sin udp destination context
* @returns -1 err for no data type in the system
*/
int system_tx(void *frame, uint16_t frame_len, bool notification,
                     uint8_t data_type,bool isresp, const uint8_t dest_mac[6],
                                                const struct sockaddr_in *sin)
{
    int ret;/*return value*/
    uint8_t raw_dest[6] = {0};
    struct sockaddr_in  tmp_sin;
    uint8_t genre_tx = data_type;/* send queue type */

    switch( genre_tx ) {
        case TRANSMIT_TYPE_ADP:
        case TRANSMIT_TYPE_ACMP:
        case TRANSMIT_TYPE_AECP: { /* same ether protocol */
            if(dest_mac != NULL) {
                memcpy(raw_dest, dest_mac, 6);
                ret = system_raw_packet_tx(raw_dest, frame, frame_len, 
                                             notification, data_type, isresp);
            }
            break;
        }
        case TRANSMIT_TYPE_UDP_SVR:
        case TRANSMIT_TYPE_UDP_CLT: {
            if (sin != NULL) {
                memcpy(&tmp_sin, sin, sizeof(struct sockaddr_in));
                ret = system_udp_packet_tx(&tmp_sin, frame, frame_len,
                                                    notification,  data_type);
            }
            break;
        }
        case TRANSMIT_TYPE_CAMERA_UART_CTRL:
        case TRANSMIT_TYPE_MATRIX_UART_CTRL: {
            ret = system_uart_packet_tx(frame, frame_len, notification,
                                                           data_type, isresp);
            break;
        }
        default: {
            ret = -1;
            DEBUG_INFO("SYSTEM TX ERR: NO such data type(%d)", genre_tx);
            break;
        }
    }
    return ret;
}

/****************************************************************************/
/**
* @description
* This function must be called in single sending pthread in the system
* It's is the only function for sending  data to extern system.
* @Note the len buffer of frame must bigger than 50 bytes.
*/
int tx_packet_event(uint8_t type, bool notification_flag,	uint8_t *frame,
               uint16_t frame_len, struct fds *file_dec, inflight_plist guard,
                                 uint8_t dest_mac[6], struct sockaddr_in* sin,
                                           bool resp, uint32_t *interval_time)
{
    int server_fd;
    int client_fd;
    uint8_t dest[6];
    uint64_t dest_addr;
    struct sockaddr_in sin_event;

    server_fd = file_dec->udp_server_fd;/* as udp server fd */
    client_fd = file_dec->udp_client_fd;/* as udp client fd */
    memcpy(&sin_event, sin, sizeof(struct sockaddr_in));
    /* convert mac to target id of 64 bits */
    convert_eui48_to_uint64(dest_mac, &dest_addr);
    /* check the valid address,if not valid use the whole values */
    if (dest_addr != 0) {
        memcpy(dest, dest_mac, 6);
    }
    else {
        memcpy(dest, jdksavdecc_multicast_adp_acmp.value, 6);
    }

    assert(interval_time);
    if (type == TRANSMIT_TYPE_ADP) {
        transmit_adp_packet_to_net(frame, frame_len, NULL,
                                            false, dest, resp, interval_time);
    }
    else if (type == TRANSMIT_TYPE_ACMP) {
        transmit_acmp_packet_network(frame, frame_len, NULL,
                                            false, dest, resp, interval_time);
    }
    else if (type == TRANSMIT_TYPE_AECP) {
        transmit_aecp_packet_network(frame, frame_len, NULL,
                                            false, dest, resp, interval_time);
    }
    else if (type == TRANSMIT_TYPE_UDP_SVR) {
        /* host as client send data to udp server using client fd */
        transmit_udp_packet_server(client_fd, frame, frame_len, NULL,
                                      false, &sin_event, resp, interval_time);
    }
    else if (type == TRANSMIT_TYPE_UDP_CLT) {
        /* host as server send data to udp client using server fd */
        transmit_udp_client_packet(server_fd, frame, frame_len, NULL,
                                      false, &sin_event, resp, interval_time);
    }
    else if (type == TRANSMIT_TYPE_CAMERA_UART_CTRL) { 
        transmit_camera_uart_control_packet(frame, frame_len, false,
                                                         resp, interval_time);
    }
    else if (type == TRANSMIT_TYPE_MATRIX_UART_CTRL) {
        matrix_output_transmit_uart_control_packet(frame, frame_len, false,
                                                         resp, interval_time);
    }
    else {
        /* will never come in this else */
    }
    return 0;
}

