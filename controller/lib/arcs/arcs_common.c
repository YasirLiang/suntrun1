/*
* @file arcs_common.c
* @brief avdecc reception control system module
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
#include "log_machine.h"
#include "extern_port.h" /*! Extern Port */
#include "ring_buffer.h"
#include "usertimer.h"
#include "inflight.h"
#include "system_packet_tx.h"
#include "arcs_extern_port.h"
#include "usertimer.h"
#include "arcs_process.h"
#include "arcs_common.h"
#include "queue_com.h"
#include <semaphore.h>
/*$ */
#define SEM_NAME "semArcs"
/*$ ARCS Recieve Buffer Process */
typedef struct TRingMsgPro {
    /*! state switch and interval timer */
    TUserTimer smTimer, itvTimer;
    /* receive message over flag */
    bool recvOver;
    /*! recieve message lenght */
    uint32_t msgLen;
}TRingMsgPro;
/*$ arcs common process type struct */
typedef struct {
    TExternPortVtbl *vptr;   /*$ arcs port virtual table */
    inflight_plist head;     /*$ the guard to system inflight */
    TCharRingBuf ringBuf;    /* circular process buffer */
    TRingMsgPro charRingPro; /*$ char ring buffer msg process */
    TProtocalQt msgPro;      /*$ arcs protocal msg storge */
    TComQueue *arcsQrQueue;  /*$ arcs query id queue */
    volatile bool qrFinishFlag;       /*$ query id cmd finish flag */
}TArcsCommon;
/*$ */
#define ARCS_RING_BUF_SIZE 1024
#define ARCS_RECV_BUF_SIZE 1024
#define ARCS_SEND_BUF_SIZE 512
/*$ enumration for queue array index----------------------------------------*/
enum {
    ARCS_QR,
    ARCS_CMD_NUM
};
/*$ enumration for queue size-----------------------------------------------*/
enum {
    /*! ONE arcs query id cmd element in queue
        \element contain most three terminal information
        \basing on the arcs protocal dataBuf size 256
        \and add three host unit */
    ARCS_QR_QSIZE =
        ((SYSTEM_TMNL_MAX_NUM + (3 - (SYSTEM_TMNL_MAX_NUM%3)))/3) + 3/3,
};
/*$ Local function declaration----------------------------------------------*/
static void ArcsCommon_qtCharMsgPro(void);
static void ArcsCommon_appDataPro(void);
static void ArcsCommon_cmdProcess(void);
/*$ */
static uint8_t l_ringBuf[ARCS_RING_BUF_SIZE];
static uint8_t l_recvBuf[ARCS_RECV_BUF_SIZE];
static uint8_t l_sendBuf[ARCS_SEND_BUF_SIZE];
/*$ story pointer */
static uint32_t l_arcsQueryBuf[ARCS_QR_QSIZE] = {
    0U
};
static TComQueue l_arcsQueue = {
    0U, 0U, 0U, ARCS_QR_QSIZE, &l_arcsQueryBuf[0]
};
/*$ Local varialable define-------------------------------------------------*/
static TArcsCommon l_arcsPro;
/*$ initial()...............................................................*/
void ArcsCommon_initial(inflight_plist head) {
    l_arcsPro.head = head;
    l_arcsPro.vptr = EP0_arcs;
    /* initial ring buffer */
    RingBuffer_initial(&l_arcsPro.ringBuf,
            ARCS_RING_BUF_SIZE, l_ringBuf);
    /*intial ring buffer processor */
    userTimerStop(&l_arcsPro.charRingPro.smTimer);
    userTimerStop(&l_arcsPro.charRingPro.itvTimer);
    l_arcsPro.charRingPro.recvOver = (bool)1;
    l_arcsPro.charRingPro.msgLen = 0;
    /* initial query id command queue */
    l_arcsPro.arcsQrQueue = &l_arcsQueue;
    l_arcsPro.qrFinishFlag = false;
    /* initial port */
    ExternPort_init(l_arcsPro.vptr);
}
/*$ send()..................................................................*/
int ArcsCommon_send(TProtocalQt *pMsg, bool isResp, int msgLen) {
    int sendLen;
    if ((pMsg == (TProtocalQt *)0)
        || (msgLen < PRO_COMMON_LEN))
    {
        return -1;
    }
    memset(l_sendBuf, 0, ARCS_SEND_BUF_SIZE);
    sendLen = ProtocalQt_Fill(pMsg, msgLen, l_sendBuf, ARCS_SEND_BUF_SIZE);
    if (sendLen > 0) {
        /* set system packet */
        system_tx(l_sendBuf,
                  sendLen,
                  RUNINFLIGHT,
                  TRANSMIT_TYPE_ARCS_CTRL,
                  isResp,
                  (uint8_t const *)0,
                  (void const *)0);
#if 0        
        int n;
        printf("Arcs sendLen = %d\n", sendLen);                           
        for (n = 0; n < sendLen; n++){
        printf("%02x ", l_sendBuf[n]);
        }
        printf("\n\n");
#endif        
    }
    /* -1 is error */
    return sendLen;
}
/*$ sendPacket()............................................................*/
int ArcsCommon_sendPacket(uint8_t *frame, uint16_t frameLen,
    bool isResp, uint32_t *invalTime)
{
/*\save no resp data to inflight list, only
     \called by tx_packet_event*/
    int sendLen;
    uint8_t ptCmd;
    uint16_t seq;
    inflight_plist pInflight;
    uint32_t timeout;
    uint8_t proType;
    if (!isResp) { /* not reponse data */
        ptCmd = ((TProtocalQt *)frame)->cmd;
        timeout = getArcsCmdTimeout(ptCmd);
        if (timeout == 0xffffffff) {
            if (NULL != gp_log_imp) {
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_ERROR,
                    "[timeout get Error %d]",
                    timeout);
            }
            /* error return */
            return -1;
        }        
        /**/
        proType = ptCmd = ((TProtocalQt *)frame)->head;
        if (PROTOCAL_QT_TYPE != proType) {
            if (NULL != gp_log_imp) {
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_ERROR,
                    "[proType get Error 0x%02x]",
                    proType);
            }
            return -1;
        }
        seq = ((TProtocalQt *)frame)->seq;
        pInflight = create_inflight_dblist_new_node(&pInflight);
	if ((inflight_plist)0 == pInflight) {
            if (NULL != gp_log_imp) {
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_ERROR,
                    "[inflight station node(ARCS) create failed!]");
            }
            /* error return */
            return -1;
	}
	memset(pInflight, 0, sizeof(inflight_list));
	pInflight->host_tx.inflight_frame.frame = allot_heap_space(frameLen,
            &pInflight->host_tx.inflight_frame.frame);
	if ((uint8_t *)0 != pInflight->host_tx.inflight_frame.frame) {
            memset(pInflight->host_tx.inflight_frame.frame, 0, frameLen);
            pInflight->host_tx.inflight_frame.inflight_frame_len = frameLen;
            memcpy(pInflight->host_tx.inflight_frame.frame, frame, frameLen);
            pInflight->host_tx.inflight_frame.seq_id = seq;
            pInflight->host_tx.inflight_frame.notification_flag = RUNINFLIGHT;
            /*protocal type 0xaa*/
            pInflight->host_tx.inflight_frame.data_type = proType;
            pInflight->host_tx.command_type = TRANSMIT_TYPE_ARCS_CTRL;
            pInflight->host_tx.flags.retried = 1;
            pInflight->host_tx.flags.resend = false;
            inflight_timer_start(timeout, pInflight);
            if (l_arcsPro.head != NULL) {
                insert_inflight_dblist_trail(l_arcsPro.head, pInflight);
                *invalTime = timeout;
            }
            else {
                free(pInflight->host_tx.inflight_frame.frame);
                pInflight->host_tx.inflight_frame.frame = (uint8_t *)0;
                free(pInflight);
                pInflight = (inflight_plist)0;
            }
	}
	else {
            free(pInflight);
            pInflight = (inflight_plist)0;
            return -1;
	}
    }
    /* send frame data */
    sendLen = ExternPort_send(l_arcsPro.vptr,
            frame, (int)frameLen);
    if (sendLen < 0) {
        if (NULL != gp_log_imp) {
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_ERROR,
                "send Arcs frame Err(Send Len = %d)!",
                sendLen);
        }
    }
    /* return send len */
    return sendLen;
}
/*$ recvPacketPro().........................................................*/
int ArcsCommon_recvPacketPro(void) {
    /*\recv packet and save to ring buffer */
    int reLen, pos;
    /* set zero */
    memset(l_recvBuf, 0, ARCS_RING_BUF_SIZE);
    reLen = ExternPort_recv(l_arcsPro.vptr,
        l_recvBuf, ARCS_RING_BUF_SIZE);
    DEBUG_INFO("reLen = %d, gregister_tmnl_pro.rgs_state = %d",
        reLen, gregister_tmnl_pro.rgs_state);
    if ((reLen > 0)
          && (gregister_tmnl_pro.rgs_state == RGST_IDLE))
    {/* save to ring buffer */
        pos = 0;
        while(pos < reLen) {
            RingBuffer_saveChar(&l_arcsPro.ringBuf,
                l_recvBuf[pos++]);
        }
    }
    /* return recv len */
    return reLen;
}
/*$ packetTimeouts()........................................................*/
void ArcsCommon_packetTimeouts(inflight_plist pIn) {
    bool retried;
    uint8_t ptCmd, *pFrame;
    uint16_t seq, frameLen;
    int sendLen;
    uint32_t timeout;
    if (pIn == (inflight_plist)0) {
        return;
    }
    /* set inflight information */
    pFrame = pIn->host_tx.inflight_frame.frame;
    if (pFrame == (uint8_t *)0) {
        return;
    }
    frameLen = pIn->host_tx.inflight_frame.inflight_frame_len;
    ptCmd = ((TProtocalQt *)pFrame)->cmd;
    seq = ((TProtocalQt *)pFrame)->seq;
    retried = is_inflight_cmds_retried(pIn);
    if (retried) {
        if ((ptCmd == QT_QUEUE_ID)
             && (l_arcsPro.qrFinishFlag))
        {/* set report query command finish */
            l_arcsPro.qrFinishFlag = false;
        }
        /* log error */
        if (NULL != gp_log_imp) {
            gp_log_imp->log.post_log_msg(&gp_log_imp->log, 
                LOGGING_LEVEL_ERROR,
                "[ARCS TIMEOUT(len = %d) cmd = %d, seq = %d]",
                frameLen, ptCmd, seq);
        }
        /* free inflight command node in the system */
        release_heap_space(&pIn->host_tx.inflight_frame.frame);
        delect_inflight_dblist_node(&pIn);
    }
    else {
        /*\ resend data */
        timeout = getArcsCmdTimeout(ptCmd);
        pIn->host_tx.flags.resend = true;
        pIn->host_tx.flags.retried++ ;
        inflight_timer_state_avail(timeout, pIn);
        /* send frame data */
        sendLen = ExternPort_send(l_arcsPro.vptr,
                pFrame, (int)frameLen);
        if (sendLen < 0) {
            if (NULL != gp_log_imp) {
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_ERROR,
                    "[TimeOut send Arcs frame Err(Send Len = %d)!]",
                    sendLen);
            }
        }
    }
}
/*$ process()...............................................................*/
void ArcsCommon_process(void) {
    ArcsCommon_qtCharMsgPro();
    ArcsCommon_cmdProcess();
}
/*$ Arcs process pthread()..................................................*/
void *ArcsCommon_pthreadPro(void) {
/* \this pthread builted for long time sem
    \operation, such as time consuming command,
    \like query command, it can't called same time with
    \ArcsCommon_process(), user only can call one between
    \ArcsCommon_process() function and ArcsCommon_pthreadPro
    \this pthread will wait when call ArcsCommon_send() until
    \reading fifo pthread sem post */
    ArcsCommon_qtCharMsgPro();
    ArcsCommon_cmdProcess();
    return (void *)0;
}
/*$ ArcsCommon_qrPro()......................................................*/
void ArcsCommon_qrPro(void) {
    TArcsQrBuf *workNode;
    uint32_t addr = 0;
    if (!l_arcsPro.qrFinishFlag) {
        if (QueueCom_popFiFo(l_arcsPro.arcsQrQueue, &addr)) {
            workNode = (TArcsQrBuf *)addr;
            if (workNode != (TArcsQrBuf *)0) {
                ArcsCommon_send((void*)workNode->buf,
                    false, (int)workNode->len);
                QueueCom_itemFree((void *)workNode);
                l_arcsPro.qrFinishFlag = true;
            }
        }
    }
}
/*$ cmdProcess()...............................................................*/
static void ArcsCommon_cmdProcess(void) {
/* \ poll cmd from all queue and handle it */
    ArcsCommon_qrPro();
}
/*$ qtCharMsgPro()..........................................................*/
static void ArcsCommon_qtCharMsgPro(void) {
    TProtocalQt *pMsgPro;
    TRingMsgPro *pRingPro;
    uint8_t ch; /* char store */
    uint16_t dataLen;
    pRingPro = &l_arcsPro.charRingPro;
    if (pRingPro->recvOver) {
        userTimerStart(3, &pRingPro->itvTimer);
        if (userTimerTimeout(&pRingPro->smTimer)) {
            /* process app data  here */
            DEBUG_INFO("appData cmd = %d",
                l_arcsPro.msgPro.cmd);
            ArcsCommon_appDataPro();
            /* stop sm timer */
            userTimerStop(&pRingPro->smTimer);
            pRingPro->msgLen = 0;
            pRingPro->recvOver = (bool)0;
        }
    }
    /* get ring char in buffer */
    pMsgPro = &l_arcsPro.msgPro;
    while (RingBuffer_getChar(&l_arcsPro.ringBuf, &ch)) {
        userTimerStart(3, &pRingPro->itvTimer);
        pRingPro->recvOver = (bool)0;
        if ((pRingPro->msgLen == 0)
              && (ch == PROTOCAL_QT_TYPE))
        {
            pMsgPro->head = PROTOCAL_QT_TYPE;
            pRingPro->msgLen = 1;
        }
        else if (pRingPro->msgLen == 1) {
            pMsgPro->type = ch;
            pRingPro->msgLen = 2;
        }
        else if (pRingPro->msgLen == 2) {
            /* '=' will clear  data stored last time, must be */
            pMsgPro->seq = (((uint16_t)ch) & 0x00ff);
            pRingPro->msgLen = 3;
        }
        else if (pRingPro->msgLen == 3) {
            /* '|=' must be */
            pMsgPro->seq |= ((((uint16_t)ch) << 8) & 0xff00);
            pRingPro->msgLen = 4;
        }
        else if (pRingPro->msgLen == 4) {
            pMsgPro->cmd = ch;
            pRingPro->msgLen = 5;
        }
        else if (pRingPro->msgLen == 5) {
            /* '=' will clear  data stored last time, must be */
            pMsgPro->dataLen = ((uint16_t)ch) & 0x00ff;
            pRingPro->msgLen = 6;
        }
        else if (pRingPro->msgLen == 6) {
            pMsgPro->dataLen |= (((uint16_t)ch) << 8) & 0xff00;
            if (pMsgPro->dataLen > PRO_QT_MAX) {
                pRingPro->msgLen = 0;
            }
            else {/* Not out of rang s*/
                pRingPro->msgLen = 7;
                dataLen = 0;
            }
        }
        else if (pRingPro->msgLen >= 7) {
            if (dataLen < pMsgPro->dataLen) {
                pMsgPro->dataBuf[dataLen] = ch;
                pRingPro->msgLen++;
                dataLen++;
            }
            else if (dataLen == pMsgPro->dataLen) {
                if (ProtocalQt_checkCrc(pMsgPro, ch) == 0) {
                    pRingPro->recvOver = (bool)1;
                    userTimerStart(2, &pRingPro->smTimer);
                }
                else {
                    pRingPro->msgLen = 0;
                }
            }
            else {
                pRingPro->msgLen = 0;
            }
        }
        else {
            /* never come this else */
        }
    }
    if (userTimerTimeout(&pRingPro->itvTimer)) {
        pRingPro->msgLen = 0;
    }
}
/*$ appDataPro()............................................................*/
static void ArcsCommon_appDataPro(void) {
/* must be called by ArcsCommon_qtCharMsgPro() */
    uint16_t seq;
    uint8_t subType, cmd;
    inflight_plist pI;
    TProtocalQt *pMsgPro = &l_arcsPro.msgPro;
    /* handled protocal cmd */
    subType = pMsgPro->head;
    seq = pMsgPro->seq;
    cmd = pMsgPro->cmd;
    if (pMsgPro->type & PRO_RESP_MASK) {
        if ((cmd == QT_QUEUE_ID)
             && (l_arcsPro.qrFinishFlag))
        {/* set report query command finish */
            l_arcsPro.qrFinishFlag = false;
        }
        
        pthread_mutex_lock(&ginflight_pro.mutex);
        pI = search_node_inflight_from_dblist(l_arcsPro.head, seq, subType);
        if (pI == (inflight_plist)0) {
            pthread_mutex_unlock(&ginflight_pro.mutex);
            return;
        }
        /* log message */
        if (NULL != gp_log_imp) {
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_DEBUG,
                "[Arcs Response seq = %d, cmd = %d]",
                seq, cmd);
        }
        /* release frame and pI space */
	release_heap_space(&pI->host_tx.inflight_frame.frame);
	delect_inflight_dblist_node(&pI);
        pthread_mutex_unlock(&ginflight_pro.mutex);
    }
    else {
        static uint16_t l_lastSeq;
        if (l_lastSeq == pMsgPro->seq) {
            /* reponse last seq */
            TProtocalQt reData;
            memset(&reData, 0, sizeof(TProtocalQt));
            reData.head = PROTOCAL_QT_TYPE;
            reData.type = PRO_RESP_MASK;
            reData.seq = pMsgPro->seq;
            reData.cmd = pMsgPro->cmd;
            reData.dataLen = 0;
            ArcsCommon_send(&reData, true, PRO_COMMON_LEN);
            return;
        }
        else {
            l_lastSeq = pMsgPro->seq;
        }
        switch (pMsgPro->cmd) {
            case QT_QUEUE_ID: {
                ArcsCmd_queryId(0, pMsgPro,
                    pMsgPro->dataLen + PRO_COMMON_LEN);
                break;
            }
            case QT_SWITCH_MATRIX: {
                ArcsCmd_switchMatrix(0, pMsgPro,
                    pMsgPro->dataLen + PRO_COMMON_LEN);
                break;
            }
            case QT_OPT_TMNL: {
                ArcsCmd_optTmnl(0, pMsgPro,
                    pMsgPro->dataLen + PRO_COMMON_LEN);
                break;
            }
            case QT_SYS_SET: {
                ArcsCmd_sysSet(0, pMsgPro,
                    pMsgPro->dataLen + PRO_COMMON_LEN);
                break;
            }
            case QT_CMR_CTL: {
                ArcsCmd_cmrCtl(0, pMsgPro,
                    pMsgPro->dataLen + PRO_COMMON_LEN);
                break;
            }
            default: {
                /* error cmd, do nothing */
                break;
            }
        }   
    }
}

void ArcsCommon_postQrQueue(void **node) {
    QueueCom_postFiFo(l_arcsPro.arcsQrQueue, node);
}

uint32_t ArcsCommon_postQrQueueCount(void) {
    uint32_t cout = QueueCom_count(l_arcsPro.arcsQrQueue);
    DEBUG_INFO("queue qr count = %d", cout);
    return cout;
}

