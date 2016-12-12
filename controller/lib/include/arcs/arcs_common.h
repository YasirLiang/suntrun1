/*
* @file arcs_common.h
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
#ifndef __ARCS_COMMON_H__
#define __ARCS_COMMON_H__
/*$ */
#include "inflight.h"
#include "protocal_qt.h"
/*$ */
/*$ query id cmd buf of queue element type----------------------------------*/
typedef struct TArcsQrBuf {
    uint8_t len;
    uint8_t buf[PRO_QT_MAX];
}TArcsQrBuf;
/*$ */
extern void ArcsCommon_initial(inflight_plist head);
/*$ */
extern int ArcsCommon_send(TProtocalQt *pMsg, bool isResp, int msgLen);
/*$ */
extern int ArcsCommon_sendPacket(uint8_t *frame, uint16_t frameLen,
    bool isResp, uint32_t *invalTime);
/*$ */
extern int ArcsCommon_recvPacketPro(void);
/*$ */
extern void ArcsCommon_packetTimeouts(inflight_plist pIn);
/*$ */
extern void ArcsCommon_process(void);
/*$ */
extern void ArcsCommon_postQrQueue(void **node);

#endif /* __ARCS_COMMON_H__ */

