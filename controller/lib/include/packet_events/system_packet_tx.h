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
#ifndef __SYSTEM_PACKET_TX_H__
#define __SYSTEM_PACKET_TX_H__

/*Including files-----------------------------------------------------------*/
#include "jdksavdecc_world.h"
#include "jdksavdecc_pdu.h"
#include "linked_list_unit.h"
#include "pipe.h"
#include "avdecc_net.h"
#include <semaphore.h>

/*$ transmit data buffer max size for Macro---------------------------------*/
#define TRANSMIT_DATA_BUFFER_SIZE 10240
/*$ conference response pos for Macro---------------------------------------*/
#define CONFERENCE_RESPONSE_POS 1

/*! data type for transmit extern data*/
enum transmit_data_type {
    TRANSMIT_TYPE_ADP,/*$ subtype of 1722.1 protocol*/
    TRANSMIT_TYPE_ACMP,/*$ subtype of 1722.1 protocol*/
    TRANSMIT_TYPE_AECP,/*$ subtype of 1722.1 protocol*/
    TRANSMIT_TYPE_UDP_SVR,/*$ udp server data*/
    TRANSMIT_TYPE_UDP_CLT,/*$ udp client data*/
    TRANSMIT_TYPE_CAMERA_UART_CTRL,/*$ camera extern control data*/
    TRANSMIT_TYPE_MATRIX_UART_CTRL,/*$ matrix extern control data*/
    TRANSMIT_TYPE_ARCS_CTRL/*$ arcs extern control data*/
};

/*! transmit data node type*/
typedef struct transmit_data {
    uint8_t data_type;/*date type of enum transmit_data_type*/
    bool notification_flag;/*sending status to notify*/
    bool resp;/*response data flags*/
    uint8_t *frame;/*pointer to data frame*/
    uint8_t raw_dest[6];/*raw destination*/
    uint16_t frame_len;/*frame len*/
    struct sockaddr_in udp_sin;/*udp context*/
}tx_data,*ptr_tx_data;

struct fds;/*fds declaration*/
/*! sending udp packet to double queue*/
extern int system_udp_packet_tx(const struct sockaddr_in *sin, void *frame,
                    uint16_t frame_len, bool notification, uint8_t data_type);
/*! sending raw packet to double queue*/
extern int system_raw_packet_tx(const uint8_t dest_mac[6], void *frame,
       uint16_t frame_len, bool notification, uint8_t data_type, bool isresp);
/*! sending uart packet to double queue*/
extern int system_uart_packet_tx(void *frame, uint16_t frame_len,
                           bool notification, uint8_t data_type, bool isresp);
/*! sending All extern system data to extern model*/
extern int tx_packet_event(uint8_t type, bool notification_flag,
                      uint8_t *frame,uint16_t frame_len, struct fds *file_dec,
         inflight_plist guard,	uint8_t dest_mac[6], struct sockaddr_in* sin,
                                          bool resp, uint32_t *interval_time);
/*! sending All extern system data to double queue*/
extern int system_tx(void *frame, uint16_t frame_len, bool notification,
                   uint8_t data_type, bool isresp, const uint8_t dest_mac[6],
                            				   const struct sockaddr_in *sin);
extern int system_arcs_packet_tx(void *frame, uint16_t frame_len,
                            bool notification, uint8_t data_type, bool isresp);
#endif

