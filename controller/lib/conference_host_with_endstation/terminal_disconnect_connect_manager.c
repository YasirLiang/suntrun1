/*
* @file terminal_disconnect_connect_manager.c
* @brief manager terminal connect or disconnect
* @ingroup terminal
* @cond
******************************************************************************
* Build Date on  2017-3-12
* Last updated for version 1.0.0
* Last updated on  2017-3-14
*
*                    Moltanisk Liang
*                    ---------------------------
*                    avb auto control system
*
* Copyright (C) Moltanisk Liang, GuangZhou Suntron. All rights reserved.
******************************************************************************
* @endcond
*/
/*$ Including File----------------------------------------------------------*/
#include "global.h"
#include "terminal_pro.h"
#include "queue_com.h"
#include "terminal_disconnect_connect_manager.h"
#include "conference_transmit_unit.h"
#include "log_machine.h"
#include "terminal_system.h"

/*! define Queue Buffer Size------------------------------------------------*/
/*! Allow max speak or number in the queue */
#define REQ_QUEUE_BUF_SIZE (MAX_LIMIT_SPK_NUM + 1U)

/*$ Queue locker------------------------------------------------------------*/
static volatile int l_queueLockers[MANAGER_NUM][PRIOR_PUB_NUM] = {
    {0}
};

/*$ connector locker--------------------------------------------------------*/
static volatile int *l_connectorLocker = l_queueLockers[CONNECTOR];

/*$ disconnector locker-----------------------------------------------------*/
static volatile int *l_disconnectorLocker = l_queueLockers[DISCONNECTOR];

/*$ Queues' Buffer----------------------------------------------------------*/
static uint32_t l_qesBuf[MANAGER_NUM][PRIOR_PUB_NUM][REQ_QUEUE_BUF_SIZE];

/*$ request Queues----------------------------------------------------------*/
static TComQueue l_reqQueues[MANAGER_NUM][PRIOR_PUB_NUM] = {
    { /*! Chairman connect Request Queue */
        {0U, 0U, 0U, REQ_QUEUE_BUF_SIZE,
            l_qesBuf[CONNECTOR][CHAIRMAN_PRIOR]},

        /*! Vip connect Request Queue */
        {0U, 0U, 0U, REQ_QUEUE_BUF_SIZE,
            l_qesBuf[CONNECTOR][VIP_PRIOR]},

        /*! Common connect present Request Queue */
        {0U, 0U, 0U, REQ_QUEUE_BUF_SIZE,
            l_qesBuf[CONNECTOR][COMMON_PRIOR]}
    },

    {
     /*! Chairman connect Request Queue */
        {0U, 0U, 0U, REQ_QUEUE_BUF_SIZE,
            l_qesBuf[DISCONNECTOR][CHAIRMAN_PRIOR]},

        /*! Vip connect Request Queue */
        {0U, 0U, 0U, REQ_QUEUE_BUF_SIZE,
            l_qesBuf[DISCONNECTOR][VIP_PRIOR]},

        /*! Common connect present Request Queue */
        {0U, 0U, 0U, REQ_QUEUE_BUF_SIZE,
            l_qesBuf[DISCONNECTOR][COMMON_PRIOR]}
     }
};

/*$ connector queue---------------------------------------------------------*/
static TComQueue *l_connectorQe = l_reqQueues[CONNECTOR];

/*$ disconnector queue------------------------------------------------------*/
static TComQueue *l_disconnectorQe = l_reqQueues[DISCONNECTOR];

/*$ callback function including connect and disconnect----------------------*/
static TPRequestCallback l_callbacks[MANAGER_NUM][PRIOR_PUB_NUM] = {
    {(TPRequestCallback)0}
};

/*$ connector queue---------------------------------------------------------*/
static TPRequestCallback *l_connectorCbs = l_callbacks[CONNECTOR];

/*$ disconnector queue------------------------------------------------------*/
static TPRequestCallback *l_disconnectorCbs = l_callbacks[DISCONNECTOR];

/*$ acmp Queues' Buffer-----------------------------------------------------*/
static uint32_t l_acmpQueueBuf[REQ_QUEUE_BUF_SIZE];

/*$ acmp Queues----------------------------------------------------------*/
static TComQueue l_acmpQueue = {
    0U, 0U, 0U, REQ_QUEUE_BUF_SIZE, l_acmpQueueBuf
};

/*$ acmp queue locker-------------------------------------------------------*/
static volatile int l_acmpQueueLocker = 0;

/*$ Connector state machine state define -----------------------------------*/
/*! connector connect finish state machine */
#define CONNECT_FINISH          (0)

/*! connector connect wait state machine */
#define CONNECT_WAIT            (1) 

/*! connector connect success state machine */
#define CONNECT_SUCCESS         (2)

/*! connector connect failed state machine */
#define CONNECT_FAILED          (3)

/*! disconnector connect finish state machine */
#define DISCONNECT_FINISH       (0)

/*! disconnector connect wait state machine */
#define DISCONNECT_WAIT         (1)

/*! disconnector connect success state machine */
#define DISCONNECT_SUCCESS      (2)

/*! disconnector connect failed state machine */
#define DISCONNECT_FAILED       (3)

/*! Local function declaration----------------------------------------------*/
static void Terminal_eventGc(TQEvt **e);

static void Terminal_connectorCallback(uint8_t prior,
                bool connectFlag, tmnl_pdblist user, uint32_t permissions);

static void Terminal_disconnectorCallback(uint8_t prior,
                bool connectFlag, tmnl_pdblist user, uint32_t permissions);

static TQState Terminal_disconnectorDispatch(TQEvt * const e);

static TQState Terminal_connectorDispatch(TQEvt * const e);

/*$ Terminal_eventGc()......................................................*/
static void Terminal_eventGc(TQEvt **e) {
    /* release event */
    if (*e != (TQEvt *)0) {
        free(*e);
        *e = (TQEvt *)0;
    }
}

/*$ Terminal_connectorCallback..............................................*/
static void Terminal_connectorCallback(uint8_t prior,
                bool connectFlag, tmnl_pdblist user, uint32_t permissions)
{
    TPRequestCallback callBack;

    /* get callback function */
    callBack = l_connectorCbs[prior];
    if (callBack != (TPRequestCallback)0) {
        callBack(connectFlag, user, permissions);
    }
}

/*$ Terminal_disconnectorCallback...........................................*/
static void Terminal_disconnectorCallback(uint8_t prior,
                bool connectFlag, tmnl_pdblist user, uint32_t permissions)
{
    TPRequestCallback callBack;

    /* get callback function */
    callBack = l_disconnectorCbs[prior];
    if (callBack != (TPRequestCallback)0) {
        callBack(connectFlag, user, permissions);
    }
}

/*$ Terminal_connectorDispatch()............................................*/
static TQState Terminal_connectorDispatch(TQEvt * const e) {
    /*$ connector state machine */
    static uint8_t l_connectorState = CONNECT_FINISH;
    TQState status_;
    int ret = -1;
    uint64_t id;
    uint16_t tAddr;
    TMicEvent * micEvt;
    TAcmpEvt *acmpEvt;
    uint8_t failerTimes;
    uint32_t micEvtPermissions;

    switch (l_connectorState) {
        /* connect finish status */
        case CONNECT_FINISH: {
            switch (e->sig) {
                case MIC_OPEN_SIG: {
                    micEvt = (TMicEvent *)e;
                    /* assert pointer */
                    assert(micEvt != (TMicEvent *)0);

                    /* check permissions */
                    micEvtPermissions = (uint32_t)get_sys_state();
                    /* frequently ?*/
                    if (micEvtPermissions & micEvt->permissions) {
                        tAddr = micEvt->spkNode->tmnl_dev.address.addr;
                        id = micEvt->spkNode->tmnl_dev.entity_id;     

                        if (trans_model_unit_is_connected(id)) {
                            micEvt->failureTimes = 0;
                            /* this event  has being handled */
                            status_ = EVT_HANDLED;
                        }
                        else {                       
                            ret = trans_model_unit_connect(id,
                                    micEvt->spkNode);
                            if (ret == 0) {
                                /*decrement failure times
                                    connect failed increment */
                                --micEvt->failureTimes;
                                
                                /* connect success, wait connection response */
                                l_connectorState = CONNECT_WAIT;
                                status_ = EVT_UNHANDLED;

                                /* log message */
                                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                                        LOGGING_LEVEL_DEBUG,
                                        "[CONNECT_FINISH(connect times = %d)"
                                        " Open Mic(0x%llx-0x%x):"
                                        "Waiting for connection response]",
                                        MAX_FAILURE_TIMES -
                                            micEvt->failureTimes,
                                        id, tAddr);
                            }
                            else {
                                failerTimes = micEvt->failureTimes;
                                if (failerTimes > 0) {
                                    --micEvt->failureTimes;
                                    status_ = EVT_UNHANDLED;
                                }
                                else {
                                    /* current open signal finish */
                                    status_ = EVT_HANDLED;
                                }
                            }
                        }
                    }
                    else { /* no executable permissions */                        
                        /* make sure no connect again */
                        micEvt->failureTimes = 0;

                        /* this event  has being handled */
                        status_ = EVT_HANDLED;
                    }
                    
                    break;
                }
                default: {
                    status_ = EVT_SUPPER;
                    break;
                }
            }
            break;
        }

        /* connect wait status */
        case CONNECT_WAIT: {
            switch (e->sig) {
                case CONNECT_RX_RESPONSE_SIG: {
                    acmpEvt = (TAcmpEvt *)e;
                    assert(acmpEvt != (TAcmpEvt *)0);
                    
                    if (acmpEvt->status == 0) {
                        /* acmp response success,
                            change to successful status */
                        l_connectorState = CONNECT_SUCCESS;
                    }
                    else {
                        /* connect failed,
                            change to failed status */
                        l_connectorState = CONNECT_FAILED;
                    }
                    
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_DEBUG,
                        "[CONNECT_WAIT state recieve "
                        "CONNECT_RX_RESPONSE_SIG,Change to "
                        "state(%d)", l_connectorState);
                    
                    status_ = EVT_HANDLED;
                    break;
                }
                case CONNECT_TIMEOUT_SIG: {
                    acmpEvt = (TAcmpEvt *)e;
                    assert(acmpEvt != (TAcmpEvt *)0);
                    /* connect failed,
                        change to failed status */
                    l_connectorState = CONNECT_FAILED;
                    status_ = EVT_HANDLED;

                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_DEBUG,
                        "[CONNECT_WAIT state recieve "
                        "CONNECT_TIMEOUT_SIG,Change to "
                        "state(%d)", l_connectorState);
                    
                    break;
                }
                case MIC_OPEN_SIG: {
                    /* connect wait status
                        can't process open mic signal */
                    status_ = EVT_UNHANDLED;
                    break;
                }
                default: {
                    status_ = EVT_SUPPER;
                    break;
                }
            }
            
            break;
        }

        /* connect success status */
        case CONNECT_SUCCESS: {
            switch (e->sig) {
                case MIC_OPEN_SIG: {
                    /* current mic open
                        signal process successfully,
                        change to connect finish state */
                    l_connectorState = CONNECT_FINISH;
                    status_ = EVT_HANDLED;

                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_DEBUG,
                        "[CONNECT_SUCCESS state recieve "
                        "MIC_OPEN_SIG,Change to "
                        "state(%d)", l_connectorState);
                    
                    break;
                }
                default: {
                    /* handle signal by
                        supper state machine */
                    status_ = EVT_SUPPER;
                    break;
                }
            }  

            break;
        }

        /* connect failed status */
        case CONNECT_FAILED: {
            switch (e->sig) {
                case MIC_OPEN_SIG: { /* connect mic again */
                    micEvt = (TMicEvent *)e;
                    /* assert pointer */
                    assert(micEvt != (TMicEvent *)0);

                    /* check execute permisions */
                    micEvtPermissions = (uint32_t)get_sys_state();
                    if (micEvtPermissions & micEvt->permissions) {
                        tAddr = micEvt->spkNode->tmnl_dev.address.addr;
                        id = micEvt->spkNode->tmnl_dev.entity_id;
                        if (trans_model_unit_is_connected(id)) {
                            micEvt->failureTimes = 0;
                            /* this event  has being handled */
                            status_ = EVT_HANDLED;
                        }
                        else {        
                            ret = trans_model_unit_connect(id,
                                    micEvt->spkNode);
                            if (ret == 0) {
                                /*decrement failure times
                                    connect failed increment */
                                --micEvt->failureTimes;
                                
                                /* connect success, wait connection response */
                                l_connectorState = CONNECT_WAIT;
                                status_ = EVT_UNHANDLED;

                                /* log message */
                                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                                        LOGGING_LEVEL_DEBUG,
                                        "[CONNECT_FAILED(connect times = %d)"
                                        " Open Mic(0x%llx-0x%x):"
                                        "Waiting for connection response]",
                                        MAX_FAILURE_TIMES -
                                            micEvt->failureTimes,
                                        id, tAddr);
                            }
                            else {
                                failerTimes = micEvt->failureTimes;
                                if (failerTimes == 0) {
                                    /* current open signal finish
                                        change to state of connect finish */
                                    l_connectorState = CONNECT_FINISH;
                                    status_ = EVT_HANDLED;
                                }
                                else {
                                    /* decrement failure times */
                                    --micEvt->failureTimes;
                                    
                                    status_ = EVT_UNHANDLED;
                                }
                            }
                        }
                    }
                    else { /* no executable permissions */                        
                        /* make sure no connect again */
                        micEvt->failureTimes = 0;

                        /* this event  has being handled */
                        status_ = EVT_HANDLED;
                    }
                    
                    break;
                }
                default: {
                    /* handle signal by
                        supper state machine */
                    status_ = EVT_SUPPER;

                    break;
                }
            }

            break;
        }
        
        default: {
            /* handle signal by
                supper state machine */
            status_ = EVT_SUPPER;
            break;
        }
    }

    /* return value:
        true value is meaning process current queue finish,
        otherwise not process finish */
    return status_;
}

/*$ Terminal_disconnectorDispatch().........................................*/
static TQState Terminal_disconnectorDispatch(TQEvt * const e) {
/* disconnect state machine */
    static uint8_t l_disconnectorState = DISCONNECT_FINISH;
    TQState status_;
    int ret = -1;
    uint64_t id;
    uint16_t tAddr;
    TMicEvent * micEvt;
    TAcmpEvt *acmpEvt;
    uint8_t failerTimes;
    uint32_t micEvtPermissions;

    switch (l_disconnectorState) {
        /* disconnect finish status */
        case DISCONNECT_FINISH: {
            switch (e->sig) {
                case MIC_CLOSE_SIG: {
                    micEvt = (TMicEvent *)e;
                    /* assert pointer */
                    assert(micEvt != (TMicEvent *)0);

                    micEvtPermissions = (uint32_t)get_sys_state();
                    if (micEvtPermissions & micEvt->permissions) {
                        tAddr = micEvt->spkNode->tmnl_dev.address.addr;
                        id = micEvt->spkNode->tmnl_dev.entity_id;
                        ret = trans_model_unit_disconnect(id,
                                            micEvt->spkNode);
                        if (ret == 0) {
                            /*decrement failure times
                                connect failed increment */
                            --micEvt->failureTimes;
                            
                            /* disconnect success,
                                wait disconnection response */
                            l_disconnectorState = DISCONNECT_WAIT;
                            status_ = EVT_UNHANDLED;

                            /* log message */
                            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                                    LOGGING_LEVEL_DEBUG,
                                    "[DISCONNECT_FINISH(disconnect times = %d)"
                                    " Close Mic(0x%llx-0x%x):"
                                    "Waiting for disconnection response]",
                                    MAX_FAILURE_TIMES -micEvt->failureTimes,
                                    id, tAddr);
                        }
                        else {
                            failerTimes = micEvt->failureTimes;
                            if (failerTimes == 0) {
                                /* current open signal finish */
                                status_ = EVT_HANDLED;
                            }
                            else {
                                --micEvt->failureTimes;
                                status_ = EVT_UNHANDLED;
                            }
                        }
                    }
                    else { /* no executable permissions */
                        /* make sure no disconnect again */
                        micEvt->failureTimes = 0;

                        /* this event  has being handled */
                        status_ = EVT_HANDLED;
                    }
                    
                    break;
                }
                default: {
                    status_ = EVT_SUPPER;
                    break;
                }
            }
            
            break;
        }

        /* disconnect wait status */
        case DISCONNECT_WAIT: {
            switch (e->sig) {
                case DISCONNECT_RX_RESPONSE_SIG: {
                    acmpEvt = (TAcmpEvt *)e;
                    assert(acmpEvt != (TAcmpEvt *)0);
                    
                    if (acmpEvt->status == 0) {
                        /* acmp response success,
                            change to successful status */
                        l_disconnectorState = DISCONNECT_SUCCESS;
                    }
                    else {
                        /* disconnect failed,
                            change to failed status */
                        l_disconnectorState = DISCONNECT_FAILED;
                    }
                    
                    status_ = EVT_HANDLED;

                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_DEBUG,
                        "[DISCONNECT_WAIT state recieve "
                        "DISCONNECT_RX_RESPONSE_SIG, Change to "
                        "state(%d)", l_disconnectorState);
                    
                    break;
                }
                case DISCONNECT_TIMEOUT_SIG: {
                    acmpEvt = (TAcmpEvt *)e;
                    assert(acmpEvt != (TAcmpEvt *)0);
                    /* disconnect failed,
                        change to failed status */
                    l_disconnectorState = DISCONNECT_FAILED;
                    status_ = EVT_HANDLED;

                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_DEBUG,
                        "[DISCONNECT_WAIT state recieve "
                        "DISCONNECT_TIMEOUT_SIG, Change to "
                        "state(%d)", l_disconnectorState);
                    
                    break;
                }
                case MIC_CLOSE_SIG: {
                    /* disconnect wait status
                        can't process open mic signal */
                    status_ = EVT_UNHANDLED;
                    break;
                }
                default: {
                    status_ = EVT_SUPPER;
                    break;
                }
            }
            
            break;
        }

        /* disconnect success status */
        case DISCONNECT_SUCCESS: {
            switch (e->sig) {
                case MIC_CLOSE_SIG: {
                    /* current mic close
                        signal process successfully,
                        change to disconnect finish state */
                    l_disconnectorState = DISCONNECT_FINISH;
                    status_ = EVT_HANDLED;

                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_DEBUG,
                        "[DISCONNECT_WAIT state recieve "
                        "MIC_CLOSE_SIG, Change to "
                        "state(%d)", l_disconnectorState);

                    break;
                }
                default: {
                    /* handle signal by
                        supper state machine */
                    status_ = EVT_SUPPER;
                    break;
                }
            }  

            break;
        }

        /* disconnect failed status */
        case DISCONNECT_FAILED: {
            switch (e->sig) {
                case MIC_CLOSE_SIG: { /* disconnect mic again */
                    micEvt = (TMicEvent *)e;
                    /* assert pointer */
                    assert(micEvt != (TMicEvent *)0);

                    micEvtPermissions = (uint32_t)get_sys_state();
                    if (micEvtPermissions & micEvt->permissions) {
                        tAddr = micEvt->spkNode->tmnl_dev.address.addr;
                        id = micEvt->spkNode->tmnl_dev.entity_id;
                        ret = trans_model_unit_disconnect(id,
                                                micEvt->spkNode);
                        if (ret == 0) {
                            /*decrement failure times
                                disconnect failed increment */
                            --micEvt->failureTimes;
                            
                            /* disconnect success, wait connection response */
                            l_disconnectorState = DISCONNECT_WAIT;
                            status_ = EVT_UNHANDLED;

                            /* log message */
                            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                                    LOGGING_LEVEL_DEBUG,
                                    "[DISCONNECT_FAILED(disconnect times = %d)"
                                    " Close Mic(0x%llx-0x%x):"
                                    "Waiting for disconnection response]",
                                    MAX_FAILURE_TIMES -micEvt->failureTimes,
                                    id, tAddr);
                        }
                        else {
                            failerTimes = micEvt->failureTimes;
                            if (failerTimes == 0) {
                                /* current open signal finish
                                    change to state of disconnect finish */
                                l_disconnectorState = DISCONNECT_FINISH;
                                status_ = EVT_HANDLED;
                            }
                            else {
                                /* decrement failure times */
                                --micEvt->failureTimes;
                                
                                status_ = EVT_UNHANDLED;
                            }
                        }
                    }
                    else { /* no executable permissions */
                        /* make sure no disconnect again */
                        micEvt->failureTimes = 0;

                        /* this event  has being handled */
                        status_ = EVT_HANDLED;
                    }
                    
                    break;
                }
                default: {
                    /* handle signal by
                        supper state machine */
                    status_ = EVT_SUPPER;
                    break;
                }
            }

            break;
        }
        
        default: {
            /* handle signal by
                supper state machine */
            status_ = EVT_SUPPER;
            break;
        }
    }

    /* return value:
        true value is meaning process current queue finish,
        otherwise not process finish */
    return status_;
}

/*$ Terminal_requestConnect()...............................................*/
bool Terminal_requestConnect(tmnl_pdblist const spk, TEReqQePrior prior,
                            int failureTimes, uint32_t permissions)
{
    /*\ request connection by terminal */
    bool reqOk = (bool)0;
    TMicEvent *qElem;

    /*! check queue */
    if ((spk == (tmnl_pdblist)0)
          || (prior >= PRIOR_PUB_NUM))
    {
        return reqOk;
    }

    if (failureTimes > MAX_FAILURE_TIMES) {
        failureTimes = MAX_FAILURE_TIMES;
    }

    /* lock queue */
    INTERRUPT_LOCK(l_connectorLocker[prior]);

    qElem = (TMicEvent *)malloc(sizeof(TMicEvent));
    if (qElem != (TMicEvent *)0) {
        /* set queue element information */
        qElem->supper.sig = MIC_OPEN_SIG;
        qElem->spkNode = spk;
        qElem->failureTimes = failureTimes;
        qElem->permissions = permissions;

        /* post to queue */
        if (QueueCom_postFiFo(&l_connectorQe[prior], (void *)qElem)) {
            reqOk = (bool)1;
        }
        else {
            free(qElem);
            qElem = (TMicEvent *)0;
        }
    }

    /* unlock queue */
    INTERRUPT_UNLOCK(l_connectorLocker[prior]);
    
    return reqOk;
}

/*$ Terminal_requestDisConnect()............................................*/
bool Terminal_requestDisConnect(tmnl_pdblist const spk,
        TEReqQePrior prior, int failureTimes, uint32_t permissions)
{
    /*\ request connection by terminal */
    bool reqOk = (bool)0;
    TMicEvent *qElem;

    /*! check queue */
    if ((spk == (tmnl_pdblist)0)
          || (prior >= PRIOR_PUB_NUM))
    {
        return reqOk;
    }

    if (failureTimes > MAX_FAILURE_TIMES) {
        failureTimes = MAX_FAILURE_TIMES;
    }

    /* lock queue */
    INTERRUPT_LOCK(l_disconnectorLocker[prior]);

    qElem = (TMicEvent *)malloc(sizeof(TMicEvent));
    if (qElem != (TMicEvent *)0) {
        /* set queue element information */
        qElem->supper.sig = MIC_CLOSE_SIG;
        qElem->spkNode = spk;
        qElem->failureTimes = failureTimes;
        qElem->permissions = permissions;

        /* post to queue */
        if (QueueCom_postFiFo(&l_disconnectorQe[prior], (void *)qElem)) {
            reqOk = (bool)1;
        }
        else {
            free(qElem);
            qElem = (TMicEvent *)0;
        }
    }

    /* unlock queue */
    INTERRUPT_UNLOCK(l_disconnectorLocker[prior]);
    
    return reqOk;
}

/*$ Terminal_registerCallBack().............................................*/
bool Terminal_registerCallBack(TEReqQePrior prior,
     int owner, TPRequestCallback callBack)
{
    /* \ */
    if ((prior >= PRIOR_PUB_NUM)
        || (owner >= MANAGER_NUM))
    {
        return (bool)0;
    }

    /* \ add new callback */
    l_callbacks[owner][prior] = callBack;
    
    return (bool)1;
}

/*$ Terminal_delRegisterCallback()...................................*/
bool Terminal_delRegisterCallback(TEReqQePrior prior, int owner) {
    /* \ */
    if ((prior >= PRIOR_PUB_NUM)
        || (owner >= MANAGER_NUM))
    {
        return (bool)0;
    }

    /* \ delete callback function */
    l_callbacks[owner][prior] = (TPRequestCallback)0;
    
    return (bool)1;
}

/*$ Terminal_managerDisConnectTask()...............................................*/
int Terminal_micManagerTask(uint32_t sysTick) {
/*\ manager mic open and close task */
    int i = 0;
    TQState status_;                    /*! state machine */
    uint8_t remainCTimes;       /*! remain connection times */
    uint8_t remainDTimes;               /*! remain disconnection times */
    static uint32_t l_lastTick;         /*! last tick */
    uint64_t id;                        /*! 1722 id */
    uint16_t tAddr;                     /*! terminal application address */
    static uint8_t l_curConnectorQe; /*! */
    static uint8_t l_curDisConnectorQe; /*! */
    /* get acmp event */
    TQEvt *e = (TQEvt *)0;
    uint32_t qAddr;
    
    /* 100ms must be for 'A8' */
    if ((sysTick - l_lastTick) < 100U) {
        return 0;
    }

    INTERRUPT_LOCK(l_acmpQueueLocker);
 
    if (QueueCom_popFiFo(&l_acmpQueue, &qAddr)) {
        e = (TQEvt *)qAddr;
    }
    else if ((sysTick - l_lastTick) < 300U) { /* 300 ms */
        INTERRUPT_UNLOCK(l_acmpQueueLocker);
        return 0;
    }
    else {
        /* update current tick */
        l_lastTick = sysTick;
    }

    INTERRUPT_UNLOCK(l_acmpQueueLocker);

    /* Task procsess  acmp event */
    if (e != (TQEvt *)0) {
        Terminal_connectorDispatch(e);
        Terminal_disconnectorDispatch(e);
        /* release event */
        Terminal_eventGc(&e);
    }

    /* Task process events in the
        priority disconnector queues */
    for (i = l_curDisConnectorQe; i < PRIOR_PUB_NUM; i++) {
        
        /* lock the queue */
        INTERRUPT_LOCK(l_disconnectorLocker[i]);
        
        if (QueueCom_popFiFo(&l_disconnectorQe[i], &qAddr)) {
            e = (TQEvt *)qAddr;
            if (e != (TQEvt *)0) {
                status_ = Terminal_disconnectorDispatch(e);
                if ((status_ == EVT_HANDLED)
                    && (e->sig == MIC_CLOSE_SIG))
                {
                    /* unlock make sure 
                        that callback fucntion can 
                        request connections or diconnections */
                    INTERRUPT_UNLOCK(l_disconnectorLocker[i]);
                
                    /* get remain disconnections times */
                    remainDTimes = ((TMicEvent *)e)->failureTimes;
                    id = ((TMicEvent *)e)->spkNode->tmnl_dev.entity_id;
                    tAddr = ((TMicEvent *)e)->spkNode->tmnl_dev.address.addr;
                    if (remainDTimes > 0) {
                        /* disconnect successfully
                            because of disconnecting successfully in
                            'failureTimes' times, callback function */
                        Terminal_disconnectorCallback((uint8_t)i,
                                (bool)1, ((TMicEvent *)e)->spkNode,
                                ((TMicEvent *)e)->permissions);

                        /* log mic close success */
                        gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                                LOGGING_LEVEL_DEBUG,
                                "[ MIC CLOSE (disconnect times = %d"
                                " terminal(0x%016llx-0x%04x) SUCCESS ]",
                                MAX_FAILURE_TIMES - remainDTimes,
                                id, tAddr);
                    }
                    else {                        
                        /* disconnect failed 
                            because of disconnecting failed in
                            'failureTimes' times, callback function */
                        Terminal_disconnectorCallback((uint8_t)i,
                                (bool)0, ((TMicEvent *)e)->spkNode,
                                ((TMicEvent *)e)->permissions);

                        /* log mic close failed close */
                        gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                                LOGGING_LEVEL_ERROR,
                                "[ MIC CLOSE (disconnect times = %d)"
                                " terminal(0x%016llx-0x%04x) Failed ]"
                                "Waiting for connection response]",
                                MAX_FAILURE_TIMES -remainDTimes,
                                id, tAddr);
                    }

                    /* release event */
                    Terminal_eventGc(&e);

                    /* process highest priority queue for next time */
                    l_curDisConnectorQe = 0;

                    break;
                }
                else if ((status_ == EVT_UNHANDLED)
                              && (e->sig == MIC_CLOSE_SIG))
                { /* open mic signal process not finishing
                        post 'e' to current queue and return */
                    QueueCom_postLiFo(&l_disconnectorQe[i],
                        (void *)e);

                    /* must unlock the queue before 'break' */
                    INTERRUPT_UNLOCK(l_disconnectorLocker[i]);
                    
                    /* next loop from current queue */
                    l_curDisConnectorQe = i;
                    
                    break; /* break 'for' loop */
                }
                else {
                    /* release event */
                    Terminal_eventGc(&e);
                }
            }
        }

        /* unlock the queue */
        INTERRUPT_UNLOCK(l_disconnectorLocker[i]);
    }
    
    /* Task process events in the
        priority connector queues, process next open signal event
        only when last connection finish */
    for (i = l_curConnectorQe; i < PRIOR_PUB_NUM; i++) {        
        /* lock the queue */
        INTERRUPT_LOCK(l_connectorLocker[i]);
        
        if (QueueCom_popFiFo(&l_connectorQe[i], &qAddr)) {
            e = (TQEvt *)qAddr;
            if (e != (TQEvt *)0) {
                status_ = Terminal_connectorDispatch(e);
                if ((status_ == EVT_HANDLED)
                    && (e->sig == MIC_OPEN_SIG))
                {
                    /* unlock make sure 
                        that callback fucntion can 
                        request connections or diconnections */
                   INTERRUPT_UNLOCK(l_connectorLocker[i]);
                
                    /* get remain connections times */
                    remainCTimes = ((TMicEvent *)e)->failureTimes;
                    id = ((TMicEvent *)e)->spkNode->tmnl_dev.entity_id;
                    tAddr = ((TMicEvent *)e)->spkNode->tmnl_dev.address.addr;
                    if (remainCTimes > 0) {
                        /* connect successfully
                            because of connecting successfully in
                            'failureTimes' times, callback function */
                        Terminal_connectorCallback((uint8_t)i,
                                (bool)1, ((TMicEvent *)e)->spkNode,
                                ((TMicEvent *)e)->permissions);

                        /* log mic open success */
                        gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                                LOGGING_LEVEL_DEBUG,
                                "[ MIC Open (connect times = %d"
                                " terminal(0x%016llx-0x%04x) SUCCESS ]",
                                MAX_FAILURE_TIMES -remainCTimes,
                                id, tAddr);
                    }
                    else {
                        /* connect failed 
                            because of connecting failed in
                            'failureTimes' times, callback function */
                        Terminal_connectorCallback((uint8_t)i,
                                (bool)0, ((TMicEvent *)e)->spkNode,
                                ((TMicEvent *)e)->permissions);
                        
                        /* log mic open success */
                        gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                                LOGGING_LEVEL_DEBUG,
                                "[ MIC Open.1 (connect times = %d"
                                " terminal(0x%016llx-0x%04x) Failed ]",
                                MAX_FAILURE_TIMES -remainCTimes,
                                id, tAddr);
                    }

                    /* release handled event */
                    Terminal_eventGc(&e);

                    /* process highest priority queue for next time */
                    l_curConnectorQe = 0;

                    break;
                }
                else if ((status_ == EVT_UNHANDLED)
                             && (e->sig == MIC_OPEN_SIG))
                { /* open mic signal process not finishing
                        post 'e' to current queue and return */
                    QueueCom_postLiFo(&l_connectorQe[i],
                        (void *)e);

                    /* unlock the queue */
                    INTERRUPT_UNLOCK(l_connectorLocker[i]);
                    
                    /* next loop from current queue */
                    l_curConnectorQe = i;
                    
                    break; /* break 'for' loop */
                }
                else {
                    /* release event */
                    Terminal_eventGc(&e);
                }
            }
        }

        /* unlock the queue */
        INTERRUPT_UNLOCK(l_connectorLocker[i]);
    }

    /* return value */
    return 0;
}

/*$ Terminal_hasNoConnectTask().............................................*/
bool Terminal_hasTaskInQueue(uint8_t manager) {
    int i = 0;
    bool noEmpty;

    if (manager >= MANAGER_NUM) {
        return true;
    }

    noEmpty = false;
    for (i = 0; i < PRIOR_PUB_NUM;i++) {
        if (!QueueCom_isEmpty(&l_reqQueues[manager][i])) {
            noEmpty = true;
            break;
        }
    }

    /* return no empty */
    return noEmpty;
}

/*$ Terminal_postAcmpEvent()................................................*/
bool Terminal_postAcmpEvent(TQEvt const * const e) {
/* post event to acmp queue */
    bool bRet = (bool)0;

    if (e == (TQEvt const * const)0) {
        return bRet;
    }

    /* Lock */
    INTERRUPT_LOCK(l_acmpQueueLocker);
    
    if (QueueCom_postFiFo(&l_acmpQueue, (void *)e)) {
        bRet = (bool)1;
    }

    /* unlock */
    INTERRUPT_UNLOCK(l_acmpQueueLocker);

    return bRet;
}

