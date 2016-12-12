/*
* @file arcs_process.h
* @brief avdecc reception control system processing
* @ingroup avdecc reception control system module
* @cond
******************************************************************************
* Build Date on  2016-12-5
* Last updated for version 1.0.0
* Last updated on  2016-12-5
*
*                    Moltanisk Liang
*                    ---------------------------
*                    avb auto control system
*
* Copyright (C) Moltanisk Liang, GuangZhou Suntron. All rights reserved.
******************************************************************************
* @endcond
*/
/*$ Including file----------------------------------------------------------*/
#include "arcs_common.h"
#include "terminal_pro.h"
#include "central_control_transmit_unit.h"
#include "conference_transmit_unit.h"
#include "control_matrix_pro.h"
#include "upper_computer_common.h"
#include "camera_pro.h"
#include "system_database.h"
#include "descriptor.h"
#include "entity.h"
#include "linked_list_unit.h"
#include "log_machine.h"
#include "terminal_system.h"
#include "system.h"
#include "arcs_process.h"
#include <readline/readline.h>
#include <readline/history.h>
/*$ Local function declaration----------------------------------------------*/
static void ArcsProcess_vote(uint8_t optCode, uint8_t *sign);
static void ArcsProcess_sign(uint8_t optCode);
/*$ Local arcs seq id-------------------------------------------------------*/
static uint16_t l_arcsSeq;
extern solid_pdblist endpoint_list;
extern desc_pdblist descptor_guard;
extern uint8_t gcamera_levelspeed;
extern uint8_t gcamera_vertspeed;
extern struct threads_info threads;
/*$ */
int ArcsCmd_queryId(uint16_t cmd, void *data, uint32_t data_len) {
    TProtocalQt const * const pMsg = (TProtocalQt *)data;
    TProtocalQt buf;
    TProtocalQtQueryData qData; /* query data */
    int msgLen;
    tmnl_pdblist pDev;
    uint16_t addr, qAddr, dataLen;
    uint8_t tNum; /* tmnl number */
    if (pMsg == (TProtocalQt const * const)0) {
        return -1;
    }
    /* check query type */
    if (pMsg->type != PRO_QUERY_TYPE) { /* query type */
        return -1;
    }
    /* set query address */
    qAddr = *((uint16_t *)&pMsg->dataBuf[0]);
    if (qAddr == 0xffff) {/* query all */
        tNum = 0;
        dataLen = 0;
        /* reponse to arcs */
        memset(&buf, 0, sizeof(TProtocalQt));
        buf.head = PROTOCAL_QT_TYPE;
        buf.type = PRO_QUERY_TYPE | PRO_RESP_MASK;
        buf.seq = pMsg->seq;
        buf.cmd = QT_QUEUE_ID;
        buf.dataLen = 0;
        msgLen = PRO_COMMON_LEN;
        ArcsCommon_send(&buf, true, msgLen);

        solid_pdblist pSolid = endpoint_list;
        tNum = 0;
        for (pSolid = pSolid->next;
                (pSolid != endpoint_list)
                    && (pSolid != NULL);
                pSolid = pSolid->next)
        {
            if (tNum == 0) {
                memset(buf.dataBuf, 0, PRO_QT_MAX);
            }
            memset(&qData, 0, sizeof(qData));
            pDev = found_terminal_dblist_node_by_endtity_id(
                pSolid->solid.entity_id);
            if (pDev != NULL) {
                addr = pDev->tmnl_dev.address.addr;
                qData.id = addr;
                qData.rgst = pDev->tmnl_dev.tmnl_status.is_rgst?1:0;
                qData.sign = pDev->tmnl_dev.tmnl_status.sign_state?1:0;
                qData.vote = pDev->tmnl_dev.tmnl_status.is_vote?1:0;
                qData.select = pDev->tmnl_dev.tmnl_status.is_select?1:0;
                qData.grade = pDev->tmnl_dev.tmnl_status.is_grade?1:0;
                /* set identify */
                qData.permision |=
                    (uint8_t)pDev->tmnl_dev.address.tmn_type;
                qData.micStatus |=
                    (uint8_t)pDev->tmnl_dev.tmnl_status.mic_state;
            }
            
            struct jdksavdecc_string endName;
            uint16_t cnNum = 0;
            memset(&endName, 0, sizeof(endName));
            if (pSolid != NULL) {            
                qData.online = (pSolid->solid.connect_flag == CONNECT)?1:0;
                /* get node of descriptor */
                desc_pdblist pD = search_desc_dblist_node(
                    pSolid->solid.entity_id,
                    descptor_guard);
                if (pD != NULL) {
                    memcpy(&endName, pD->endpoint_desc.entity_name.value,
                        sizeof(endName));
                    if (strcmp((char *)endName.value,
                        conference_uint_name) ==0)
                    {
                        qData.avbIdentity = 3; /* conferent unit */
                        /* search for tark all connect num */
                        cnNum = conference_transmit_unit_allcount_cnnts(
                            pSolid->solid.entity_id);
                    }
                    else {
                        if (strcmp((char *)&endName,
                                central_control_unit_transmit_name) == 0)
                        {
                            qData.avbIdentity = 1;
                            cnNum = central_control_transmit_unit_allcount_cnnts(
                                pSolid->solid.entity_id);  
                        }
                        else if (strcmp((char *)&endName,
                                central_control_unit_name) == 0) {
                            qData.avbIdentity = 2;
                            cnNum = central_control_transmit_unit_allcount_cnnts(
                                pSolid->solid.entity_id);
                        }
                        else {
                            /* error case do nothing */
                        }
                    }
                }
            }
            
            qData.cnntNum = cnNum;
            /* set 1722 id */
            memcpy(&qData.id_1722, &pSolid->solid.entity_id, 8);
            /* set name */
            memcpy(qData.name, &endName, sizeof(endName));
            /* copy to databuf */
            uint16_t pos = tNum * sizeof(qData);
            memcpy(buf.dataBuf + pos, &qData, sizeof(qData));
            tNum++; /* inc */
            dataLen = tNum * sizeof(qData);
            if ((tNum == 3)
                  ||(pSolid->next == endpoint_list))
            {/* end or 3 count terminal number */
                tNum = 0;
                /* system tx packet */
                /* report tmnl information */
                buf.head = PROTOCAL_QT_TYPE;
                buf.type = PRO_REPORT_TYPE;
                buf.seq = ++l_arcsSeq;
                buf.cmd = QT_QUEUE_ID;
                buf.dataLen = dataLen;
                msgLen = buf.dataLen + PRO_COMMON_LEN;
                /* save buffer to query id command queue */
                if (msgLen <= PRO_QT_MAX) {
                    TArcsQrBuf *pBuf =
                        (TArcsQrBuf *)malloc(sizeof(TArcsQrBuf));
                    if (pBuf != NULL) {
                        pBuf->len = msgLen;
                        memset(pBuf->buf, 0, PRO_QT_MAX);
                        memcpy(pBuf->buf, &buf, msgLen);
                        ArcsCommon_postQrQueue((void *)pBuf);
                    }
                }
            }
        }
    }
    else {
        bool found = (bool)0;
        for (pDev = dev_terminal_list_guard->next;
                (pDev != dev_terminal_list_guard)
                    && (pDev != NULL);
                pDev = pDev->next)
        {
            if (pDev->tmnl_dev.address.addr == qAddr) {
                found = (bool)1;
                break;
            }
        }
        /* reponse to arcs */
        memset(&buf, 0, sizeof(TProtocalQt));
        buf.head = PROTOCAL_QT_TYPE;
        buf.type = PRO_QUERY_TYPE | PRO_RESP_MASK;
        if (!found) {
            buf.type |= 0x01; /* no such id */
        }
        buf.seq = pMsg->seq;
        buf.cmd = QT_QUEUE_ID;
        buf.dataLen = 2;
        buf.dataBuf[0] = pMsg->dataBuf[0];
        buf.dataBuf[1] = pMsg->dataBuf[1];
        msgLen = buf.dataLen + PRO_COMMON_LEN;
        ArcsCommon_send(&buf, true, msgLen);
        
        if (found) {
            memset(buf.dataBuf, 0, PRO_QT_MAX);
            memset(&qData, 0, sizeof(qData));
            addr = pDev->tmnl_dev.address.addr;
            qData.id = addr;
            qData.rgst = pDev->tmnl_dev.tmnl_status.is_rgst?1:0;
            qData.sign = pDev->tmnl_dev.tmnl_status.sign_state?1:0;
            qData.vote = pDev->tmnl_dev.tmnl_status.is_vote?1:0;
            qData.select = pDev->tmnl_dev.tmnl_status.is_select?1:0;
            qData.grade = pDev->tmnl_dev.tmnl_status.is_grade?1:0;
            solid_pdblist pSolid = search_endtity_node_endpoint_dblist(
                    endpoint_list,
                    pDev->tmnl_dev.entity_id);
            struct jdksavdecc_string endName;
            uint16_t cnNum = 0;
            memset(&endName, 0, sizeof(endName));
            if (pSolid != NULL) {            
                qData.online = (pSolid->solid.connect_flag == CONNECT)?1:0;
                /* get node of descriptor */
                desc_pdblist pD = search_desc_dblist_node(
                    pDev->tmnl_dev.entity_id,
                    descptor_guard);
                if (pD != NULL) {
                    memcpy(&endName, pD->endpoint_desc.entity_name.value,
                        sizeof(endName));
                    if (strcmp((char *)endName.value,
                        conference_uint_name) ==0)
                    {
                        qData.avbIdentity = 3;
                        /* search for tark all connect num */
                        cnNum = conference_transmit_unit_allcount_cnnts(
                            pDev->tmnl_dev.entity_id);
                    }
                    else {
                        if (strcmp((char *)&endName,
                                central_control_unit_transmit_name) == 0)
                        {
                            qData.avbIdentity = 1;
                            cnNum = central_control_transmit_unit_allcount_cnnts(
                                pSolid->solid.entity_id);  
                        }
                        else if (strcmp((char *)&endName,
                                central_control_unit_name) == 0) {
                            qData.avbIdentity = 2;
                            cnNum = central_control_transmit_unit_allcount_cnnts(
                                pSolid->solid.entity_id);
                        }
                        else {
                            /* error case do nothing */
                        }
                    }
                }
            }
            /* set identify */
            qData.permision |= (uint8_t)pDev->tmnl_dev.address.tmn_type;
            qData.micStatus |= (uint8_t)pDev->tmnl_dev.tmnl_status.mic_state;
            
            qData.cnntNum = cnNum;
            /* set 1722 id */
            memcpy(&qData.id_1722, &pDev->tmnl_dev.entity_id, 8);
            /* set name */
            memcpy(qData.name, &endName, sizeof(endName));
            /* copy to databuf */
            memcpy(buf.dataBuf, &qData, sizeof(qData));
            /* report tmnl information */
            buf.head = PROTOCAL_QT_TYPE;
            buf.type = PRO_REPORT_TYPE;
            buf.seq = ++l_arcsSeq;
            buf.cmd = QT_QUEUE_ID;
            buf.dataLen = sizeof(qData);
            msgLen = buf.dataLen + PRO_COMMON_LEN;
            if (msgLen <= PRO_QT_MAX) {
                TArcsQrBuf *pBuf =
                    (TArcsQrBuf *)malloc(sizeof(TArcsQrBuf));
                if (pBuf != NULL) {
                    pBuf->len = msgLen;
                    memset(pBuf->buf, 0, PRO_QT_MAX);
                    memcpy(pBuf->buf, &buf, msgLen);
                    ArcsCommon_postQrQueue((void *)pBuf);
                }
            }
        }
    }
    /* return value */
    return 0;
}
/*$ */
int ArcsCmd_switchMatrix(uint16_t cmd, void *data, uint32_t data_len) {
    TProtocalQt const * const pMsg = (TProtocalQt *)data;
    TProtocalQt buf;
    int msgLen;
    if (pMsg == (TProtocalQt const * const)0) {
        return -1;
    }
    /* reponse to arcs */
    memset(&buf, 0, sizeof(TProtocalQt));
    buf.head = PROTOCAL_QT_TYPE;
    buf.type = PRO_QUERY_TYPE | PRO_RESP_MASK;
    buf.seq = pMsg->seq;
    buf.cmd = QT_SWITCH_MATRIX;
    buf.dataLen = 0;
    msgLen = PRO_COMMON_LEN;
    ArcsCommon_send(&buf, true, msgLen);
    /* check setting type */
    if (pMsg->type & PRO_SET_TYPE) { /* set type */
        uint8_t in = (pMsg->dataBuf[0] & 0x0f) + 1;
        uint8_t out = ((pMsg->dataBuf[0] & 0xf0) >> 4) + 1;
        char str[5] = {0};
        if (NULL != gp_log_imp) {
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_DEBUG,
                "[ArcsCmd_switchMatrix in-ou(%d-%d)]",
                in,
                out);
        }
        str[0] = in;
        str[1] = 'B';
        str[2] = out;
        str[3] = '.';
        control_matrix_switch(str, 4);
        return 0;
    }

    return -1;
}
/*$ */
int ArcsCmd_optTmnl(uint16_t cmd, void *data, uint32_t data_len) {
    TProtocalQt const * const pMsg = (TProtocalQt *)data;
    TProtocalQt buf;
    int msgLen;
    if (pMsg == (TProtocalQt const * const)0) {
        return -1;
    }
    /* check set type */
    if (pMsg->type & PRO_SET_TYPE) { /* set type */
        uint8_t optCode = pMsg->dataBuf[0] & 0x0f;
        uint8_t optType = (pMsg->dataBuf[0] & 0xf0) >> 4u;
        uint8_t signFg;
        tcmpt_data_mic_switch mic;
        tmnl_pdblist pDev = dev_terminal_list_guard;
        uint16_t addr;
        bool f_ = (bool)0;
        switch (optType) {
            case 0: { /* talk */
                /* found avail address */
                addr = *((uint16_t*)&pMsg->dataBuf[1]);
                printf("addr = 0x%04x\n", addr);
                for (pDev = pDev->next;
                    (pDev != dev_terminal_list_guard)
                        && (pDev != NULL);
                    pDev = pDev->next)
                {
                    if (pDev->tmnl_dev.address.addr == addr) {
                        /*  */
                        f_ = (bool)1;
                        break;
                    }
                }
                if (f_) {/* found */
                    /* reponse to arcs */
                    memset(&buf, 0, sizeof(TProtocalQt));
                    buf.head = PROTOCAL_QT_TYPE;
                    buf.type = PRO_SET_TYPE | PRO_RESP_MASK;
                    buf.seq = pMsg->seq;
                    buf.cmd = QT_OPT_TMNL;
                    buf.dataLen = 3;
                    buf.dataBuf[0] = pMsg->dataBuf[0];
                    buf.dataBuf[1] = pMsg->dataBuf[1];
                    buf.dataBuf[2] = pMsg->dataBuf[2];
                    msgLen = PRO_COMMON_LEN + buf.dataLen;
                    ArcsCommon_send(&buf, true, msgLen);

                    mic.addr.low_addr = pMsg->dataBuf[1];
                    mic.addr.high_addr = pMsg->dataBuf[2];
                    if (optCode == 0) {/* begin */
                        mic.switch_flag = 1;
                        terminal_upper_computer_speak_proccess(mic);
                    }
                    else if (optCode == 3) {
                        mic.switch_flag = 0;
                        terminal_upper_computer_speak_proccess(mic);
                    }
                    else {
                        /* error option code */
                    }
                }
                else {
                    /* set error code */
                    memset(&buf, 0, sizeof(TProtocalQt));
                    buf.head = PROTOCAL_QT_TYPE;
                    /* 0x07 is no such id */
                    buf.type = (PRO_SET_TYPE | PRO_RESP_MASK) | 0x07;
                    buf.seq = pMsg->seq;
                    printf("buf.seq = %d\n", pMsg->seq);
                    buf.cmd = QT_OPT_TMNL;
                    buf.dataLen = 3;
                    buf.dataBuf[0] = pMsg->dataBuf[0];
                    buf.dataBuf[1] = pMsg->dataBuf[1];
                    buf.dataBuf[2] = pMsg->dataBuf[2];
                    msgLen = PRO_COMMON_LEN + buf.dataLen;
                    ArcsCommon_send(&buf, true, msgLen);
                }
                break;
            }
            case 1: { /* sign */
                ArcsProcess_sign(optCode);
                
                buf.head = PROTOCAL_QT_TYPE;
                buf.type = PRO_SET_TYPE | PRO_RESP_MASK;
                buf.seq = pMsg->seq;
                buf.cmd = QT_OPT_TMNL;
                buf.dataLen = 1;
                buf.dataBuf[0] = pMsg->dataBuf[0];
                msgLen = PRO_COMMON_LEN + buf.dataLen;
                ArcsCommon_send(&buf, true, msgLen);
                break;
            }
            case 2: { /* vote */
                ArcsProcess_vote(optCode, &signFg);
                if (optCode == 0) {
                    /* set error code */
                }
                else {
                    /* set error code */
                }
                buf.head = PROTOCAL_QT_TYPE;
                buf.type = PRO_SET_TYPE | PRO_RESP_MASK;
                buf.seq = pMsg->seq;
                buf.cmd = QT_OPT_TMNL;
                buf.dataLen = 1;
                buf.dataBuf[0] = pMsg->dataBuf[0];
                msgLen = PRO_COMMON_LEN + buf.dataLen;
                ArcsCommon_send(&buf, true, msgLen);
                break;
            }
            default: {
                /* error */
                 if (NULL != gp_log_imp) {
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_ERROR,
                        "[ArcsCmd_optTmnl Error type(%d)]",
                        optType);
                }
                break;
            }
        }
        return 0;
    }

    return -1;
}
/*$ */
int ArcsCmd_sysSet(uint16_t cmd, void *data, uint32_t data_len) {
    /* do thing */
    TProtocalQt const * const pMsg = (TProtocalQt *)data;
    TProtocalQt buf;
    int msgLen;
 
    /* check set type */
    if (pMsg->type & PRO_SET_TYPE) { /* set type */
        if (((pMsg->dataBuf[0] & 0xfe) >> 1) == 0) { /* stop sytem*/
            /* update, reponse by 'abcs',there is no need */
            write(fileno(rl_instream), "arcsE", 5); /* set end readline */
            system_close(&threads);
        }
        else {
            memset(&buf, 0, sizeof(TProtocalQt));
            buf.head = PROTOCAL_QT_TYPE;
            buf.type = PRO_QUERY_TYPE | PRO_RESP_MASK;
            buf.seq = pMsg->seq;
            buf.cmd = QT_CMR_CTL;
            buf.dataLen = 0;
            msgLen = PRO_COMMON_LEN;
            ArcsCommon_send(&buf, true, msgLen);
        }
    }

    return -1;
}
/*$ */
int ArcsCmd_cmrCtl(uint16_t cmd, void *data, uint32_t data_len) {
    TProtocalQt const * const pMsg = (TProtocalQt *)data;
    TProtocalQt buf;
    int msgLen;
    if (pMsg == (TProtocalQt const * const)0) {
        return -1;
    }
    /* reponse to arcs */
    memset(&buf, 0, sizeof(TProtocalQt));
    buf.head = PROTOCAL_QT_TYPE;
    buf.type = PRO_QUERY_TYPE | PRO_RESP_MASK;
    buf.seq = pMsg->seq;
    buf.cmd = QT_CMR_CTL;
    buf.dataLen = 0;
    msgLen = PRO_COMMON_LEN;
    ArcsCommon_send(&buf, true, msgLen);
    /* check set type */
    if (pMsg->type & PRO_SET_TYPE) { /* set type */
        uint8_t curCmr = pMsg->dataBuf[0];
        gcamera_levelspeed = pMsg->dataBuf[1];
        gcamera_vertspeed = pMsg->dataBuf[2];
        uint8_t optCode = (pMsg->dataBuf[0] & 0xf0) >> 4;
        uint8_t optType = pMsg->dataBuf[0] & 0x0f;
        if (optCode == 0) {/* switch */
            camera_select_num(0, &curCmr, sizeof(uint8_t));
            gset_sys.current_cmr = curCmr;
            system_db_update_configure_system_table(gset_sys);
            profile_system_file_write_gb_param(profile_file_fd, &gset_sys);
            Fflush(profile_file_fd);
        }
        else if (optCode == 1) {
            uint8_t temp;
            switch (optType) {
                case 0:
                case 4:
                case 5: { /* up */
                    temp = 1;
                    camera_control_updown(0, &temp,sizeof(temp));
                    break;
                }
                case 1:
                case 6:
                case 7: { /* down */
                    temp = 0;
                    camera_control_updown(0, &temp, sizeof(temp));
                    break;
                }
                case 2: { /* left */
                    temp = 0;
                    camera_control_rightleft(0, &temp, sizeof(temp));
                    break;
                }
                case 3: { /* right */
                    temp = 1;
                    camera_control_rightleft(0, &temp, sizeof(temp));
                    break;
                }
                case 8: { /* iris */
                    temp = 0;
                    camera_control_iris(0, &temp, sizeof(temp));
                    break;
                }
                case 9: { /* iris bigger */
                    temp = 1;
                    camera_control_iris(0, &temp, sizeof(temp));
                    break;
                }
                case 10: { /* near */
                    temp = 1;
                    camera_control_fouce(0, &temp, sizeof(temp));
                    break;
                }
                case 11: {/* far */
                    temp = 0;
                    camera_control_fouce(0, &temp, sizeof(temp));
                    break;
                }
                case 12: {/* ZOOM_WIDE */
                    temp = 0;
                    camera_control_zoom(0, &temp, sizeof(temp));
                    break;
                }
                case 13: {/* ZOOM_TELE */
                    temp = 1;
                    camera_control_zoom(0, &temp, sizeof(temp));
                    break;
                }
                default: {
                    break;
                }
            }
        }
        else { /* err code */
        }
        return 0;
    }

    return -1;
}

static void ArcsProcess_vote(uint8_t optCode, uint8_t *sign) {
    switch (optCode) {
        case 0: { /* begin */
            terminal_chman_control_begin_vote(VOTE_MODE, false,
                                sign);/* last key value */
            break;
        }
        case 1: {/* pause */
            /* pause voting */
            terminal_pause_vote(0, NULL, 0);
            break;
        }
        case 2: { /* regain */
            terminal_regain_vote(0, NULL, 0);
            break;
        }
        case 3: { /* stop */
            terminal_end_vote( 0, NULL, 0); 
            set_terminal_system_state(DISCUSS_STATE, true);
            terminal_start_discuss(false);
            break;
        }
        default: {
            break;
        }
    }
}

static void ArcsProcess_sign(uint8_t optCode) {
    switch (optCode) {
        case 0: { /* begin */
            Terminal_arcsStarSign();
            break;
        }
        case 3: { /* stop */
            terminal_end_sign(0, NULL, 0);
            break;
        }
        default: {
            break;
        }
    }
}

