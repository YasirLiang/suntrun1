/*
* @file terminal_disconnect_connect_manager.h
* @brief manager terminal connect or disconnect
* @ingroup terminal
* @cond
******************************************************************************
* Build Date on  2017-3-12
* Last updated for version 1.0.0
* Last updated on  2017-3-13
*
*                    Moltanisk Liang
*                    ---------------------------
*                    avb auto control system
*
* Copyright (C) Moltanisk Liang, GuangZhou Suntron. All rights reserved.
******************************************************************************
* @endcond
*/

#ifndef __TERMINAL_DISCONNECT_CONNECT_MANAGER_H__
#define __TERMINAL_DISCONNECT_CONNECT_MANAGER_H__

/*! data type of signal */
typedef uint8_t TQSignal;

/*! data type of state machine state */
typedef uint8_t TQState;

/*! the return value after state machine handle define----------------------*/
/*!  Event handle */
#define EVT_HANDLED         (0)
/*!  Event unhandle */
#define EVT_UNHANDLED       (1)
/*!  Event post to supper state machine */
#define EVT_SUPPER          (2)
/*! ignore Event */
#define EVT_IGNORE          (3)

/*! supper event type define */
typedef struct {
    TQSignal sig;
}TQEvt;

/*! TMicEvent---------------------------------------------------------------*/
typedef struct {
    TQEvt supper;              /*! signal of mic event */
    tmnl_pdblist spkNode;      /*! terminal node */
    uint32_t permissions;      /*!Event execute permissions */
    uint8_t failureTimes;      /*! failure times */
}TMicEvent;

/*! TAcmpEvt----------------------------------------------------------------*/
typedef struct {
    TQEvt supper;              /*! signal of acmp event */
    uint8_t subMsg;            /*! subtype of acmp message */
    uint64_t talkerId;         /*! talker id */
    uint16_t talkerIndex;      /*! talker index */
    uint64_t listenerId;       /*! listener id */
    uint16_t listenerIndex;    /*! listener index */
    int32_t status;           /*! response status */
}TAcmpEvt;

/*! MIC signal--------------------------------------------------------------*/
#define MIC_CLOSE_SIG               (0)
#define MIC_OPEN_SIG                (1)

/*! acmp signal-------------------------------------------------------------*/
#define CONNECT_RX_RESPONSE_SIG     (2)
#define CONNECT_TIMEOUT_SIG         (3)
#define DISCONNECT_RX_RESPONSE_SIG  (4)
#define DISCONNECT_TIMEOUT_SIG      (5)

/*! managers */
enum {
    CONNECTOR,
    DISCONNECTOR,
    MANAGER_NUM
};

/*! pointer request to callback function------------------------------------*/
typedef int (*TPRequestCallback)(bool, tmnl_pdblist, uint32_t);

/*$ Queue Manager priority--------------------------------------------------*/
typedef enum TEReqQePrior {
   CHAIRMAN_PRIOR = 0, /*! chairman prior */
   VIP_PRIOR,          /*! vip prior */
   COMMON_PRIOR,       /*! common prior */
   PRIOR_PUB_NUM       /*! priority number */
}TEReqQePrior;

/*$ MAX failure times(Include connect and disconnect) */
#define MAX_FAILURE_TIMES (5)

/*$ Extern function declaration---------------------------------------------*/
/*$ Terminal_requestConnect()...............................................*/
extern bool Terminal_requestConnect(tmnl_pdblist const spk,
        TEReqQePrior prior, int failureTimes, uint32_t permissions);

/*$ Terminal_requestDisConnect()............................................*/
extern bool Terminal_requestDisConnect(tmnl_pdblist const spk,
        TEReqQePrior prior, int failureTimes, uint32_t permissions);

/*$ Terminal_registerCallBack().............................................*/
extern bool Terminal_registerCallBack(TEReqQePrior prior,
         int owner, TPRequestCallback callBack);

/*$ Terminal_delRegisterCallback()...................................*/
extern bool Terminal_delRegisterCallback(TEReqQePrior prior, int owner);

/*$ Terminal_managerDisConnectTask()...............................................*/
extern int Terminal_micManagerTask(uint32_t sysTick);

/*$ Terminal_hasTaskInQueue()...............................................*/
extern bool Terminal_hasTaskInQueue(uint8_t manager);

/*$ Terminal_hasTask()......................................................*/
extern bool Terminal_hasTask(TEReqQePrior prior, uint16_t user);

/*$ Terminal_cancelTask()...................................................*/
extern bool Terminal_cancelTask(uint8_t manager,
        TEReqQePrior prior, uint16_t user);

/*$ Terminal_postAcmpEvent()................................................*/
extern bool Terminal_postAcmpEvent(TQEvt const * const e);

#endif /* __TERMINAL_DISCONNECT_CONNECT_MANAGER_H__ */

