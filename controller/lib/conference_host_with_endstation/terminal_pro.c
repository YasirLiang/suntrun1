/*
* @file terminal_pro.c
* @brief meeting proccess
* @ingroup Terminal
* @cond
******************************************************************************
* Last updated for version 1.0.0
* Last updated on  2016-03-14
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
#include "terminal_pro.h"
#include "linked_list_unit.h"
#include "host_controller_debug.h"
#include "conference_host_to_end.h"
#include "system_packet_tx.h"
#include "inflight.h"
#include "aecp_controller_machine.h"
#include "file_util.h"
#include "conference_end_to_host.h"
#include "profile_system.h"
#include "terminal_command.h"
#include "terminal_system.h"
#include "upper_computer_pro.h"
#include "camera_pro.h"
#include "time_handle.h"
#include "log_machine.h" /* system log include file*/
#include "conference_transmit_unit.h"
#include "central_control_recieve_unit.h" /* CHANNEL_MUX_NUM is defined */
#include "queue_com.h"

#ifdef MIC_PRIOR_MANEGER_ENABLE
#include "terminal_disconnect_connect_manager.h"
#endif /* MIC_PRIOR_MANEGER_ENABLE */

/* terminal mic time out set macro------------------------------------------*/
#define MIC_SET_TIME_OUT 0

/* terminal mic num macro---------------------------------------------------*/
#define MIC_ARRAY_NUM             (20)

/*! TSpkQueueElem-----------------------------------------------------------*/
typedef struct {
    tmnl_pdblist spkNode;
    uint32_t permissions; /* excutable permission */
    uint8_t failureTimes; /*! connect failure times */
}TSpkQueueElem;

/*$ MAX connect failure times */
#define MAX_CONNECT_FAILURE_TIMES (5)

/* terminal mic structure --------------------------------------------------*/
/*${terminal::Terminal_mic} ................................................*/
typedef struct terminal_micLater {
    tmnl_pdblist node;  /*! terminal unit node pointer */
    uint32_t timeTick;                    /*! time out */
    uint8_t setCount;                /*! count setting */
    uint8_t micState; /*!mic status to set enpointment */
}Terminal_mic;

/* Terminal micQueue structure ---------------------------------------------*/
/*${Terminal::micQueue } ...................................................*/
typedef struct terminal_micQueue {
    uint8_t head; /*head of queue*/
    uint8_t trail; /*trail of queue*/
    uint8_t size;   /*size of queue*/
    Terminal_mic * const pBuf; /*const point to buf of queue */
}Terminal_micQueue;

/* Local objects -----------------------------------------------------------*/
static Terminal_mic l_proMics[MIC_ARRAY_NUM] = {
    {NULL, 0U, 0U}
};

/* Local objects -----------------------------------------------------------*/
static Terminal_micQueue l_micQueue = {
    0U, 0U, MIC_ARRAY_NUM, &l_proMics[0]
};

/*$ {@ First apply speaking QUEUE-------------------------------------------*/

#ifdef FIRST_SPEAK_QUEUE_ENABLE

/*$ function declaration----------------------------------------------------*/
bool Terminal_firstSpkRequestConnect(tmnl_pdblist const spk,
    TComQueue * const pRestQueue, int * const locker,
    int failureTimes, uint32_t permissions);

bool Terminal_changeFirstSpkTaskPer(uint16_t addr,
    TComQueue * const pRestQueue, int * const locker, uint32_t permissions);

/*$ APPLY MODE callback setting permissions----------------------------------*/
#define APPLY_CB_PERMISSION (0x00000001 << 8)

#define COMMON_SPK_PERMISSION \
    (SIGN_STATE | DISCUSS_STATE | VOTE_STATE | GRADE_STATE | ELECT_STATE)

/*$ define queue number of speaking */
#define FS_SPEAK_QUEUE_NUM  (3)

/*$ first speak Queue size */
#define FS_QUEUE_SIZE       (MAX_LIMIT_APPLY_NUM * 5)

/*$ Local first speak queue Buf---------------------------------------------*/
static uint32_t l_fSpkQueueBuf[FS_QUEUE_SIZE] = {
    0U
};

/*$ Local first speak Queue-------------------------------------------------*/
static TComQueue l_FSpkQueue = {
    0U, 0U, 0U, FS_QUEUE_SIZE, l_fSpkQueueBuf
};

/*$ Local first speak queue Buf(chairman)-----------------------------------*/
static uint32_t l_chairmanFSpkQueueBuf[MAX_LIMIT_APPLY_NUM] = {
    0U
};

/*$ Local first speak Queue(chairman)---------------------------------------*/
static TComQueue l_chairmanFSpkQueue = {
    0U, 0U, 0U, MAX_LIMIT_APPLY_NUM, l_chairmanFSpkQueueBuf
};

/*$ Local first speak queue Buf(vip)----------------------------------------*/
static uint32_t l_vipFSpkQueueBuf[FS_QUEUE_SIZE/2] = {
    0U
};

/*$ Local first speak Queue(vip)--------------------------------------------*/
static TComQueue l_vipFSpkQueue = {
    0U, 0U, 0U, FS_QUEUE_SIZE/2, l_vipFSpkQueueBuf
};

/*$ Queue Locker------------------------------------------------------------*/
static int l_FSpkLocker = 0;

/*$ First speaking QUEUE @}-------------------------------------------------*/

#endif /* FIRST_SPEAK_QUEUE_ENABLE */

#ifdef MIC_PRIOR_MANEGER_ENABLE

/*$ disable key oporation timeout */
#define DISABLE_KEY_OPT_TIMEOUT

/*$ callback fuction locker-------------------------------------------------------*/
static volatile int l_callBackLocker = 0;

/* function declarations----------------------------------------------------*/
int Terminal_chairmanOpenMicCallback(bool isSuccess,
    tmnl_pdblist user, uint32_t permissions);

int Terminal_vipOpenMicCallback(bool isSuccess,
    tmnl_pdblist user, uint32_t permissions);

int Terminal_commonOpenMicCallback(bool isSuccess,
    tmnl_pdblist user, uint32_t permissions);

static void Terminal_limitModeApplyTSpeak(void);

int Terminal_chairmanCloseMicCallback(bool isSuccess,
    tmnl_pdblist user, uint32_t permissions);

int Terminal_vipCloseMicCallback(bool isSuccess,
    tmnl_pdblist user, uint32_t permissions);

int Terminal_commonCloseMicCallback(bool isSuccess,
    tmnl_pdblist user, uint32_t permissions);

bool Terminal_chairmanInSpeakSuccess(uint16_t speakNum,
    uint32_t permissions);

bool Terminal_getDisconnectAddress(uint16_t * const addr, TEReqQePrior prior,
    uint16_t const * const speakList, uint16_t const speakListLen);

bool terminal_firstSpeakQueueHasTask(TComQueue * const spkQueue,
    uint16_t const addr);

void terminal_chairmanCancelAllTask(void);

void terminal_common_speak(tmnl_pdblist dis_node, bool key_down);

void terminal_vip_speak(tmnl_pdblist dis_node, bool key_down);

void terminal_chairman_speak(tmnl_pdblist dis_node, bool key_down);

#endif /* MIC_PRIOR_MANEGER_ENABLE */

#ifdef __DEBUG__
/*Macro define Terminal debug-----------------------------------------------*/
#define __TERMINAL_PRO_DEBUG__
#endif
#ifdef __TERMINAL_PRO_DEBUG__
/*Macro define terminal_pro_debug-------------------------------------------*/
#define terminal_pro_debug(fmt, args...) \
	fprintf(stdout,"\033[32m %s-%s-%d:\033[0m "fmt" \r\n",\
	                 __FILE__, __func__, __LINE__, ##args);
#else
#define terminal_pro_debug(fmt, args...)
#endif

/*Global varialable define--------------------------------------------------*/
/*the file decriptor of saving terminal address.............................*/
FILE* addr_file_fd = NULL;
/*Table of alloting terminal address........................................*/
terminal_address_list tmnl_addr_list[SYSTEM_TMNL_MAX_NUM];
terminal_address_list_pro allot_addr_pro;
/*the guard node of terminal double list....................................*/
tmnl_pdblist dev_terminal_list_guard = NULL;
tmnl_pdblist gcur_tmnl_list_node = NULL;
tmnl_state_set gtmnl_state_opt[TMNL_TYPE_NUM];
/*the flag of system discuss proccessing....................................*/
tsys_discuss_pro gdisc_flags;
/*chairman interrupt........................................................*/
tchairman_control_in gchm_int_ctl;
/*terminal register proccessing.............................................*/
volatile ttmnl_register_proccess gregister_tmnl_pro;
/*speaking time, zero means no limit........................................*/
volatile uint8_t speak_limit_time = 0; 						
/*lcd num display...........................................................*/
volatile uint8_t glcd_num = 0;
/*terminal led lamp.........................................................*/
volatile uint8_t gled_buf[2] = {0};
/*sign status...............................................................*/
volatile enum_signstate gtmnl_signstate;
/*time of retroactive.......................................................*/
volatile uint8_t gsign_latetime;
/*the flags of sign.........................................................*/
volatile bool gsigned_flag = false;
/*voting proccessing........................................................*/
volatile evote_state_pro gvote_flag = NO_VOTE;
/*the global index of voting................................................*/
volatile uint16_t gvote_index;
/*true means first key being value..........................................*/
volatile bool gfirst_key_flag;
/*the stype of vote mode....................................................*/
volatile tevote_type gvote_mode;							
type_spktrack gspeaker_track;
/*proccess query the voting result..........................................*/
tquery_svote  gquery_svote_pro;
/* the flag of finishing single terminal register...........................*/
extern volatile bool gvregister_recved;
/*$ init_terminal_proccess_fd...............................................*/
void init_terminal_proccess_fd(FILE ** fd) {
    *fd = Fopen(ADDRESS_FILE, "rb+");
    if (NULL == *fd) {
        terminal_pro_debug("open %s Err: not exit!"
                       "Will create for the address file for the first time!",
                            ADDRESS_FILE);
        terminal_open_addr_file_wt_wb();
    }
}
/*$ initial termianl address list...........................................*/
int init_terminal_address_list_from_file(void) {
    int i; /* loop varialable */
    int ret;  /* return value */
    /* initial terminal address list */ 
    memset(tmnl_addr_list, 0, sizeof(tmnl_addr_list));
    for(i = 0; i < SYSTEM_TMNL_MAX_NUM; i++) {
        tmnl_addr_list[i].addr = INIT_ADDRESS;
        tmnl_addr_list[i].tmn_type = TMNL_TYPE_COMMON_RPRST;
    }
    /* read terminal address information from address file
        if read failed, the system need to reallot agian */
    ret = terminal_address_list_read_file(addr_file_fd, tmnl_addr_list);
    if (ret == -1) {
        terminal_pro_debug("init tmnl_addr_list from address file"
                                      "need to reallot terminal address\n\t\t"
                              "Please send reAllot command by command line!");
    }
    
    return ret;
}
/*$ init_terminal_address_list..............................................*/
void init_terminal_address_list(void) {
    int i;
    /* initial terminal address list */ 
    memset(tmnl_addr_list, 0, sizeof(tmnl_addr_list));
    for (i = 0; i < SYSTEM_TMNL_MAX_NUM; i++) {
        tmnl_addr_list[i].addr = INIT_ADDRESS;
        tmnl_addr_list[i].tmn_type = TMNL_TYPE_COMMON_RPRST;
    }
}
/*$ Inline function init_terminal_allot_address.............................*/
inline void init_terminal_allot_address(void) {
    allot_addr_pro.addr_start = 0;
    allot_addr_pro.index = 0;
    allot_addr_pro.renew_flag= 0;
}
/*$ Inline function init_terminal_device_double_list........................*/
inline void init_terminal_device_double_list(void) {
    /* init terminal system double list */
    init_terminal_dblist(&dev_terminal_list_guard);
    assert(dev_terminal_list_guard != NULL);
    gcur_tmnl_list_node = dev_terminal_list_guard;
}
/*$ init_terminal_discuss_param.............................................*/
int init_terminal_discuss_param(void) {
    thost_system_set set_sys;/* the format structure of system profile file */
    memcpy(&set_sys, &gset_sys, sizeof(thost_system_set));   /* zero buffer */
    gdisc_flags.apply_limit = set_sys.apply_limit;    /* apply limit number */
    gdisc_flags.limit_num = set_sys.speak_limit;     /* speak limit number */
    gdisc_flags.currect_first_index = set_sys.apply_limit;/* priority index */
    gdisc_flags.apply_num = 0;                              /* apply number */
    gdisc_flags.speak_limit_num = 0;                  /* speak limit number */
    gdisc_flags.edis_mode = (ttmnl_discuss_mode)set_sys.discuss_mode;
    gdisc_flags.vipSpkNum = 0;
    memset(gdisc_flags.speak_addr_list, 0xffff, MAX_LIMIT_SPK_NUM);
    memset(gdisc_flags.vipSpeakList, 0xffff, MAX_LIMIT_SPK_NUM);
    memset(gdisc_flags.apply_addr_list, 0xffff, MAX_LIMIT_APPLY_NUM);
    return 0; /* default return value */
}
/*$ terminal_speak_track_pro_init...........................................*/
void terminal_speak_track_pro_init(void) {/* initial speak list */
    int i;
    gspeaker_track.spk_num = 0;
    for (i = 0; i < MAX_SPK_NUM; i++) {
        gspeaker_track.spk_addrlist[i] = 0xffff;
    }
}

void print_out_terminal_addr_infomation(terminal_address_list* p, int num) {
    int i; /* loop varialable */
    
#ifdef __TERMINAL_PRO_DEBUG__
    printf("Addr Info:\n");/* head information */
    for (i = 0; i < num; i++) {
        printf("[ (addr-type)-> (%d -%d) ]\n", p[i].addr, p[i].tmn_type);
    }
#endif
    if (gp_log_imp == NULL) {
        return;
    }
    
    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
            LOGGING_LEVEL_DEBUG,
            "\n\n[**************Addr Info****************]");
   
    for (i = 0; i < num; i++) {
        gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_DEBUG,
                "[ (addr-type)-> (%d - %d) ]",
                p[i].addr, p[i].tmn_type);
    }
}
/*$ init_terminal_proccess_system...........................................*/
void init_terminal_proccess_system(void) {/*initial terminal proccess sytem*/
    int tmnl_count;/* terminal count */
    int i;    /* loop varialable */
    tmnl_pdblist p; /* pointer to new terminal node */
    
    /* terminal register initial */
    terminal_register_init();
    
    /* initial terminal double list accroding to address file */
    init_terminal_device_double_list();
    
    /* initial terminal allot proccessing */
    init_terminal_allot_address();
    
    /* initial terminal discuss param */
    init_terminal_discuss_param();
    
    /* initial speak track proccessing */
    terminal_speak_track_pro_init();
    
    /* initial query the result of voting proccessing */
    terminal_query_proccess_init();
    
#ifdef MIC_PRIOR_MANEGER_ENABLE
    /* register mic open task callback functions */
    Terminal_registerCallBack(CHAIRMAN_PRIOR,
        CONNECTOR, Terminal_chairmanOpenMicCallback);
    Terminal_registerCallBack(VIP_PRIOR,
        CONNECTOR, Terminal_vipOpenMicCallback);
    Terminal_registerCallBack(COMMON_PRIOR,
        CONNECTOR, Terminal_commonOpenMicCallback);

    /* register mic close task callback functions */
    Terminal_registerCallBack(CHAIRMAN_PRIOR,
        DISCONNECTOR, Terminal_chairmanCloseMicCallback);
    Terminal_registerCallBack(VIP_PRIOR,
        DISCONNECTOR, Terminal_vipCloseMicCallback);
    Terminal_registerCallBack(COMMON_PRIOR,
        DISCONNECTOR, Terminal_commonCloseMicCallback);
#endif /* MIC_PRIOR_MANEGER_ENABLE */

    /* initial terminal proccess fd */
    init_terminal_proccess_fd(&addr_file_fd);
    if (NULL == addr_file_fd) {/* Error initial file fd? */
        return;
    }
    
    /* termanal counts in the system */
    tmnl_count = init_terminal_address_list_from_file();
    if (tmnl_count != -1) {/* initial success ? */
        /* Debug terminal counts */
        if (gp_log_imp != NULL) {
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_DEBUG,
                "[Initail terminal address success, number = %d",
                tmnl_count);
        }

        /* Debug terminal address information */
        print_out_terminal_addr_infomation(tmnl_addr_list, tmnl_count);
        
        /* set the total number of terminal */
        gregister_tmnl_pro.tmn_total = tmnl_count;
        
        /* create node of terminal and initial */
        for (i = 0; i < tmnl_count; i++) {
            /* create terminal double list node */
            p = create_terminal_dblist_node(&p);
            if (p != NULL) {
                /* initial terminal node number */
                init_terminal_dblist_node_info(p);
                p->tmnl_dev.entity_id = 0;
                p->tmnl_dev.address.addr = tmnl_addr_list[i].addr;
                p->tmnl_dev.address.tmn_type = tmnl_addr_list[i].tmn_type;
                /* save node to link list */
                insert_terminal_dblist_trail(dev_terminal_list_guard, p);
                
                terminal_pro_debug("create new "
                        "tmnl list node[0x%04x] Success",
                               p->tmnl_dev.address.addr);
            }
        }
        
        /* sort terminal node by address */
        if (-1 == sort_terminal_dblist_node(dev_terminal_list_guard)) {
            terminal_pro_debug("insert register node is Err!");
        }
    }
}

/*$ terminal_proccess_system_close..........................................*/
void terminal_proccess_system_close(void) {         /* release address file */
    if (addr_file_fd != NULL) {
        Fclose(addr_file_fd); /* close fd */ 
        if (addr_file_fd != NULL) {
            addr_file_fd = NULL;/* make sure no be using agian */
        }
    }
}
/*$ terminal_register_pro_address_list_save.................................*/
bool terminal_register_pro_address_list_save(uint16_t addr_save,
                                                        bool is_register_save)
{
    volatile ttmnl_register_proccess *p_regist_pro = &gregister_tmnl_pro;
    volatile uint16_t *pRelist;               /*pointer to register list */
    volatile uint16_t *pUnTrail; /* pointer to noregister trail */
    if ((p_regist_pro != NULL)
          && (addr_save != 0xffff)) 
    {
        pRelist = p_regist_pro->register_pro_addr_list;
        pUnTrail = &p_regist_pro->noregister_trail;
        
        if (!is_register_save) { /* no save to register list */
            if (*pUnTrail < p_regist_pro->list_size) {
                if ((pRelist[*pUnTrail] == 0xffff)
                      &&(*pUnTrail == p_regist_pro->noregister_head))
                {
                    /* no unregister address in the current list */
                    pRelist[*pUnTrail] = addr_save;
                }
                else if (pRelist[*pUnTrail] != 0xffff)
                {
                    pRelist[++(*pUnTrail)] = addr_save;
                }
                else
                {
                    /* no other case */
                }
                
                if (*pUnTrail >= p_regist_pro->list_size) {
                    p_regist_pro->unregister_list_full = true;
                }
                return true;
            }
        }
        else {
            /*step1: insert the element of unregister list head to unregister
                list trail 
                step2: increment the head of unregister list 
                step3: insert address to register list trail */
            uint16_t a;/* temp address saving */
            uint16_t head; /* head of no register list */
            uint16_t trail; /* trail of register list */
            head = p_regist_pro->noregister_head;
            a = pRelist[head]; /* head element of register list */
            if (terminal_register_pro_address_list_save(a, false)) {
                p_regist_pro->noregister_head++;
                trail = ++p_regist_pro->rgsted_trail;
                pRelist[trail] = addr_save;
            }
        }
    }
    else {
        /* nothing to do */
    }
    return false; /* default return value */
}
/*$ terminal_delect_unregister_addr.................................*/
bool terminal_delect_unregister_addr(uint16_t register_addr_delect)
{/* delect registed address from unregister list and save it to register
    list, and register_addr_delect must be address of being registed */
    volatile ttmnl_register_proccess *p_regist_pro = &gregister_tmnl_pro;
    volatile uint16_t *pRelist;               /*pointer to register list */
    volatile uint16_t *pUnTrail; /* pointer to noregister trail */
    int i = 0, delect_index;
    bool found_dl = false;
    volatile uint16_t *p_head; /* pointer to head of no register head */

    if ((p_regist_pro != NULL)
          && (register_addr_delect != 0xffff))
    {
        p_head = &p_regist_pro->noregister_head;
        pUnTrail = &p_regist_pro->noregister_trail;
        pRelist = p_regist_pro->register_pro_addr_list;
        if ((*p_head > *pUnTrail)
              || (*p_head > (SYSTEM_TMNL_MAX_NUM-1))
              || (*pUnTrail > (SYSTEM_TMNL_MAX_NUM-1))
              || ((*p_head !=  (p_regist_pro->rgsted_trail + 1))
                      &&(*p_head != 0)))
        {
            terminal_pro_debug("Err delect unregister address %d"
            "(head_index)----%d(trail)---%d(rgsted_trail)",
            *p_head, *pUnTrail, p_regist_pro->rgsted_trail);
            return false;
        }
        /*look for delect address */
        for (i = *p_head; i <= *pUnTrail; i++) {
            if (pRelist[i] == register_addr_delect) {
                delect_index = i;
                found_dl = true;
                break;
            }
        }

        if (found_dl) {
            terminal_pro_debug("save register addr = %04x ?="
                                     "( (delect index = %d)list addr = %04x)-"
                                   "(swap addr = %04x)<<====>> %d(head_index)"
                                           "----%d(trail)---%d(rgsted_trail)",
                                           register_addr_delect, delect_index,
                           pRelist[delect_index], pRelist[*p_head],
                               *p_head, *pUnTrail, p_regist_pro->rgsted_trail);
            if (*p_head > *pUnTrail) {
                return false;
            }
            else {/* swap with the head of unregister list */
                if (swap_valtile_uint16(&pRelist[*p_head],
                                                      &pRelist[delect_index]))
                {
                    /* step1:move the tail of register list to the head of
                         unregister
                         step2: move the head of unregister to the next
                         unregister elememt of unregister list head
                        */
                    p_regist_pro->rgsted_trail = *p_head;
                    if (*p_head == *pUnTrail) {
                        *pUnTrail = ++(*p_head);
                    }
                    else {
                        ++(*p_head);
                    }
                    return true;
                }
            }
        }
    }
    return false; /* default return value */
}
/*$ terminal_clear_from_unregister_addr_list................................*/
bool terminal_clear_from_unregister_addr_list(uint16_t addr) {
    volatile ttmnl_register_proccess *p_regist_pro = &gregister_tmnl_pro;
    volatile uint16_t *pRelist;              /*pointer to register list */
    volatile uint16_t *p_head;/* pointer to the head of unregister list */
    volatile uint16_t *p_trail;/* pointer to the trail of register list */
    if ((p_regist_pro != NULL)
          && (addr != 0xffff))
    {
        int i = 0, delect_index;
        bool found_dl = false;
        p_head = &p_regist_pro->noregister_head;
        p_trail = &p_regist_pro->noregister_trail;
        pRelist = p_regist_pro->register_pro_addr_list;
        if ((*p_head > *p_trail)
              || (*p_head > (SYSTEM_TMNL_MAX_NUM-1))
              || (*p_trail > (SYSTEM_TMNL_MAX_NUM-1))
              || ((*p_head !=  (p_regist_pro->rgsted_trail + 1))
                     && (*p_head != 0)))
        {
            terminal_pro_debug("Err delect unregister address %d"
                               "(head_index)----%d(trail)---%d(rgsted_trail)",
                              *p_head, *p_trail, p_regist_pro->rgsted_trail );
            return false;
        }

        /*looking for address delected*/
        for (i = *p_head; i <= *p_trail; i++) {
            if (pRelist[i] == addr) {
                delect_index = i;
                found_dl = true;
                break;
            }
        }

        if (found_dl) {
            /* swap the address of delect and the head of unregister list */
            if (swap_valtile_uint16(&pRelist[*p_trail], &pRelist[delect_index])) {
                /* make trail to 0xffff and move the trial to ahead of trail */
                pRelist[(*p_trail)--] = 0xffff;
                terminal_pro_debug("noregister list trail index = %d-trail"
                               "emlem value = %d", *p_trail, pRelist[(*p_trail)]);
                return true;
            }
        }
    }
    return false; /* default return value */
}
/*$ terminal_delect_register_addr...........................................*/
bool terminal_delect_register_addr(uint16_t addr_delect) {
    /* delect the address of being register,
        and put it to the head of unregister list */
    volatile ttmnl_register_proccess *p_regist_pro = &gregister_tmnl_pro;
    volatile uint16_t *pRelist;              /*pointer to register list */
    volatile uint16_t *p_head;/* pointer to the head of unregister list */
    volatile uint16_t *p_trail;/* pointer to the trail of register list */
    if ((p_regist_pro != NULL)
          && (addr_delect != 0xffff)) 
    {
        int i = 0, delect_index = -1;
        bool found_dl = false;
        p_head = &p_regist_pro->rgsted_head;
        p_trail = &p_regist_pro->rgsted_trail;
        pRelist = p_regist_pro->register_pro_addr_list;
        if ((*p_head > *p_trail)
              || (*p_head > (SYSTEM_TMNL_MAX_NUM-1))
              || (*p_trail > (SYSTEM_TMNL_MAX_NUM-1))
              || ((*p_head !=  (p_regist_pro->rgsted_trail + 1))
                      &&(*p_head != 0)))
        {
            terminal_pro_debug("Err delect register address %d"
                    "(head_index)----%d(trail)---%d(rgsted_trail)",
                    *p_head, *p_trail, p_regist_pro->rgsted_trail);
            return false;
        }
        /* found delect address index */              
        for (i = *p_head; i <= *p_trail; i++) {
            if (pRelist[i] == addr_delect) {
                delect_index = i;
                found_dl = true;
                break;
            }
        }
        /* found ? */
        if (found_dl) {
            /* swap the address of delect and the head of unregister list */
            if (swap_valtile_uint16(&pRelist[*p_trail], &pRelist[delect_index])) {
                /*Step1: move the head of unregister list to
                the trail of register list; Step2: move the head of unregister to the next
                unregister elememt of unregister list head*/
                p_regist_pro->noregister_head = *p_trail;
                if( *p_trail > 0 ) {/* zero is least index */
                    (*p_trail)--;
                }
                gregister_tmnl_pro.tmn_rgsted--;
                return true;
            }
        }
    }
    return false;/* default return value */
}
/*$ terminal_register.......................................................*/
bool terminal_register(uint16_t address, uint8_t dev_type,
                                tmnl_pdblist p_tmnl_station)
{
    bool bret;       /* bool return value */
    int i;      /* the varialable of loop */
    uint16_t addr; /* address of terminal */
    uint16_t type;       /* terminal type */
    tmnl_pdblist p; /* pointer to guard of terminal double link list */

    if (NULL == p_tmnl_station) {
        assert(p_tmnl_station);
        bret = false;
    }
    else {
        if (!p_tmnl_station->tmnl_dev.tmnl_status.is_rgst) {
            /* set address */
            addr = address & TMN_ADDR_MASK;
            p = dev_terminal_list_guard;
            /* found address in the address list */
            for (i = 0; i < SYSTEM_TMNL_MAX_NUM; i++) {
                if (addr == tmnl_addr_list[i].addr) {
                    /* debug register address */
                    terminal_pro_debug("register addr = %04x-"
                                       "%04x, index = %d ", addr,
                                      tmnl_addr_list[i].addr, i);
                    /* save terminal register information */
                    type = tmnl_addr_list[i].tmn_type;
                    p_tmnl_station->tmnl_dev.tmnl_status.is_rgst = true;
                    p_tmnl_station->tmnl_dev.tmnl_status.device_type = dev_type;
                    p_tmnl_station->tmnl_dev.address.addr = addr;
                    p_tmnl_station->tmnl_dev.address.tmn_type = type;
                    /* sort the terminal double list */
                    if (-1 == sort_terminal_dblist_node(p)) {
                        terminal_pro_debug("insert  register node is Err!");
                    }
                    /* set system state */
                    set_terminal_system_state(DISCUSS_STATE, true);
                    /* increment of register num */
                    gregister_tmnl_pro.tmn_rgsted++;
                    /* register success */
                    bret = true;
                    /* log message */
                    if (NULL != gp_log_imp) {
                        gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                                                          LOGGING_LEVEL_DEBUG,
                             "[ terminal (0x%016llx-%04x) registed success ]",
                                           p_tmnl_station->tmnl_dev.entity_id,
                                        p_tmnl_station->tmnl_dev.address.addr);
                    }
                    break;/* end for loop */
                }
            }
        }
    }
    return bret; /* return value */
}

/*Extern function declaration-----------------------------------------------*/
/*Extern inflight_conference_command_exist declaration......................*/
extern bool inflight_conference_command_exist(void);
/*Extern menu_first_display declaration.....................................*/
extern void menu_first_display(void);
/*$ system register terminal proccess.......................................*/
void system_register_terminal_pro(void) {
    static bool l_resetFlag = true;              /* first reset flag is true */
    static uint16_t l_norgstIndex = 0;    /* index of terminal address list */
    static uint16_t l_queryIndex =  0;/* query index in idle register state */
    volatile register_state regState = gregister_tmnl_pro.rgs_state;
    bool registing = false; /* one terminal register success */
    tmnl_pdblist pRe; /* terminal node */
    uint16_t count_num = 0; /* while count num */
    uint16_t totalNum; /* all terminal num */
    uint16_t regNum;/* register num */

    if (l_resetFlag) {
        l_resetFlag = false; /* make run once only */
        over_time_set(WAIT_TMN_RESTART, 10000);/*waiting timeout*/
    }

    if (over_time_listen(WAIT_TMN_RESTART)
         && (RGST_WAIT == regState))
    {
        gregister_tmnl_pro.rgs_state = RGST_QUERY;
        /* set register handle timeout */
        over_time_set(TRGST_OTIME_HANDLE, 15000);
    }

    if (RGST_QUERY == regState) {
        while (count_num < SYSTEM_TMNL_MAX_NUM) {
            uint16_t addr = tmnl_addr_list[l_norgstIndex].addr;
            if (addr != 0xffff) {
                pRe  = found_terminal_dblist_node_by_addr(addr);
                if (NULL == pRe) {/* not bind with 1722 target id */
                    if ((gvregister_recved)
                          ||(over_time_listen(SIG_TMNL_REGISTER))) {
                        /* query address */
                        terminal_query_endstation(addr, (uint64_t)0);
                        l_norgstIndex++;/* loop to next */
                        l_norgstIndex %= SYSTEM_TMNL_MAX_NUM;
                        /* reset register flag */
                        gvregister_recved = false;
                        /*at least 150ms*/
                        over_time_set(SIG_TMNL_REGISTER, 150);
                        registing = true;
                        /* terminal register log debug */
                        if (NULL != gp_log_imp) {
                            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                                                      LOGGING_LEVEL_DEBUG,
                                 "[Terminal (0x%016llx-%04x) Registing ]",
                                                              (uint64_t)0,
                                                                      addr);
                        }
                        break;/* register one */
                    }
                }
                else {
                    if (!pRe->tmnl_dev.tmnl_status.is_rgst) {
                        /* single terminal register finishing or timeout?*/
                        if ((gvregister_recved)
                              ||over_time_listen(SIG_TMNL_REGISTER))
                        {
                            /* query address */
                            terminal_query_endstation(addr, (uint64_t)0);
                            l_norgstIndex++;/* loop to next */
                            l_norgstIndex %= SYSTEM_TMNL_MAX_NUM;
                            /* reset register flag */
                            gvregister_recved = false;
                            /*at least 150ms*/
                            over_time_set(SIG_TMNL_REGISTER, 150);
                            registing = true;
                            /* terminal register log debug */
                            if (NULL != gp_log_imp) {
                            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                                                          LOGGING_LEVEL_DEBUG,
                                     "[Terminal (0x%016llx-%04x) Registing ]",
                                                                  (uint64_t)0,
                                                                        addr);
                            }
                            break; /* register one */
                        }
                    }
                }
            }
            
            l_norgstIndex++; /* loop to next address */
            l_norgstIndex %= SYSTEM_TMNL_MAX_NUM;
            /* count while run times */
            count_num++;
        }

        /* get total num and the num of registered */
        totalNum = gregister_tmnl_pro.tmn_total;
        regNum = gregister_tmnl_pro.tmn_rgsted;
        if (((count_num >= SYSTEM_TMNL_MAX_NUM)
                && (!registing)
                && (totalNum <= regNum))
             || (totalNum == 0U)
             || (over_time_listen(TRGST_OTIME_HANDLE)))
        {/* register finished or register timeout */
            /* debug now */
            DEBUG_INFO( "total = %d, rgsted = %d",
                     gregister_tmnl_pro.tmn_total,
                   gregister_tmnl_pro.tmn_rgsted);
            /* register idle */
            gregister_tmnl_pro.rgs_state = RGST_IDLE;
            /* set system state first */
            set_terminal_system_state(DISCUSS_STATE, true);
            menu_first_display();
            /* system discuss start, to close all mic */
            terminal_start_discuss(false);
            /* send main state of terminal */
            usleep(1000); /* for set chairman state*/
            terminal_main_state_send(0, NULL, 0);
        }
    }
    else if ((RGST_IDLE == regState)
                  && (over_time_listen(QUEUE_REGISTER_TIMEOUT)))
    {
        while (count_num < SYSTEM_TMNL_MAX_NUM) {
            uint16_t addr = tmnl_addr_list[l_queryIndex].addr;
            if (addr != 0xffff) {
                pRe = found_terminal_dblist_node_by_addr(addr);
                if (NULL == pRe) {/* not found? */
                    /* address is not in dev_terminal_list_guard double list,
                        should register again */
                    if ((gvregister_recved)
                          ||over_time_listen(SIG_TMNL_REGISTER))
                    {
                        /* send query command */
                        terminal_query_endstation(addr, (uint64_t)0);
                        /* reset register flag */
                        gvregister_recved = false;
                        over_time_set(SIG_TMNL_REGISTER, 100);/* 100ms */
                        l_queryIndex++;/* loop to next */
                        l_queryIndex %= SYSTEM_TMNL_MAX_NUM;
                        registing = true;
                        /* terminal register log debug */
                        if (NULL != gp_log_imp) {
                        gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                                LOGGING_LEVEL_DEBUG,
                                "[Terminal (0x%016llx-%04x) Registing ]",
                                (uint64_t)0,
                                addr);
                        }
                        break;/* register one */
                    }
                }
                else {
                    bool regis = false;
                    solid_pdblist p = NULL;
                    if (!pRe->tmnl_dev.tmnl_status.is_rgst) {/*no register? */
                        regis = true;
                    }

                    if (!regis && (0 != pRe->tmnl_dev.entity_id)) {
                        p = search_endtity_node_endpoint_dblist(endpoint_list,
                                               pRe->tmnl_dev.entity_id);
                        if (p != NULL) {
                            if (!p->solid.connect_flag) {/*online?*/
                                pRe->tmnl_dev.tmnl_status.is_rgst = false;
                                regis = true;
                            }
                        }
                    }

                    if ((regis)
                          && ((gvregister_recved)
                                   || (over_time_listen(SIG_TMNL_REGISTER))))
                    {
                        terminal_query_endstation(addr, (uint64_t)0);
                        gvregister_recved = false; /* wait for register success */
                        /* interval timeout set */
                        over_time_set(SIG_TMNL_REGISTER, 100);
                        l_queryIndex++;/* loop to next */
                        l_queryIndex %= SYSTEM_TMNL_MAX_NUM;
                        registing = true;
                        /* terminal register log debug */
                        if (NULL != gp_log_imp) {
                        gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                                                  LOGGING_LEVEL_DEBUG,
                             "[Terminal (0x%016llx-%04x) Registing ]",
                                                          (uint64_t)0,
                                pRe->tmnl_dev.address.addr);
                        }

                        break;/* register one */
                    }
                }
            }

            l_queryIndex++;/* loop to next */
            l_queryIndex %= SYSTEM_TMNL_MAX_NUM;
            count_num++; /* count while run times */
        }

        if ((count_num >= SYSTEM_TMNL_MAX_NUM)
              && (!registing)) { /* no terminal register*/
            over_time_set(QUEUE_REGISTER_TIMEOUT, 3*1000);/*3S*/
        }
        else {
            over_time_set(QUEUE_REGISTER_TIMEOUT, 500);
        }
    }
    else {
        /* no other else case */
    }
}
/*$ begin to register.......................................................*/
void terminal_begin_register(void) {
	gregister_tmnl_pro.rgs_state = RGST_WAIT;
	over_time_set(WAIT_TMN_RESTART, 500);
}
/*$ register system initial.................................................*/
void terminal_register_init(void) {
    int i = 0; /* varialable of loop */
    gregister_tmnl_pro.list_size = SYSTEM_TMNL_MAX_NUM;
    for (i = 0; i < SYSTEM_TMNL_MAX_NUM; i++) {
        gregister_tmnl_pro.register_pro_addr_list[i] = 0xffff;
    }

    gregister_tmnl_pro.tmn_rgsted = 0;
    gregister_tmnl_pro.tmn_total = 0;
    gregister_tmnl_pro.noregister_head = 0;
    gregister_tmnl_pro.noregister_trail = 0;
    gregister_tmnl_pro.rgsted_head = 0;
    gregister_tmnl_pro.rgsted_trail = 0;
    gregister_tmnl_pro.register_list_full = false;
    gregister_tmnl_pro.unregister_list_full = false;
    gregister_tmnl_pro.rgs_state = RGST_WAIT;
    /* wait for query register */
    gregister_tmnl_pro.rgs_query_state = QUERY_RTST_WAIT;
    /* permit to register first */
    gvregister_recved = true;
    over_time_set(SIG_TMNL_REGISTER, 50);
    over_time_set(QUEUE_REGISTER_TIMEOUT, 10*1000);/*10S*/
}
/*$ terminal_type_save......................................................*/
void terminal_type_save(uint16_t address, uint8_t tmnl_type, bool is_chman) {
    int i = 0;/* varialable of loop */
    tmnl_pdblist p;/* pointer to terminal node */

    for (i = 0; i < SYSTEM_TMNL_MAX_NUM; i++) {
        if ((address != 0xffff)
               && (address == (tmnl_addr_list[i].addr))) {
            terminal_pro_debug("terminal(--%04x--) save type = %d ",
                                                address, tmnl_type);
            /* check for right type */
            if (((is_chman)
                    && ((tmnl_type == TMNL_TYPE_CHM_COMMON)
                            ||(tmnl_type == TMNL_TYPE_CHM_EXCUTE)))
                 || ((!is_chman)
                         && ((tmnl_type == TMNL_TYPE_COMMON_RPRST)
                                 ||(tmnl_type == TMNL_TYPE_VIP))))
            {
                p = found_terminal_dblist_node_by_addr(address);
                if (p !=NULL) {
                    p->tmnl_dev.address.tmn_type = tmnl_type;
                }
                /* save type */
                tmnl_addr_list[i].tmn_type = tmnl_type;
                break; /* break for loop */
            }	
        }
    }
}
/*$ terminal_trasmint_message.................................................*/
void terminal_trasmint_message(uint16_t address, uint8_t *p_data,
                        uint16_t msg_len)
{
    assert(NULL != p_data);
    if (p_data == NULL) { /* NULL pointer? */
        return;/* return */
    }
    /* report to upper computer */
    upper_cmpt_terminal_message_report(p_data, msg_len, address);
}
/*$ find_new_apply_addr.....................................................*/
uint16_t find_new_apply_addr(terminal_address_list_pro* p_gallot,
                     terminal_address_list* p_gaddr_list, uint16_t* new_index)
{
    uint16_t temp_addr = 0;/* temp address */
    uint16_t i = 0;/* loop varialable */
    uint16_t current_index = p_gallot->index; /* current allot index */
    /* assert */
    assert((NULL != p_gallot)
                && (NULL != p_gaddr_list)
                && (NULL != new_index));
    if ((p_gaddr_list == NULL)
          || (p_gallot == NULL)
          || (new_index == NULL))
    {
        return 0xffff;
    }

    if ((current_index >= SYSTEM_TMNL_MAX_NUM)
          || (NULL == new_index))
    {
        return 0xffff;
    }

    if (p_gaddr_list[current_index].addr == 0xffff) {
        temp_addr = p_gallot->addr_start + current_index;
        *new_index = current_index;
    }
    else {
        i = current_index + 1;
        temp_addr = p_gaddr_list[i].addr;
        do {
            i %= SYSTEM_TMNL_MAX_NUM;
            if( p_gaddr_list[i].addr == 0xffff) {
                break;
            }

            i++;
        }while (i != current_index);

        if (i != current_index) {
            temp_addr = p_gallot->addr_start + i;
            *new_index = i;
        }
    }
    return temp_addr; /* return allot address */
}
/*{@ $Terminal command function begin---------------------------------------*/
/*$ Terminal command function::terminal_func_allot_address..................*/
int terminal_func_allot_address(uint16_t cmd, void *data, uint32_t data_len) {
    /* procces conference allot command data sended by terminal */
    struct endstation_to_host msg; 
    struct endstation_to_host_special spe_msg;
    terminal_address_list* p_addr_list = tmnl_addr_list;
    terminal_address_list_pro* p_allot = &allot_addr_pro;
    uint16_t new_addr = 0;
    uint16_t new_index = 0;
    uint8_t data_buf[DATAMAXLENGTH] = {0};
    uint16_t send_data_lng = 0;

    conference_end_to_host_frame_read(data, &msg, &spe_msg, 0, sizeof(msg));

    /* save address data */
    if (msg.cchdr.command_control & COMMAND_TMN_REPLY) {
        send_data_lng = 0;
        if ((p_addr_list[p_allot->index].addr != 0xffff)
             && (!p_allot->renew_flag))
       {
            p_allot->renew_flag = 1;

            terminal_pro_debug( "man type = 0x%02x ",
                     msg.cchdr.command_control & COMMAND_TMN_CHAIRMAN);
            if (msg.cchdr.command_control & COMMAND_TMN_CHAIRMAN) {
                p_addr_list[p_allot->index].tmn_type = TMNL_TYPE_CHM_EXCUTE;
                terminal_pro_debug("tmn type = %d ",
                    p_addr_list[p_allot->index].tmn_type);
            }
            else {
                p_addr_list[p_allot->index].tmn_type = TMNL_TYPE_COMMON_RPRST;
                terminal_pro_debug("tmn type = %d ",
                    p_addr_list[p_allot->index].tmn_type);
            }

            terminal_address_list tmp_addr;
            tmp_addr.addr = p_addr_list[p_allot->index].addr;
            tmp_addr.tmn_type = p_addr_list[p_allot->index].tmn_type;
            if (1 == terminal_address_list_write_file(addr_file_fd,
                &tmp_addr, 1))
            {
                /* nomal save,  start register terminal */
                gregister_tmnl_pro.tmn_total++;
            }
        }
    }
    else {
        if ((msg.data == ADDRESS_ALREADY_ALLOT)
              && (!p_allot->renew_flag)
              && (p_addr_list[p_allot->index].addr != 0xffff))
        {
            new_addr = p_addr_list[p_allot->index].addr;
        }
        else
        {
            new_addr = find_new_apply_addr(p_allot, p_addr_list, &new_index);
            p_allot->index = new_index;
            p_allot->renew_flag = 0;
            p_addr_list[p_allot->index].addr = new_addr;
        }
        
        /* low bytes in the head */
        data_buf[0] = (uint8_t)((new_addr & 0x00ff) >> 0);
        data_buf[1] = (uint8_t )((new_addr & 0xff00) >> 8);
        send_data_lng = sizeof(uint16_t);
    }
    /* reply for terminal allot */
    host_reply_terminal(ALLOCATION, msg.cchdr.address,
        data_buf, send_data_lng);
    return 0;
}
/*$ Terminal command function::terminal_func_key_action.....................*/
int terminal_func_key_action(uint16_t cmd, void *data, uint32_t data_len) {
    struct endstation_to_host msg;               /* host message format */
    struct endstation_to_host_special spe_msg;    /* special message format */
    uint16_t addr;     /*address of terminal*/
    uint8_t key_num; /* terminal key number */
    uint8_t key_value;/* terminal key value */
    uint8_t tmnl_state;  /* terminal status */
    uint8_t sys_state;         /* sys state */
    uint8_t reply;     /* the flag of reply */
    uint8_t tType;         /* terminal type */
    tmnl_pdblist p;        /* terminal node */
    /* format message to msg or spe_msg */
    conference_end_to_host_frame_read(data, &msg, &spe_msg, 0, sizeof(msg));
    /* get address */
    addr = msg.cchdr.address & TMN_ADDR_MASK;
    /* get key num */
    key_num = KEY_ACTION_KEY_NUM(msg.data);
    /* get key value */
    key_value = KEY_ACTION_KEY_VALUE(msg.data);
    /* get terminal state */
    tmnl_state = KEY_ACTION_STATE_VALUE(msg.data);
    /* sys state get */
    sys_state = get_sys_state();
    /* get reply flag */
    reply = msg.cchdr.command_control & COMMAND_TMN_REPLY;
    /* debug info */
    terminal_pro_debug("key_num = %d, key_value = %d,"
                    "tmnl_state = %d, sys_state = %d",key_num,
                            key_value, tmnl_state, sys_state);
    if (reply == COMMAND_TMN_REPLY) {/* only not reply message */
        terminal_pro_debug("key action command not valid!");
        return -1; /* error value */
    }

    switch (sys_state) {
        case SIGN_STATE:
        case VOTE_STATE:
        case GRADE_STATE:
        case ELECT_STATE: {
            terminal_vote(addr, key_num, key_value, tmnl_state, msg.data);
#ifdef MIC_PRIOR_MANEGER_ENABLE
            /* proccess key to discuccess */
            terminal_key_discuccess(addr, key_num, key_value,
                                    tmnl_state, msg.data);
#else
            terminal_key_speak(addr, key_num, key_value,
                                                        tmnl_state, msg.data);
#endif /* MIC_PRIOR_MANEGER_ENABLE */
            terminal_key_action_chman_interpose(addr, key_num,
                                             key_value, tmnl_state, msg.data);
            break;
        }
        case DISCUSS_STATE: {
            /* terminal type */
            tType = msg.cchdr.command_control & COMMAND_TMN_CHAIRMAN;
            /* proccess key to discuccess */
            terminal_key_discuccess(addr, key_num, key_value,
                                    tmnl_state, msg.data);
            /* only reply to chairman */
            terminal_chairman_apply_reply(tType, addr, key_num,
                                key_value, tmnl_state, msg.data);
            /* proccess chairman interpose */
            terminal_key_action_chman_interpose(addr, key_num, key_value,
                                        tmnl_state, msg.data);
            break;
        }
        case INTERPOSE_STATE: {
            /* proccess chairman interpose */
            terminal_key_action_chman_interpose(addr, key_num,
                                             key_value, tmnl_state, msg.data);
            break;
        }
        case CAMERA_PRESET: {
            p = found_terminal_dblist_node_by_addr(addr);
            if (p != NULL) {
                terminal_key_action_host_common_reply(msg.data, p);
                terminal_key_preset(0, addr, tmnl_state, key_num, key_value);
            }
            break;
        }
        default: {
            /* will never come in this case */
            break;
        }
    }
    return 0;/* default value */
}
/*$ Terminal command function::terminal_func_chairman_control...............*/
int terminal_func_chairman_control(uint16_t cmd,
                                       void *data, uint32_t data_len)
{
    struct endstation_to_host msg; /* message format */
    struct endstation_to_host_special spe_msg; /* special message format */
    uint16_t addr; /* addresss of termianl */
    uint8_t sign_value; /* sign value */
    uint8_t chair_opt; /* chairman option */
    uint8_t sign_flag; /* sign flag */
    tmnl_pdblist p, tmp; /* terminal node */
    thost_system_set set_sys; /*system profile format*/

    /* set temp profile */
    conference_end_to_host_frame_read(data, &msg, &spe_msg, 0, sizeof(msg));
    addr = msg.cchdr.address & TMN_ADDR_MASK;
    chair_opt = msg.data&CHAIRMAN_CONTROL_MEET_MASK;
    memcpy(&set_sys, &gset_sys, sizeof(thost_system_set));
    sign_value = 0; /* error or  */

    /* found terminal node basing on current node */
    tmp = found_terminal_dblist_node_by_addr(addr);
    if (tmp == NULL) {
        terminal_pro_debug("not found chairman conntrol address!");
        return -1;
    }

    if (chair_opt != CHM_BEGIN_VOTE) {
        terminal_chairman_control_meeting(tmp->tmnl_dev.entity_id,
                    addr, sign_value);
    }

    switch (chair_opt) {
        case CHM_BEGIN_SIGN: {
            gset_sys.sign_type = KEY_SIGN_IN;
            terminal_chman_control_start_sign_in(KEY_SIGN_IN, 10);
            break;
        }
        case CHM_END_SIGN: {
            /* ending sign */
            terminal_end_sign(0, NULL, 0);
            break;
        }
        case CHM_BEGIN_VOTE: {
            if (gtmnl_signstate) { /* sign proccessed */
                terminal_chairman_control_meeting(tmp->tmnl_dev.entity_id,
                                addr, sign_value);
                terminal_chman_control_begin_vote(VOTE_MODE, false,
                                &sign_flag);/* last key value */
            }
            else {
                sign_value = 1; /* not sign */
                terminal_chairman_control_meeting(tmp->tmnl_dev.entity_id,
                                                addr, sign_value);
            }
            break;
        }
        case CHM_END_VOTE: {
            /* sending command of end voting */
            terminal_end_vote(0, NULL, 0);
            /* set terminal type */
            gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0;
            gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keyup = 0;
            terminal_state_set_base_type(BRDCST_ALL,
                        gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);
            terminal_state_all_copy_from_common();
            /*set lcd */
            terminal_lcd_display_num_send(BRDCST_ALL,
                        LCD_OPTION_CLEAR, VOTE_INTERFACE);
            /* set led */
            terminal_led_set_save(BRDCST_ALL, TLED_KEY2, TLED_OFF);
            terminal_led_set_save(BRDCST_ALL, TLED_KEY3, TLED_OFF);
            terminal_led_set_save(BRDCST_ALL, TLED_KEY4, TLED_OFF);
            fterminal_led_set_send(BRDCST_ALL);
            /*send voting result */
            terminal_broadcast_end_vote_result(msg.data&0x10?BRDCST_ALL:addr);
            break;
        }
        case CHM_SUSPEND_VOTE: {
            /* pause voting */
            terminal_pause_vote(0, NULL, 0);
            break;
        }
        case CHM_RECOVER_VOTE: {
            /* recover voting */
            terminal_regain_vote(0, NULL, 0);
            break;
        }
        case CHM_RETURN_DISCUSS: {
            /* return to discuss */
            set_terminal_system_state(DISCUSS_STATE, true);
            terminal_start_discuss(false);
            break;
        }
        case CHM_CLOSE_ALL_MIC: {
            assert(dev_terminal_list_guard);
            if (dev_terminal_list_guard == NULL) {
                return -1;
            }
            /* close all chairman terminal */
            for (p = dev_terminal_list_guard->next;
                  p != dev_terminal_list_guard;
                  p = p->next)
            {
                if ((p->tmnl_dev.address.addr != 0xffff)
                      && (p->tmnl_dev.tmnl_status.is_rgst)
                      && (p->tmnl_dev.address.tmn_type ==\
                                    TMNL_TYPE_COMMON_RPRST)
                      && (p->tmnl_dev.tmnl_status.mic_state \
                                             != MIC_COLSE_STATUS))
                {
                    p->tmnl_dev.tmnl_status.mic_state = MIC_COLSE_STATUS;
                    trans_model_unit_disconnect(p->tmnl_dev.entity_id, p);
                    terminal_speak_track(p->tmnl_dev.address.addr, false);
                }
            }

            cmpt_miscrophone_status_list();
            gdisc_flags.speak_limit_num = 0;
            gdisc_flags.apply_num = 0;
            gdisc_flags.currect_first_index = gdisc_flags.apply_limit;
            terminal_main_state_send(0, NULL, 0);
            break;
        }
        default: {
            break;
        }
    }
    return 0; /* default return value */
}
/*$ terminal end vote process...............................................*/
void Terminal_endVotePro(void) {
    /* sending command of end voting */
    terminal_end_vote(0, NULL, 0);
    /* set terminal type */
    gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0;
    gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keyup = 0;
    terminal_state_set_base_type(BRDCST_ALL,
                gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);
    terminal_state_all_copy_from_common();
    /*set lcd */
    terminal_lcd_display_num_send(BRDCST_ALL,
                LCD_OPTION_CLEAR, VOTE_INTERFACE);
    /* set led */
    terminal_led_set_save(BRDCST_ALL, TLED_KEY2, TLED_OFF);
    terminal_led_set_save(BRDCST_ALL, TLED_KEY3, TLED_OFF);
    terminal_led_set_save(BRDCST_ALL, TLED_KEY4, TLED_OFF);
    fterminal_led_set_send(BRDCST_ALL);
}
/*$ Terminal command function::terminal_func_send_main_state................*/
int terminal_func_send_main_state(uint16_t cmd,
                            void *data,uint32_t data_len)
{
    terminal_main_state_send(0, NULL, 0);
    return 0;
}
/*$ Terminal command function::terminal_func_cmd_event......................*/
int terminal_func_cmd_event(uint16_t cmd, void *data, uint32_t data_len) {
    struct endstation_to_host msg;
    struct endstation_to_host_special spe_msg;
    conference_end_to_host_frame_read(data, &msg, &spe_msg, 0, sizeof(msg));
    uint16_t addr = msg.cchdr.address & TMN_ADDR_MASK;
    thost_system_set set_sys; /* system profile format */
    tmnl_pdblist p; /* terminal node */
    uint8_t dis_mode; /* discuss mode */
    thost_sys_state sys_state; /* system state */
    uint8_t special_event; /* special message data */
    memcpy(&set_sys, &gset_sys, sizeof(thost_system_set));

    /* reply termianl */
    if (msg.cchdr.command_control & COMMAND_TMN_REPLY) {
        return -1;
    }

    p = found_terminal_dblist_node_by_addr(addr);
    if (p == NULL) {
        return -1;
    }

    /* reply special event for terminal */
    terminal_endstation_special_event_reply(p->tmnl_dev.entity_id, addr);

    special_event = msg.data;
    if (special_event == SIGN_IN_SPECIAL_EVENT) {/* terminal sign */
        terminal_sign_in_special_event(p);
        sys_state = get_terminal_system_state();
        if (DISCUSS_STATE == sys_state.host_state) {
            dis_mode = set_sys.discuss_mode;
            if ((APPLY_MODE == dis_mode)
                  && (p->tmnl_dev.address.tmn_type ==\
                                        TMNL_TYPE_CHM_EXCUTE))
            {
                terminal_chairman_apply_type_set(addr);
            }
            else {
                terminal_state_set_base_type(addr,
                        gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);
            }
        }
    }
    return 0;/* default return value */
}
/*$ Terminal command function::terminal_mic_auto_close......................*/
int terminal_mic_auto_close(uint16_t cmd, void *data, uint32_t data_len) {
    uint8_t auto_close = 0; /* auto close flag */
    tmnl_pdblist p; /* terminal node */
    int i; /* loop varialable */
    thost_system_set set_sys; /* temp varialable of system profile setting */
    uint64_t id; /* terminal 1722.1 target id */
    uint16_t addr; /* address of terminal */
    /* set temp variabable */
    memcpy(&set_sys, &gset_sys, sizeof(thost_system_set));
    /*set state of micphone */
    auto_close = set_sys.auto_close;
    /* set state operation */
    for (i = 0; i < TMNL_TYPE_NUM; i++) {
        gtmnl_state_opt[i].auto_close = auto_close?1:0;
        gtmnl_state_opt[i].MicClose = MIC_CLOSE;
    }
    /* close all micphone */
    p = dev_terminal_list_guard->next;
    for (; p != dev_terminal_list_guard; p = p->next) {
        id = p->tmnl_dev.entity_id;
        if ((p->tmnl_dev.tmnl_status.is_rgst)
              && (trans_model_unit_is_connected(id)))
        {
            addr = p->tmnl_dev.address.addr;
            /* check and disconnect terminal */
            if (0 == trans_model_unit_disconnect(id, p)) {
                /* camera track */
                terminal_speak_track(addr, false);
            }
        }
    }
    /* send main state */
    terminal_main_state_send(0, NULL, 0);
    return 0; /* default value */
}
/*$ Terminal command function::terminal_main_state_send.....................*/
int terminal_main_state_send(uint16_t cmd, void *data, uint32_t data_len) {
    tmnl_main_state_send host_main_state;/* main state */
    uint8_t spk_num = 0;               /* speak number */
    uint8_t spk_common_num = 0; /* speak common number */
    uint8_t apply_num = 0;                /* apply num */
    tmnl_pdblist p, pGuard;	          /* terminal node */
    thost_system_set set_sys;  /* temp varialable of system profile setting */

    terminal_pro_debug("conference_stype = %d",
                     host_main_state.conference_stype);
    /* set guard node */
    pGuard = dev_terminal_list_guard;
    assert(NULL != pGuard);
    if (pGuard == NULL) { /* no terminal */
        return -1;
    }

    for (p = pGuard->next; p != pGuard; p = p->next) {
        if ((p->tmnl_dev.address.addr != 0xffff)
              && (p->tmnl_dev.tmnl_status.mic_state == MIC_OPEN_STATUS))
        {
            spk_num++;
        }

        /* calculate the number of speaking person who is not chairman */
        if ((p->tmnl_dev.address.addr != 0xffff)
              && (p->tmnl_dev.tmnl_status.is_rgst)
              && (p->tmnl_dev.address.tmn_type == TMNL_TYPE_COMMON_RPRST)
              && (p->tmnl_dev.tmnl_status.mic_state == MIC_OPEN_STATUS))
        {
            spk_common_num++;
        }

        /* calculate the number of speaking person who is appling */
        if ((p->tmnl_dev.address.addr != 0xffff)
              && (p->tmnl_dev.tmnl_status.is_rgst)
              && ((p->tmnl_dev.tmnl_status.mic_state ==\
                                                MIC_FIRST_APPLY_STATUS)
                      ||(p->tmnl_dev.tmnl_status.mic_state ==\
                                    MIC_OTHER_APPLY_STATUS)))
        {
            apply_num++;
        }
    }

    memcpy(&set_sys, &gset_sys, sizeof(thost_system_set));
    host_main_state.unit = gregister_tmnl_pro.tmn_total;
    host_main_state.camera_follow = set_sys.camara_track ? 1 : 0;
    host_main_state.chm_first = set_sys.temp_close ? 1 : 0;
    /* set low 3bit change intime 2016-06-28 */
    host_main_state.conference_stype = (set_sys.discuss_mode&0x07);
    /* set the number of limited speak */
    host_main_state.limit = set_sys.speak_limit;
    /* set the number of limited apply */
    host_main_state.apply_set = set_sys.apply_limit;
    /* set the number of speaking currently */
    host_main_state.spk_num = spk_num;
    host_main_state.apply = gdisc_flags.apply_num;
    /* muticastor to all terminal */
    terminal_host_send_state(BRDCST_1722_ALL, host_main_state);
    return 0;/* default number */
}

/*terminal_lcd_display_num_send.............................................*/
int terminal_lcd_display_num_send(uint16_t addr, uint8_t display_opt,
    uint8_t display_num)
{
    tmnl_send_end_lcd_display lcd_dis;
    lcd_dis.opt = display_opt;
    lcd_dis.num = display_num;

    if (display_opt == LCD_OPTION_DISPLAY) {
        glcd_num = display_num; /* set lcd number */
    }

    terminal_send_end_lcd_display(0, addr, lcd_dis);

    return 0; /* return success */
}
/*$ Terminal command function::terminal_pause_vote..........................*/
int terminal_pause_vote(uint16_t cmd, void *data, uint32_t data_len) {
    /* set voting  pause command to all terminal */
    terminal_option_endpoint(BRDCST_1722_ALL,
            CONFERENCE_BROADCAST_ADDRESS, OPT_TMNL_SUSPEND_VOTE);
    return 0; /* return success */
}
/*$ Terminal command function::terminal_regain_vote.........................*/
int terminal_regain_vote(uint16_t cmd, void *data, uint32_t data_len) {
    /* set voting regain command to all terminal */
    terminal_option_endpoint(BRDCST_1722_ALL,
            CONFERENCE_BROADCAST_ADDRESS, OPT_TMNL_RECOVER_VOTE);
    return 0;/* return success */
}
/*$ Terminal command function::terminal_system_discuss_mode_set.............*/
int terminal_system_discuss_mode_set(uint16_t cmd, void *data,
                                                        uint32_t data_len)
{
    uint8_t dis_mode;          /* discuss mode */
    tmnl_pdblist p; /*pointer to terminal node */
    uint8_t micState; /* mic status */
#ifdef MIC_PRIOR_MANEGER_ENABLE
    TEReqQePrior prior;
    bool reSuccess;
    uint8_t tType;
#endif /* MIC_PRIOR_MANEGER_ENABLE */ 

    assert((data != (void *)0)
                && (dev_terminal_list_guard != (tmnl_pdblist)0));
    if ((data == NULL)
          || (dev_terminal_list_guard == (tmnl_pdblist)0))
    {   
         return -1;
    }

    /* initail speak track pro */
    terminal_speak_track_pro_init();
    
    /*set discuss mode */
    dis_mode = *((uint8_t*)data);
    gdisc_flags.edis_mode = (ttmnl_discuss_mode)dis_mode;
    gdisc_flags.currect_first_index = MAX_LIMIT_APPLY_NUM;
    gdisc_flags.apply_num = 0;
#ifndef MIC_PRIOR_MANEGER_ENABLE
    gdisc_flags.speak_limit_num = 0;
#endif /* MIC_PRIOR_MANEGER_ENABLE */

    if (APPLY_MODE == dis_mode) {
        terminal_chairman_apply_type_set(BRDCST_EXE);
    }
    else {
        terminal_chairman_apply_type_clear(BRDCST_EXE);
    }

    /* initial pointer */
    p = dev_terminal_list_guard;
    for (p = p->next; p != dev_terminal_list_guard; p = p->next) {
        micState = p->tmnl_dev.tmnl_status.mic_state;        
        if ((p->tmnl_dev.tmnl_status.is_rgst)
              && trans_model_unit_is_connected(p->tmnl_dev.entity_id))
        {
#ifdef MIC_PRIOR_MANEGER_ENABLE
                tType = p->tmnl_dev.address.tmn_type;
                if ((tType == TMNL_TYPE_CHM_EXCUTE)
                      || (tType == TMNL_TYPE_CHM_COMMON))
                {
                    prior = CHAIRMAN_PRIOR;
                }
                else if (tType == TMNL_TYPE_VIP) {
                    prior = VIP_PRIOR;
                }
                else { /* conmon type */
                    prior = COMMON_PRIOR;
                }
                
                /* request connection */
                reSuccess = Terminal_requestDisConnect(p,
                    prior, MAX_FAILURE_TIMES,
                    get_sys_state());
                if (reSuccess) {
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_DEBUG,
                        "[Dismode reset Terminal(0x%04x) Close "
                        " Request disconnection success: Waitting for"
                        " disConnection...]", p->tmnl_dev.address.addr);
                }
                else {
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_ERROR,
                        "[Dismode reset Terminal(0x%04x) Close "
                        " Request disconnection Faied]",
                        p->tmnl_dev.address.addr);
                }
#else
                if (0 ==trans_model_unit_disconnect(p->tmnl_dev.entity_id,
                        p))
                {
                    terminal_speak_track(p->tmnl_dev.address.addr, false);
                }
#endif /* MIC_PRIOR_MANEGER_ENABLE */                
        }
        else if ((p->tmnl_dev.tmnl_status.is_rgst)
                      && (micState != MIC_COLSE_STATUS))
        {
            terminal_set_mic_status(MIC_COLSE_STATUS,
                p->tmnl_dev.address.addr, p->tmnl_dev.entity_id);
            
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_DEBUG,
                "[terminal_system_discuss_mode_set "
                "Send Mic status(%d) Command Success]",
                MIC_COLSE_STATUS);
        }
        else { 
            /* do nothing */
        }
    }
    
    /* send main state */
    terminal_main_state_send(0, NULL, 0);
    return 0;/* return success */
}
/*$ Terminal command function::terminal_speak_limit_num_set.................*/
int terminal_speak_limit_num_set(uint16_t cmd, void *data,
                                                uint32_t data_len)
{ /* open apply list terminal because of speaking
        limit number reset bigger than before */
    thost_system_set ss; /* system setting */
    uint16_t curAddr; /* current address */
    tmnl_pdblist p; /* pointer to termianl node */
    uint64_t id; /* 1722.1 taget id */
    
    memcpy(&ss, &gset_sys, sizeof(thost_system_set));

    /* open lest mic */
    while ((ss.speak_limit > gdisc_flags.limit_num)
               && (ss.discuss_mode == LIMIT_MODE)
               && (gdisc_flags.apply_num > 0))
    {
        curAddr = gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index];
        if (addr_queue_delete_by_index(gdisc_flags.apply_addr_list,
                &gdisc_flags.apply_num, gdisc_flags.currect_first_index))
        {/* open next apply terminal */
            p = found_terminal_dblist_node_by_addr(curAddr);
            if (p != NULL) {
                id = p->tmnl_dev.entity_id;
                
                if (!trans_model_unit_is_connected(id)) {
#ifdef MIC_PRIOR_MANEGER_ENABLE
#ifdef FIRST_SPEAK_QUEUE_ENABLE                    
                    if (Terminal_firstSpkRequestConnect(p,
                            &l_FSpkQueue, &l_FSpkLocker,
                            MAX_CONNECT_FAILURE_TIMES,
                            COMMON_SPK_PERMISSION))
                    {
                        gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                            LOGGING_LEVEL_DEBUG,
                            "[terminal_speak_limit_num_set() Speak Post First"
                            " speaking node(address = 0x%x)"
                            " to Queue Success ]",
                            p->tmnl_dev.address.addr);
                    }
                    else {
                        gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                            LOGGING_LEVEL_ERROR,
                            "[terminal_speak_limit_num_set() Speak Post First"
                            " speaking node(address = 0x%x)"
                            "to Queue Failed ]",
                            p->tmnl_dev.address.addr);
                    }
#else
                    bool reSuccess;
                    
                    /* request connection */
                    reSuccess = Terminal_requestConnect(p,
                        COMMON_PRIOR, MAX_FAILURE_TIMES,
                        COMMON_SPK_PERMISSION);
                    if (reSuccess) {
                        gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                            LOGGING_LEVEL_DEBUG,
                            "[terminal_speak_limit_num_set() "
                            "Terminal(0x%04x) Speak "
                            " Request connection success: Waitting for"
                            "Connection...]",
                            p->tmnl_dev.address.addr);
                    }
                    else {
                        gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                            LOGGING_LEVEL_ERROR,
                            "[terminal_speak_limit_num_set()"
                            " Terminal(0x%04x) Speak "
                            " Request connection Faied]",
                            p->tmnl_dev.address.addr);
                    }
#endif /* FIRST_SPEAK_QUEUE_ENABLE */
#else
                    if (0 == trans_model_unit_connect(id, p)){
                        /* connect success */
                        gdisc_flags.speak_limit_num++;
                        terminal_speak_track(p->tmnl_dev.address.addr, true);
                        terminal_apply_list_first_speak();
                    }
#endif /* MIC_PRIOR_MANEGER_ENABLE */                    
                }
            }
            else {
                terminal_pro_debug(" no such tmnl dblist node!");
            }
        }
        else {
            gdisc_flags.currect_first_index = 0;
        }
    }

    /* set new limit speak number */
    gdisc_flags.limit_num = ss.speak_limit;
    /* main state send */
    terminal_main_state_send(0, NULL, 0);
    /* return success */
    return 0;
}
/*$ Terminal command function::terminal_apply_limit_num_set.................*/
int terminal_apply_limit_num_set(uint16_t cmd, void *data,
                                                    uint32_t data_len)
{
    uint8_t alb; /* apply limit number */
    assert(NULL != data);
    if (NULL == data) {
        return -1; /* return failed */
    }
    /* setting after database update */
    alb = *((uint8_t*)data);
    gdisc_flags.apply_limit = alb;
    gset_sys.apply_limit = alb;
    /* main state send */
    terminal_main_state_send(0, NULL, 0);
    (void)cmd; /* avoid warning */
    (void)data_len; /* avoid warning */
    /* return success */
    return 0;
}
/*$ Terminal command function::terminal_limit_speak_time_set................*/
int terminal_limit_speak_time_set(uint16_t cmd, void *data,
                                                    uint32_t data_len)
{
    tmnl_limit_spk_time spk_time; /* struct speak limit time */
    thost_system_set ss; /* system setting */
    uint16_t nolimit_addr; /* no limit address */
    uint16_t limit_addr; /* limit address */
    tmnl_limit_spk_time nolimit_spk_time; /* struct no speak limit time */
    
    memcpy(&ss, &gset_sys, sizeof(thost_system_set));
    spk_time.limit_time = ss.spk_limtime;
    speak_limit_time = (uint8_t)spk_time.limit_time;
    /* no limit time? */
    if(!ss.spk_limtime) {
        terminal_limit_spk_time(0, BRDCST_ALL, spk_time);
    }
    else {
        nolimit_addr = 0;
        limit_addr = BRDCST_MEM; /* to all common */

        if (ss.vip_limitime) {
            /* vip limit time */
            limit_addr |= BRDCST_VIP;
        }
        else {
            nolimit_addr |= BRDCST_VIP;
        }

        if (ss.chman_limitime) {
            /* chairman limit time */
            limit_addr |= BRDCST_CHM |BRDCST_EXE;
        }
        else {
            nolimit_addr |= BRDCST_CHM |BRDCST_EXE;
        }

        if (nolimit_addr) {
            nolimit_spk_time.limit_time = 0;
            terminal_limit_spk_time(BRDCST_1722_ALL,
                                    nolimit_addr, nolimit_spk_time);
        }

        terminal_limit_spk_time(BRDCST_1722_ALL, limit_addr, spk_time);
    }

    return 0;
}
/*$ Terminal command function::terminal_end_sign............................*/
int terminal_end_sign(uint16_t cmd, void *data, uint32_t data_len)
{
    /* set discuss state */
    set_terminal_system_state(DISCUSS_STATE, true);
    gtmnl_signstate = SIGN_IN_BE_LATE;
    /* start discuss state */
    terminal_start_discuss(false);
    /* begin sign in late and set timeout of signing */
    over_time_set(SIGN_IN_LATE_HANDLE, gsign_latetime * 60 * 1000);
    gquery_svote_pro.running = false;
    gquery_svote_pro.endQr = true;
    /* return success */
    return 0;
}
/*$ Terminal command function::terminal_end_vote............................*/
int terminal_end_vote( uint16_t cmd, void *data, uint32_t data_len) {
    /*set end voting flag */
    gvote_flag = NO_VOTE;
    /* end query voting result */
    gquery_svote_pro.index = 0;
    gquery_svote_pro.running = false;
    gquery_svote_pro.endQr = true;
    host_timer_stop(&gquery_svote_pro.query_timer);
    /* return success */
    return 0;
}
/*$ Terminal command function::termianal_music_enable.......................*/
int termianal_music_enable(uint16_t cmd, void *data, uint32_t data_len) {
    uint8_t t; /* the temp varialable music enable flag */
    if ((data_len != sizeof(uint8_t))
        || (data == NULL))
    {   
        return -1;/* return failed */
    }
    /* get flags */
    t = *((uint8_t*)data);
    if (gset_sys.chman_music != t) {
        /* set flags */
        gset_sys.chman_music = t;
    }
    /* return success */
    return 0;
}
/*$ Terminal command function::termianal_chairman_prior_set.................*/
int termianal_chairman_prior_set(uint16_t cmd, void *data,
                                uint32_t data_len)
{
    uint8_t t; /* the temp varialable of prior flag */
    if ((data_len != sizeof(uint8_t))
          || (data == NULL))
    {   
        return -1;/* return failed */
    }
    /* get flags */
    t = *((uint8_t*)data);
    if (gset_sys.chman_first != t) {
        /* set flags */
        gset_sys.chman_first = t;
    }
    /* return success */
    return 0;
}
/*$ Terminal command function::terminal_system_register.....................*/
int terminal_system_register(uint16_t cmd, void *data, uint32_t data_len) {
/* register terminal */
    uint16_t t; /* the temp varialable of address */
    assert(NULL != data);
    if (data == NULL) {
        return -1;/* return failed */
    }
    /* get flag */
    t = *((uint16_t*)data);
    /* send query endstation command */
    terminal_query_endstation(t, BRDCST_1722_ALL);
    /* return success */
    return 0;
}
/*$ Terminal command function::termianal_temp_close_set.....................*/
int termianal_temp_close_set(uint16_t cmd, void *data, uint32_t data_len) {
/* SET temp close flags */
    uint8_t t; /* temp close flags */
    assert (data != NULL);
    if ((data_len != sizeof(uint8_t))
        || (data == NULL))
    {
        return -1;/* return failed */
    }
    /* get flag */
    t = *((uint8_t*)data);
    if (gset_sys.temp_close != t) {
        /* set flag */
        gset_sys.temp_close = t;
    }
    /* return success */
    return 0;
}
/*$ Terminal command function::termianal_camera_track_set...................*/
int termianal_camera_track_set(uint16_t cmd, void *data, uint32_t data_len) {
/* SET camera track flags */
    uint8_t ct; /* camara track flag */
    assert(data != NULL);
    if ((data_len != sizeof(uint8_t))
          || (data == NULL)) {
        return -1;/* return failed */
    }
    /* get flag */
    ct = *((uint8_t*)data);
    if (gset_sys.camara_track != ct) {
        /* set flag */
        gset_sys.camara_track = ct;
    }
    /* main state send */
    terminal_main_state_send(0, NULL, 0);
    /* return success */
    return 0;
}
/*$ Terminal command function ending--------------------------------------@}*/

/*$ terminal_socroll_synch()................................................*/
int terminal_socroll_synch(void) {
    terminal_option_endpoint(BRDCST_1722_ALL,
            CONFERENCE_BROADCAST_ADDRESS,
            OPT_TMNL_LED_DISPLAY_ROLL_SYNC);
    /* return value */
    return 0;
}
/*$ terminal_remove_unregitster()...........................................*/
void terminal_remove_unregitster(void) {
    tmnl_pdblist pStore = (tmnl_pdblist)0;
    tmnl_pdblist pLoop = (tmnl_pdblist)0;

#ifdef __DEBUG__
    show_terminal_dblist(dev_terminal_list_guard);
#endif

    pLoop = dev_terminal_list_guard->next;
    for (; pLoop != dev_terminal_list_guard; pLoop = pStore) {
        uint64_t id = pLoop->tmnl_dev.entity_id;
        pStore = pLoop->next;
        if ((pLoop->tmnl_dev.address.addr == 0xffff)
            || (!pLoop->tmnl_dev.tmnl_status.is_rgst))
        {
            delect_terminal_dblist_node(&pLoop);
            conference_transmit_model_node_destroy(id);
        }
    }

#ifdef __DEBUG__
    show_terminal_dblist(dev_terminal_list_guard);
#endif
}
/*$ terminal_mic_state_set()................................................*/
void terminal_mic_state_set(uint8_t mic_status, uint16_t addr,
    uint64_t tarker_id, bool is_report_cmpt, tmnl_pdblist tmnl_node)
{
    if  ((tmnl_node == (tmnl_pdblist)0)
          && (!(addr & BROADCAST_FLAG)))
    {
        terminal_pro_debug("nothing to send to set mic status!");
        return;
    }

    /* set terminal microphone status */
    terminal_set_mic_status(mic_status, addr, tarker_id);
    
    if (tmnl_node != (tmnl_pdblist)0) {
        if ((tmnl_node->tmnl_dev.address.tmn_type == TMNL_TYPE_CHM_EXCUTE)
              && (mic_status == MIC_CHM_INTERPOSE_STATUS))
        {
            upper_cmpt_report_mic_state(MIC_OPEN_STATUS,
                tmnl_node->tmnl_dev.address.addr);
        }
        else {
            upper_cmpt_report_mic_state(mic_status,
                tmnl_node->tmnl_dev.address.addr);
        }
        
        if ((is_report_cmpt)
              && (mic_status != MIC_CHM_INTERPOSE_STATUS))
        {
            tmnl_node->tmnl_dev.tmnl_status.mic_state = mic_status;
        }
    }
}
/*$ terminal_mic_state_set_send_terminal()..................................*/
void	terminal_mic_state_set_send_terminal(bool send_tmnl,
        uint8_t mic_status, uint16_t addr, uint64_t tarker_id,
        bool is_report_cmpt, tmnl_pdblist tmnl_node)
{
    assert(tmnl_node != (tmnl_pdblist)0);
    if ((tmnl_node == NULL)
          && (!(addr & BROADCAST_FLAG)))
    {
        terminal_pro_debug("nothing to send to set mic status!");
        return;
    }

    if (send_tmnl) {
        terminal_set_mic_status(mic_status, addr, tarker_id);
    }

    if (tmnl_node != NULL) {
        if ((tmnl_node->tmnl_dev.address.tmn_type == TMNL_TYPE_CHM_EXCUTE)
            && (mic_status ==MIC_CHM_INTERPOSE_STATUS))
        {
            upper_cmpt_report_mic_state(MIC_OPEN_STATUS,
                tmnl_node->tmnl_dev.address.addr);
        }
        else {
            upper_cmpt_report_mic_state(mic_status,
                tmnl_node->tmnl_dev.address.addr);
        }
        if ((is_report_cmpt)
              && (mic_status != MIC_CHM_INTERPOSE_STATUS))
        {
            tmnl_node->tmnl_dev.tmnl_status.mic_state = mic_status;
        }
    }       
}
/*$ terminal_speak_limit_timeout_set()......................................*/
int terminal_speak_limit_timeout_set(tmnl_pdblist p_tmnl_node) {
    uint8_t spk_limit_time = gset_sys.speak_limit;
    bool vip_time_limit = gset_sys.vip_limitime ? true : false;
    bool chm_time_limit = gset_sys.chman_limitime ? true : false;
    int ret = -1;

    /* Limit speak? */
    if ((p_tmnl_node != NULL)
        && (!spk_limit_time))
    {
        if (p_tmnl_node->tmnl_dev.address.tmn_type
             == TMNL_TYPE_COMMON_RPRST)
        {
            host_timer_start(spk_limit_time*60*1000,
                &p_tmnl_node->tmnl_dev.spk_timeout );
        }
        else if (p_tmnl_node->tmnl_dev.address.tmn_type
                    == TMNL_TYPE_VIP)
        {
            if (vip_time_limit) {
                host_timer_start(spk_limit_time*60*1000,
                &p_tmnl_node->tmnl_dev.spk_timeout);
            }
        }
        else if ((p_tmnl_node->tmnl_dev.address.tmn_type
                        == TMNL_TYPE_CHM_COMMON)
                     || (p_tmnl_node->tmnl_dev.address.tmn_type
                            == TMNL_TYPE_CHM_EXCUTE))
        {
            if(chm_time_limit) {
                host_timer_start(spk_limit_time*60*1000,
                &p_tmnl_node->tmnl_dev.spk_timeout);
            }
        }
        else {
            /* never come this case */
        }

        ret = 0;
    }

    return ret;
}
/*${Terminal::isMicQueueEmpty}..............................................*/
/*${Terminal::isMicQueueEmpty}*/
static bool Terminal_isMicQueueEmpty(Terminal_micQueue * const queue) {
    assert(queue != NULL);
    return queue->head == queue->trail;
}
/*${Terminal::isMicQueueFull}...............................................*/
/*${Terminal::isMicQueueFull}*/
static bool Terminal_isMicQueueFull(Terminal_micQueue * const queue) {
    assert(queue != NULL);
    return (queue->trail + 1) % queue->size == queue->head;
}
/*${Terminal::postMicFiFo}..................................................*/
/*${Terminal::postMicFiFo}*/
static bool Terminal_postMicFiFo(Terminal_micQueue * const queue, 
                                     Terminal_mic node)
{
    assert(queue != NULL);
    if (!Terminal_isMicQueueFull(queue)) {
        queue->pBuf[queue->trail] = node;
        queue->trail = (queue->trail + 1U) % queue->size;
        return true;
    }
    return false;
}
/*${Terminal::popMicFiFo}..................................................*/
/*${Terminal::popMicFiFo}*/
static bool Terminal_popMicFiFo(Terminal_micQueue* const queue,
                                     Terminal_mic *node)
{
    assert(queue != NULL);
    if (!Terminal_isMicQueueEmpty(queue)) {
        *node = queue->pBuf[queue->head];
        queue->head = (queue->head + 1U) % queue->size;
        return true;
    }
    return false;
}
/*${Terminal::micCallbackPro}...............................................*/
/*${Terminal::micCallbackPro}*/
void Terminal_micCallbackPro(void) {
    static Terminal_mic l_setNode;
    static bool l_finish = true;
    tmnl_pdblist p;
    uint32_t curTime;

    if (l_finish) {/*finish proccess the head*/
        Terminal_popMicFiFo(&l_micQueue, &l_setNode);
    }

    curTime = get_current_time();
    p = l_setNode.node;
    if ((p != NULL)
          && ((curTime - l_setNode.timeTick) > MIC_SET_TIME_OUT))
    {
        if (get_sys_state() != INTERPOSE_STATE) {
            terminal_set_mic_status(l_setNode.micState,
                                p->tmnl_dev.address.addr,
                                p->tmnl_dev.entity_id);
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_DEBUG,
                    "[ Send Mic status(%d) Command Success ]",
                    l_setNode.micState);
        }
        
        upper_cmpt_report_mic_state(l_setNode.micState, 
                                p->tmnl_dev.address.addr);
        
        terminal_main_state_send(0, NULL, 0);
        /*send twice before finishing pro main state send coding*/
        terminal_main_state_send(0, NULL, 0);
        l_setNode.node = NULL;
        l_finish = true;
    }
    else if (p != NULL) {
        l_finish = false;
    }
    else {
        /*no node in queue,do noting*/
    }
}

/*********************************************************
*writer:YasirLiang
*Date:2016/3/16
*Name:terminal_mic_status_set_callback
*Param:
*	connect_flag:mic connect flag
*	p_tmnl_node:callback terminal node
*Retern Value:
*	None
*state:打开或关闭终端mic的回调函数:设置mic的状态;若成功连接，设置发言的
*超时时间
***********************************************************/ 
void terminal_mic_status_set_callback(bool connect_flag,
    tmnl_pdblist p_tmnl_node)
{
    uint8_t mic_status, sysState, micType;
    Terminal_mic micNode;

    assert(p_tmnl_node != NULL);
    if (p_tmnl_node == NULL) {
        return;
    }

    sysState = get_sys_state();
    mic_status = connect_flag ? MIC_OPEN_STATUS : MIC_COLSE_STATUS;
    micType = p_tmnl_node->tmnl_dev.address.tmn_type;
    
    if ((sysState == INTERPOSE_STATE)
            && (micType != TMNL_TYPE_CHM_EXCUTE)
            && (micType != TMNL_TYPE_CHM_COMMON))
    {
        /*interpose state should will save mic state if endpiont
        *is common and temp close is set.
        */
        if (!gset_sys.temp_close) {
            p_tmnl_node->tmnl_dev.tmnl_status.mic_state = mic_status;
        }
    }
    else if (sysState != INTERPOSE_STATE) {
        p_tmnl_node->tmnl_dev.tmnl_status.mic_state = mic_status;
    }
    else {
        /*will never came this else*/
    }

    if (connect_flag) {
        terminal_speak_limit_timeout_set(p_tmnl_node);
    }

    if (!connect_flag && p_tmnl_node != NULL) {
        host_timer_stop( &p_tmnl_node->tmnl_dev.spk_timeout);
    }

    /*report*/
    upper_cmpt_report_mic_state(mic_status, 
            p_tmnl_node->tmnl_dev.address.addr);

    micNode.micState = mic_status;
    micNode.node = p_tmnl_node;
    micNode.setCount = 1U;
    micNode.timeTick = get_current_time();
    Terminal_postMicFiFo(&l_micQueue, micNode);

    /* log information */
    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_DEBUG,
                "[ Post Mic(0x%04x) state(%d) Setting"
                " Command to micQueue(0x%x)]",
                p_tmnl_node->tmnl_dev.address.addr,
                mic_status,
                &l_micQueue);
}
/*********************************************************
*writer:YasirLiang
*Date:2015/11/4
*Name:terminal_mic_speak_limit_time_manager_event
*Param:
*	none
*Retern Value:
*	-1,Err;
*	0,nomal.
*state:管理系统终端的发言时长，到时断开连接，
*	单位是分钟;超时检查从讨论开始; 终端限时发言的系统超时管理机制的接口函数
*Change Date:26-4-2016
*Context:与函数terminal_mic_status_set_callback(设置超时时间)共同实现发言的超时机制
*注:在协议下的处理方式是不用调用此函数，见函数terminal_key_discuccess的协议
*注释部分(// 见协议命令bit3-bit0=1000:关闭麦克风)
***********************************************************/ 
int terminal_mic_speak_limit_time_manager_event( void )
{
	assert( dev_terminal_list_guard );
	if( dev_terminal_list_guard == NULL )
		return -1;

	/*查看会讨单元链表，并检查超时*/
	tmnl_pdblist  tmnl_node = dev_terminal_list_guard->next;
	for( ; tmnl_node != dev_terminal_list_guard; tmnl_node = tmnl_node->next )
	{
		if( (tmnl_node != NULL) && host_timer_timeout(&tmnl_node->tmnl_dev.spk_timeout) )
		{// timer out
			trans_model_unit_disconnect( tmnl_node->tmnl_dev.entity_id, tmnl_node );
			terminal_speak_track(tmnl_node->tmnl_dev.address.addr, false );
		}
	}

	return 0;
}

/* 设置终端开始讨论的状态*/
int terminal_start_discuss( bool mic_flag )
{
	thost_system_set set_sys; // 系统配置文件的格式
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));
	
	assert( dev_terminal_list_guard );
	if( dev_terminal_list_guard == NULL )
		return -1;
	
	tmnl_pdblist  tmnl_node = dev_terminal_list_guard->next;
	if( !mic_flag ) // 关闭所有麦克风
	{	
		/*关闭所有麦克风*/
		for( ; tmnl_node != dev_terminal_list_guard; tmnl_node = tmnl_node->next )
		{
			if( tmnl_node->tmnl_dev.tmnl_status.is_rgst && (tmnl_node->tmnl_dev.tmnl_status.mic_state != MIC_COLSE_STATUS) ) // 断开注册连接的终端
			{
				if (0 == trans_model_unit_disconnect( tmnl_node->tmnl_dev.entity_id, tmnl_node ))
					terminal_speak_track(tmnl_node->tmnl_dev.address.addr, false );
			}
		}
	}
	
	int i = 0;
	for( i = 0; i < TMNL_TYPE_NUM; i++ )
	{
		gtmnl_state_opt[i].auto_close = set_sys.auto_close?1:0;
		gtmnl_state_opt[i].keydown = 0;
		gtmnl_state_opt[i].keyup = 0;
		gtmnl_state_opt[i].MicClose = mic_flag?1:0;
		gtmnl_state_opt[i].sys = TMNL_SYS_STA_DISC; // 讨论模式
	} 

	/* 设置终端状态*/
	if( APPLY_MODE == set_sys.discuss_mode )
	{
		terminal_state_set_base_type( BRDCST_MEM |BRDCST_VIP|BRDCST_CHM, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);	// 根据终端类型设置终端的状态
		terminal_lcd_display_num_send( BRDCST_MEM |BRDCST_VIP|BRDCST_CHM, LCD_OPTION_CLEAR, glcd_num );// 发送lcd显示屏号
		terminal_chairman_apply_type_set( BRDCST_EXE );
	}
	else
	{
		terminal_state_set_base_type( BRDCST_MEM |BRDCST_VIP|BRDCST_CHM|BRDCST_EXE,gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);	// 根据终端类型设置终端的状态
		terminal_lcd_display_num_send( BRDCST_MEM |BRDCST_VIP|BRDCST_CHM|BRDCST_EXE, LCD_OPTION_CLEAR, glcd_num );// 发送lcd显示屏号
	}

	/*设置终端指示灯*/
	terminal_led_set_save( BRDCST_ALL, TLED_KEY1, TLED_OFF );
	terminal_led_set_save( BRDCST_ALL, TLED_KEY2, TLED_OFF );
	terminal_led_set_save( BRDCST_ALL, TLED_KEY3, TLED_OFF );
	terminal_led_set_save( BRDCST_ALL, TLED_KEY4, TLED_OFF );
	terminal_led_set_save( BRDCST_ALL, TLED_KEY5, TLED_OFF );
	fterminal_led_set_send( BRDCST_ALL );

	terminal_main_state_send( 0, NULL, 0 );

	return 0;
}

/*依据类型设置终端的状态*/
void terminal_state_set_base_type( uint16_t addr, tmnl_state_set tmnl_state )
{
	tmnl_pdblist tmp = found_terminal_dblist_node_by_addr( addr );
	if( tmp == NULL )
	{
		terminal_state_set( tmnl_state, addr, BRDCST_1722_ALL );
	}
	else
	{
		terminal_state_set( tmnl_state, addr, tmp->tmnl_dev.entity_id );
	}
}

/*主席申请类型终端状态设置*/
void terminal_chairman_apply_type_set( uint16_t addr )
{
	if( addr == 0xffff )
		return;
	
	gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE].keydown = 0x0e; // 2 3 4键按下有效
	gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE].keyup = 0;
	gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE].sys = TMNL_SYS_STA_DISC;

	terminal_state_set_base_type( addr, gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE] );
	terminal_lcd_display_num_send( addr, LCD_OPTION_DISPLAY, CHM_APPROVE_APPLY_INTERFACE );
}

void terminal_chairman_apply_type_clear( uint16_t addr )
{
	if( addr == 0xffff )
		return;
	
	gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE].keydown = 0; 
	gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE].keyup = 0;
	gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE].sys = TMNL_SYS_STA_DISC;

	terminal_state_set_base_type( addr, gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE] );
	terminal_lcd_display_num_send( addr, LCD_OPTION_CLEAR, CHM_APPROVE_APPLY_INTERFACE );
}

/*保存led指示灯的状态*/
bool terminal_led_set_save( uint16_t addr, uint8_t led_id, uint8_t  led_state )
{
	uint8_t byte_num,shifting;

	if( 0xFFFF == addr )
	{
		return false;
	}
	
	if(led_id>=TLED_MAX_NUM)
	{
		return false;
	}

	byte_num = led_id/4;
	shifting = (led_id%4)*2;
	gled_buf[byte_num]&=(~(0x03<<shifting));
	gled_buf[byte_num]|=(led_state<<shifting);

	return true;
}

/*发送led指示灯的状态*/ 
void fterminal_led_set_send( uint16_t addr )
{
	ttmnl_led_lamp led_lamp;
	led_lamp.data_low = gled_buf[0];
	led_lamp.data_high = gled_buf[1];

	tmnl_pdblist tmp = found_terminal_dblist_node_by_addr( addr );
	if( tmp == NULL )
	{
		terminal_set_indicator_lamp( led_lamp, addr, BRDCST_1722_ALL );
	}
	else
	{
		terminal_set_indicator_lamp( led_lamp, addr, tmp->tmnl_dev.entity_id  );
	}
}

#ifdef MIC_PRIOR_MANEGER_ENABLE
int terminal_upper_computer_speak_process(tcmpt_data_mic_switch mic_flag) {
    uint16_t spkAddr;
    uint8_t setMicState;
    ttmnl_discuss_mode disMode;
    tmnl_pdblist spkNode;
    thost_system_set setSys;
    uint8_t tType; /* terminal type */
    uint8_t user;
    TEReqQePrior prior;
    bool rightType, isConnect, willCancel;
    uint64_t entityId;
    TComQueue *pQueue;

    /* set parameter */
    spkAddr = (uint16_t)mic_flag.addr.low_addr;
    spkAddr |= (uint16_t)(mic_flag.addr.high_addr << 8);
    setMicState = mic_flag.switch_flag;
    disMode = gdisc_flags.edis_mode;
    memcpy(&setSys, &gset_sys, sizeof(thost_system_set));

    spkNode = found_terminal_dblist_node_by_addr(spkAddr);
    if (spkNode == (tmnl_pdblist)0) {
        gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_ERROR,
                "[Not found terminal by address(0x%04x)]",
                spkAddr);
        
        return -1;
    }
    
    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_DEBUG,
                "[upper computer speak Found"
                " terminal by address(0x%04x), setMicState = %d]",
                spkAddr, setMicState);

    if (!spkNode->tmnl_dev.tmnl_status.is_rgst) {
        gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_DEBUG,
                "[ upper computer speak terminal "
                " by address(0x%04x) Failed: not registing ]",
                spkAddr, setMicState);
        
        return -1;
    }

    entityId = spkNode->tmnl_dev.entity_id;
    tType = spkNode->tmnl_dev.address.tmn_type;
    
    rightType = (bool)1;
    if (tType == TMNL_TYPE_COMMON_RPRST) {
        prior = COMMON_PRIOR;
        pQueue = &l_FSpkQueue;
    }
    else if (tType == TMNL_TYPE_VIP) {
        prior = VIP_PRIOR;
        pQueue = &l_vipFSpkQueue;
    }
    else if ((tType == TMNL_TYPE_CHM_EXCUTE)
                  || (tType == TMNL_TYPE_CHM_COMMON))
    {
        prior = CHAIRMAN_PRIOR;
        pQueue = &l_chairmanFSpkQueue;
    }
    else {
        /* error type */
        rightType = (bool)0;
    }

    /* if no task in the queue(connector or disconnector)
        process, otherwise discard this key command */
    if ((!rightType)
          || (Terminal_hasTask(prior, spkAddr))
          || (terminal_firstSpeakQueueHasTask(pQueue, spkAddr)))
    {
        /* cancel task? */
        willCancel = (bool)0;
        
        if (rightType) {
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_ERROR,
                "[2.Terminal(0x%04x) has Task in"
                "  prior(%d) Queue ]", spkAddr, prior);

            isConnect = trans_model_unit_is_connected(entityId);
            
            if (setMicState) {
                /* canecl disconnect task */
                user = DISCONNECTOR;

                if (isConnect) {
                    willCancel = (bool)1;
                }
            }
            else {
                /* canecl connect task */
                user = CONNECTOR;
                
                if (!isConnect) {
                    willCancel = (bool)1;
                }
            }

            if (willCancel) {
                if (Terminal_cancelTask(user, prior, spkAddr)) {
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_ERROR,
                        "[2.Terminal(0x%04x) cancel user(%d) Task"
                        "  prior(%d) Queue success ]", spkAddr, user, prior);
                }
                else {
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_ERROR,
                        "[2.Terminal(0x%04x) cancel user(%d) Task"
                        "  prior(%d) Queue Error ]", spkAddr, user, prior);
                }

                /* cancel task, zero meaning cancel */
                if (Terminal_changeFirstSpkTaskPer(spkAddr,
                        pQueue, &l_FSpkLocker, 0U))
                {
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_ERROR,
                        "[2.Terminal(0x%04x) cancel connections Task"
                        "  prior(%d) Queue success ]", spkAddr, prior);
                }
                else {
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_ERROR,
                        "[2.Terminal(0x%04x) cancel connections Task"
                        "  prior(%d) Queue Error ]", spkAddr, prior);
                }
            }
        }
        else {
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_ERROR,
                "[2. No Right Type ]");
        }

        return false;
    }
    
    switch (tType) {
        /* common preset terminal frequency operation */
        case TMNL_TYPE_COMMON_RPRST: {
            terminal_common_speak(spkNode, setMicState);
            break;
        }
        case TMNL_TYPE_VIP: {
            terminal_vip_speak(spkNode, setMicState);
            break;
        }
        case TMNL_TYPE_CHM_EXCUTE:
        case TMNL_TYPE_CHM_COMMON: {
            terminal_chairman_speak(spkNode, setMicState);
            break;
        }
        default: {
            /* error type of terminal */
            break;
        }
    }   

    return 0;
}
#else
int terminal_upper_computer_speak_process(tcmpt_data_mic_switch mic_flag)
{
	uint16_t addr = (((uint16_t)(mic_flag.addr.low_addr << 0)) | ((uint16_t)(mic_flag.addr.high_addr << 8)));
	uint8_t mic_state_set = mic_flag.switch_flag;
	ttmnl_discuss_mode dis_mode = gdisc_flags.edis_mode;
	tmnl_pdblist speak_node = NULL;
	uint8_t limit_time = 0;
	bool found_node = false;
	thost_system_set set_sys; // 系统配置文件的格式
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));
	
	limit_time = set_sys.spk_limtime;
	
	assert( dev_terminal_list_guard );
	if( dev_terminal_list_guard == NULL )
		return -1;
	
	terminal_pro_debug( " speak addr = %04x, discuccess mode = %d", addr, dis_mode );
	for( speak_node = dev_terminal_list_guard->next; speak_node != dev_terminal_list_guard; speak_node = speak_node->next )
	{
		if( speak_node->tmnl_dev.address.addr == addr && speak_node->tmnl_dev.tmnl_status.is_rgst )
		{
			found_node = true;
			break;
		}
	}

	if( found_node )
	{		
		if( dis_mode == PPT_MODE ||\
			(speak_node->tmnl_dev.address.tmn_type == TMNL_TYPE_VIP) ||\
			(speak_node->tmnl_dev.address.tmn_type == TMNL_TYPE_CHM_COMMON)||\
			(speak_node->tmnl_dev.address.tmn_type == TMNL_TYPE_CHM_EXCUTE))
		{
			if( mic_state_set )
			{
				if (trans_model_unit_is_connected(speak_node->tmnl_dev.entity_id))
				{// already connected
					if (speak_node->tmnl_dev.tmnl_status.mic_state != MIC_OPEN_STATUS)
						speak_node->tmnl_dev.tmnl_status.mic_state = MIC_OPEN_STATUS;					
				}
				else
				{
					/*主席在发言上限里*/
					uint16_t speak_num = terminal_speak_num_count();
					int ret_cnnt = -1;
					if (speak_num < gdisc_flags.limit_num)
					{
						ret_cnnt = trans_model_unit_connect(speak_node->tmnl_dev.entity_id, speak_node);
					}
					
					if ((speak_num < gdisc_flags.limit_num) && (ret_cnnt == 0))
					{
						terminal_speak_track(speak_node->tmnl_dev.address.addr, true );
					}
					else if ((speak_num < gdisc_flags.limit_num) ||ret_cnnt != -2)
					{//has timeout for operation transmit ouput channel
						/*
						  *1\断开连接时间最长的
						  *2\超时后连接
						  */
						tmnl_pdblist disconnect_lnode = NULL;
						uint64_t id = 0;
						int ret = -1;
						bool if_common = false;

						ret = Ctrans_disLongest(&id, &disconnect_lnode);
						if (0 == ret)
						{
							if (disconnect_lnode != NULL && \
								disconnect_lnode->tmnl_dev.address.tmn_type == TMNL_TYPE_COMMON_RPRST)
							{
								if_common = true;
							}
							else if (disconnect_lnode == NULL && id != 0)
							{
								disconnect_lnode = found_terminal_dblist_node_by_endtity_id(id);
								if (disconnect_lnode != NULL && disconnect_lnode->tmnl_dev.address.tmn_type == TMNL_TYPE_COMMON_RPRST)
								{
									if_common = true;
								}
							}

							if (if_common)
							{
								if (dis_mode == FIFO_MODE)
									addr_queue_delect_by_value(gdisc_flags.speak_addr_list, &gdisc_flags.speak_limit_num, disconnect_lnode->tmnl_dev.address.addr);
								else if (dis_mode == LIMIT_MODE)
									gdisc_flags.speak_limit_num--;
							}

							terminal_over_time_speak_node_set(speak_node);
							if (NULL != disconnect_lnode)
								DEBUG_INFO("tmnl type = %d disconnect add = 0x%02x  spking num = %d====================1", 
									disconnect_lnode->tmnl_dev.address.tmn_type, 
									disconnect_lnode->tmnl_dev.address.addr, 
									gdisc_flags.speak_limit_num);
						}
					}
					else if (speak_num >= gdisc_flags.limit_num)
					{
						/*
						  *1\断开连接时间最长的
						  *2\超时后连接
						  */
						tmnl_pdblist disconnect_lnode = NULL;
						uint64_t id = 0;
						int ret = -1;
						bool if_common = false;

						ret = Ctrans_disLongest(&id, &disconnect_lnode);
						if (0 == ret)
						{
							if (disconnect_lnode != NULL && \
								disconnect_lnode->tmnl_dev.address.tmn_type == TMNL_TYPE_COMMON_RPRST)
							{
								if_common = true;
							}
							else if (disconnect_lnode == NULL && id != 0)
							{
								disconnect_lnode = found_terminal_dblist_node_by_endtity_id(id);
								if (disconnect_lnode != NULL && disconnect_lnode->tmnl_dev.address.tmn_type == TMNL_TYPE_COMMON_RPRST)
								{
									if_common = true;
								}
							}

							if (if_common)
							{
								if (dis_mode == FIFO_MODE)
									addr_queue_delect_by_value(gdisc_flags.speak_addr_list, &gdisc_flags.speak_limit_num, disconnect_lnode->tmnl_dev.address.addr);
								else if (dis_mode == LIMIT_MODE)
									gdisc_flags.speak_limit_num--;
							}

							terminal_over_time_speak_node_set(speak_node);
							if (NULL != disconnect_lnode)
								DEBUG_INFO("tmnl type = %d disconnect add = 0x%02x  spking num = %d====================1", 
									disconnect_lnode->tmnl_dev.address.tmn_type, 
									disconnect_lnode->tmnl_dev.address.addr, 
									gdisc_flags.speak_limit_num);
						}
					}
				}
			}
			else
			{
				if (trans_model_unit_is_connected(speak_node->tmnl_dev.entity_id))
				{
					if (0 == trans_model_unit_disconnect( speak_node->tmnl_dev.entity_id, speak_node ))
					{
						terminal_speak_track(speak_node->tmnl_dev.address.addr, false );

						/*主席\vip控制限制模式下的普通终端的mic状态*/
						if (dis_mode == LIMIT_MODE)
						{
							terminal_chman_vip_control_common_mic();
						}
					}
				}
			}
		}
		else
		{
			switch( dis_mode )
			{
				case LIMIT_MODE:
				{
					terminal_limit_disccuss_mode_cmpt_pro( mic_state_set, limit_time, speak_node );
					break;
				}
				case FIFO_MODE:
				{
					terminal_fifo_disccuss_mode_cmpt_pro( mic_state_set, limit_time, speak_node );
					break;
				}
				case APPLY_MODE:
				{
					terminal_apply_disccuss_mode_cmpt_pro( mic_state_set, limit_time, speak_node );
					break;
				}
				default:
				{
					terminal_pro_debug( " out of discuss mode bound!" );
					break;
				}
			}
		}
	}
	else if( !found_node)
	{
		return -1;
	}
	
	return 0;
}
#endif /* MIC_PRIOR_MANEGER_ENABLE */

//  file STSTEM_SET_STUTUS_PROFILE must be close,before use this function
bool terminal_read_profile_file( thost_system_set *set_sys )
{
	FILE* fd = NULL;
	thost_system_set tmp_set_sys;
	int ret = false;
	
	fd = Fopen( STSTEM_SET_STUTUS_PROFILE, "rb" ); // 只读读出数据
	if( NULL == fd )
	{
		terminal_pro_debug( "mian state send ->open files %s Err!",  STSTEM_SET_STUTUS_PROFILE );
		return false;
	}

	ret = true;
	memset( &tmp_set_sys, 0, sizeof( thost_system_set));
	if( profile_system_file_read( fd, &tmp_set_sys ) == -1)
	{
		terminal_pro_debug( "Read profile system Err!" );
		ret = false;
	}

	if( ret )
	{
		assert( set_sys );
		if( set_sys != NULL )
		{
			memcpy( set_sys, &tmp_set_sys, sizeof(thost_system_set));
		}
		else
		{
			ret = false;
		}	
	}
			
	Fclose( fd );
	return ret;
}

void terminal_free_disccuss_mode_cmpt_pro( uint8_t mic_flag, uint8_t limit_time, tmnl_pdblist speak_node )
{
	if( speak_node == NULL )
	{
		return;
	}
	
	if( mic_flag ) // 打开麦克风
	{
		if( gdisc_flags.speak_limit_num < FREE_MODE_SPEAK_MAX )
		{
			if( -1 == trans_model_unit_connect( speak_node->tmnl_dev.entity_id, speak_node ))
					terminal_mic_status_set_callback( false, speak_node );
			
			terminal_speak_track(speak_node->tmnl_dev.address.addr, true );
		}
	}
	else
	{
		trans_model_unit_disconnect( speak_node->tmnl_dev.entity_id, speak_node );
		terminal_speak_track(speak_node->tmnl_dev.address.addr, false );
	}

	terminal_main_state_send( 0, NULL, 0 );
}

bool terminal_limit_disccuss_mode_cmpt_pro( uint8_t mic_flag, uint8_t limit_time, tmnl_pdblist speak_node )
{
	bool ret = false;
	uint16_t addr = speak_node->tmnl_dev.address.addr;
	uint16_t current_addr = 0;
	uint8_t cc_state = 0;
	tmnl_pdblist first_apply = NULL; // 首位申请发言
	
	if( speak_node == NULL )
	{
		return ret;
	}

	if( mic_flag ) // 打开麦克风
	{
		uint16_t speak_num = terminal_speak_num_count();
		if( speak_node->tmnl_dev.tmnl_status.mic_state != MIC_COLSE_STATUS )
		{
			ret = true;
		}
		else if( gdisc_flags.speak_limit_num < gdisc_flags.limit_num \
			&& speak_num < gdisc_flags.limit_num) // 打开麦克风
		{
			if(0 == trans_model_unit_connect( speak_node->tmnl_dev.entity_id, speak_node ))
			{
				terminal_speak_track(speak_node->tmnl_dev.address.addr, true );
				gdisc_flags.speak_limit_num++;
				ret = true;
			}
		}
		else if( gdisc_flags.apply_num < gdisc_flags.apply_limit ) // 申请发言
		{
			uint8_t state = MIC_OTHER_APPLY_STATUS;
			if( 0 == gdisc_flags.apply_num )
			{
				gdisc_flags.currect_first_index = 0;
				state = MIC_FIRST_APPLY_STATUS;
			}
			gdisc_flags.apply_addr_list[gdisc_flags.apply_num] = speak_node->tmnl_dev.address.addr;
			gdisc_flags.apply_num++;

			terminal_mic_state_set( state, speak_node->tmnl_dev.address.addr, speak_node->tmnl_dev.entity_id, true, speak_node );
			terminal_main_state_send( 0, NULL, 0 );
			ret = true;
		}
	}
	else
	{
		if (0 == trans_model_unit_disconnect( speak_node->tmnl_dev.entity_id, speak_node ))
		{
			terminal_speak_track(speak_node->tmnl_dev.address.addr, false );
		}
		
		current_addr = gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index];
		cc_state = speak_node->tmnl_dev.tmnl_status.mic_state;
		if( cc_state == MIC_FIRST_APPLY_STATUS || cc_state == MIC_OTHER_APPLY_STATUS )
		{
			addr_queue_delect_by_value( gdisc_flags.apply_addr_list, &gdisc_flags.apply_num, addr );
			terminal_mic_state_set(MIC_COLSE_STATUS, speak_node->tmnl_dev.address.addr, speak_node->tmnl_dev.entity_id, true, speak_node);
			if( gdisc_flags.apply_num > 0 && current_addr == addr )// 置下一个申请为首位申请状态
			{
				gdisc_flags.currect_first_index %= gdisc_flags.apply_num;
				first_apply = found_terminal_dblist_node_by_addr( gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index]);
				if( first_apply != NULL )
				{
					terminal_mic_state_set( MIC_FIRST_APPLY_STATUS, first_apply->tmnl_dev.address.addr, first_apply->tmnl_dev.entity_id, true, first_apply );
				}
			}

			terminal_main_state_send( 0, NULL, 0 );
			ret = true;
		}
		else if( cc_state == MIC_OPEN_STATUS )
		{
			if( gdisc_flags.speak_limit_num > 0 )
			{
				gdisc_flags.speak_limit_num--;
			}

			if(gdisc_flags.speak_limit_num < gdisc_flags.limit_num && gdisc_flags.apply_num > 0 )// 结束发言,并开始下一个申请终端的发言
			{
				if( addr_queue_delete_by_index( gdisc_flags.apply_addr_list, &gdisc_flags.apply_num, gdisc_flags.currect_first_index) )// 开启下一个申请话筒
				{
					tmnl_pdblist first_speak = found_terminal_dblist_node_by_addr( current_addr );
					if( first_speak != NULL )
					{
						if (0 == trans_model_unit_connect( first_speak->tmnl_dev.entity_id, first_speak ))
						{// connect success
							gdisc_flags.speak_limit_num++;
                                                        terminal_speak_track(first_speak->tmnl_dev.address.addr, true);
							terminal_apply_list_first_speak();
						}
						else
						{
							/*
							  *当前的mic断开成功后会留出一个通道
							  */
							terminal_over_time_speak_node_set(first_speak);
						}
					}
					else
					{
						terminal_pro_debug( " no such tmnl dblist node!");
					}
				}
				else
				{
					gdisc_flags.currect_first_index = 0;
				}
			}

			terminal_main_state_send( 0, NULL, 0 );
			ret = true;
		}
	}

	return ret;
}

bool terminal_fifo_disccuss_mode_cmpt_pro( uint8_t mic_flag, uint8_t limit_time, tmnl_pdblist speak_node )
{
	bool ret = false;
	uint16_t addr = speak_node->tmnl_dev.address.addr;
	int dis_ret = -1;
	uint8_t speak_limit_num = gdisc_flags.speak_limit_num;
	uint16_t speak_num = terminal_speak_num_count();

	if( speak_node == NULL )
	{
		terminal_pro_debug( " NULL speak node!" );
		return false;
	}

	if( mic_flag ) // 打开话筒
	{
		if( addr_queue_find_by_value( gdisc_flags.speak_addr_list, speak_limit_num, addr, NULL))
		{
			if (trans_model_unit_is_connected(speak_node->tmnl_dev.entity_id))
			{
				ret = true;
			}
			else
			{
				if (0 == trans_model_unit_connect( speak_node->tmnl_dev.entity_id, speak_node ))
				{
					terminal_speak_track(speak_node->tmnl_dev.address.addr, true );
					ret = true;
				}
			}
		}
		else if( speak_limit_num < gdisc_flags.limit_num \
			&& speak_num < gdisc_flags.limit_num)
		{
			if ( 0 == trans_model_unit_connect( speak_node->tmnl_dev.entity_id, speak_node ))
			{
				terminal_speak_track(speak_node->tmnl_dev.address.addr, true );
				gdisc_flags.speak_addr_list[speak_limit_num] = addr;
				gdisc_flags.speak_limit_num++;
				ret = true;
			}			
		}
		else // 发言人数大于或等于限制人数
		{
			if (!trans_model_unit_is_connected(speak_node->tmnl_dev.entity_id))
			{
				if( gdisc_flags.speak_addr_list[0] != 0xffff ) // 先进先出
				{
					tmnl_pdblist first_speak = found_terminal_dblist_node_by_addr( gdisc_flags.speak_addr_list[0] );
					if( first_speak != NULL )
					{
						if (trans_model_unit_is_connected(first_speak->tmnl_dev.entity_id))
						{
							dis_ret = trans_model_unit_disconnect( first_speak->tmnl_dev.entity_id, first_speak );
							if (0 == dis_ret)
							{
								terminal_speak_track(first_speak->tmnl_dev.address.addr, false );
								addr_queue_delete_by_index( gdisc_flags.speak_addr_list, &gdisc_flags.speak_limit_num, 0 );// 首位发言删除
								terminal_over_time_speak_node_set(speak_node);
							}
						}
					}
					else
					{
						terminal_pro_debug( "fifo not found tmnl list node!");
					}			
				}
			}
			else
			{
				if (speak_node->tmnl_dev.tmnl_status.mic_state != MIC_OPEN_STATUS)
					speak_node->tmnl_dev.tmnl_status.mic_state = MIC_OPEN_STATUS;
			}
		}
	}
	else
	{
		if ( 0 == trans_model_unit_disconnect( speak_node->tmnl_dev.entity_id, speak_node ))
		{
			addr_queue_delect_by_value( gdisc_flags.speak_addr_list, &gdisc_flags.speak_limit_num, speak_node->tmnl_dev.address.addr );
			terminal_speak_track(speak_node->tmnl_dev.address.addr, false );
			ret = true;
		}
	}

	return ret;
}

bool terminal_apply_disccuss_mode_cmpt_pro( uint8_t mic_flag, uint8_t limit_time, tmnl_pdblist speak_node )
{
	if( speak_node == NULL )
	{
		return false;
	}

	bool ret = false;
	uint16_t addr = speak_node->tmnl_dev.address.addr;
	uint16_t current_addr = 0;
	tmnl_pdblist first_apply = NULL;

	if( mic_flag ) // 申请发言,加地址入申请列表
	{
		if(  gdisc_flags.apply_num < gdisc_flags.apply_limit )
		{
			uint8_t state = MIC_OTHER_APPLY_STATUS;
			if( 0 == gdisc_flags.apply_num )
			{
				gdisc_flags.currect_first_index = 0;
				state = MIC_FIRST_APPLY_STATUS;
			}
			gdisc_flags.apply_addr_list[gdisc_flags.apply_num] = speak_node->tmnl_dev.address.addr;
			gdisc_flags.apply_num++;

			terminal_mic_state_set( state, speak_node->tmnl_dev.address.addr, speak_node->tmnl_dev.entity_id, true, speak_node );
			terminal_main_state_send( 0, NULL, 0 );
			ret = true;
		}
	}
	else // 取消申请发言
	{
		current_addr = gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index];
		if(addr_queue_delect_by_value( gdisc_flags.apply_addr_list, &gdisc_flags.apply_num, addr ))
		{
			terminal_mic_state_set( MIC_COLSE_STATUS, speak_node->tmnl_dev.address.addr, speak_node->tmnl_dev.entity_id, true, speak_node );// 上报mic状态
			if( gdisc_flags.apply_num > 0 && current_addr == addr )// 置下一个申请为首位申请状态
			{
				gdisc_flags.currect_first_index %= gdisc_flags.apply_num;
				first_apply = found_terminal_dblist_node_by_addr( gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index] );
				if( first_apply != NULL )
				{
					terminal_mic_state_set( MIC_FIRST_APPLY_STATUS, speak_node->tmnl_dev.address.addr, speak_node->tmnl_dev.entity_id, true, speak_node );
				}
			}
			
			terminal_main_state_send( 0, NULL, 0 );
		}	
		else
		{// 关闭话筒
			if (trans_model_unit_is_connected(speak_node->tmnl_dev.entity_id))
			{
				trans_model_unit_disconnect( speak_node->tmnl_dev.entity_id, speak_node );
				terminal_speak_track(speak_node->tmnl_dev.address.addr, false );
			}
		}
		
		ret = true;
	}

	return ret;
}

bool addr_queue_delete_by_index( uint16_t *addr_queue, uint8_t *queue_len, uint8_t index )
{
	uint8_t i;
	
	if((NULL==addr_queue)||(NULL==queue_len))
	{
		return false;
	}
	if((index >= *queue_len)||(0==*queue_len))
	{
		return false;
	}

	for(i = index; i<(*queue_len-1); i++)
	{
		*(addr_queue+i)=*(addr_queue+i+1);
	}
	
	*(addr_queue+*queue_len-1)=0xFFFF;
	(*queue_len)--;

	return true;
}

bool addr_queue_delect_by_value( uint16_t *addr_queue, uint8_t *queue_len, uint16_t value)
{
	uint16_t   i;
	uint16_t  index;
	if((NULL==addr_queue)||(NULL==queue_len))
	{
		return false;
	}
	
	if( 0 == *queue_len )
	{
		return false;
	}
	
	for(i=0; i<*queue_len; i++)
	{
		if(*(addr_queue+i)==value)
		{
			break;
		}
	}
	
	index = i;
	if(index<*queue_len)
	{
		for(i=index; i<(*queue_len-1); i++)
		{
			*(addr_queue+i)=*(addr_queue+i+1);
		}
		
		*(addr_queue+*queue_len-1)=0xFFFF;
		(*queue_len)--;
		
		return true;
	}
	
	return false;
}

bool addr_queue_find_by_value( uint16_t *addr_queue, uint8_t queue_len, uint16_t value, uint8_t *index)
{
	uint8_t i;
	
	if((NULL==addr_queue)||(0==queue_len))
	{
		return false;
	}
	
	for(i=0; i<queue_len; i++)
	{
		if(value == *(addr_queue+i))
		{
			if(index != NULL)
			{
				*index = i;
			}
			
			return true;
		}
	}
	
	return false;
}

tmnl_pdblist found_terminal_dblist_node_by_endtity_id(const uint64_t tarker_id)
{
	assert( dev_terminal_list_guard );
	if( dev_terminal_list_guard == NULL )
		return NULL;
	
	tmnl_pdblist tmp_node = dev_terminal_list_guard->next;
	for( ; tmp_node != dev_terminal_list_guard; tmp_node = tmp_node->next )
	{
		if (tmp_node->tmnl_dev.entity_id == tarker_id)
			return tmp_node;
	}

	return NULL;
}

tmnl_pdblist found_terminal_dblist_node_by_addr( uint16_t addr )
{
	assert( dev_terminal_list_guard );
	if( dev_terminal_list_guard == NULL )
		return NULL;
	
	tmnl_pdblist tmp_node = dev_terminal_list_guard->next;
	for( ; tmp_node != dev_terminal_list_guard; tmp_node = tmp_node->next )
	{
		if( tmp_node->tmnl_dev.address.addr == addr )
			return tmp_node;
	}

	return NULL;
}

void terminal_select_apply( uint16_t addr ) // 使选择的申请人是首位申请人
{
	tmnl_pdblist apply_first = NULL; 
	uint8_t apply_index = MAX_LIMIT_APPLY_NUM;
	int i = 0;
	
	if( addr != gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index]) // 不是首位申请
	{
		apply_first = found_terminal_dblist_node_by_addr( gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index] );
		terminal_mic_state_set( MIC_OTHER_APPLY_STATUS, apply_first->tmnl_dev.address.addr, apply_first->tmnl_dev.entity_id, true, apply_first );

		for( i = 0; i < gdisc_flags.apply_num; i++ )
		{
			if( addr == gdisc_flags.apply_addr_list[i])
			{
				apply_index = i;
				break;
			}
		}

		if( apply_index < gdisc_flags.apply_num )
		{
			apply_first = found_terminal_dblist_node_by_addr( addr );
			terminal_mic_state_set( MIC_FIRST_APPLY_STATUS, apply_first->tmnl_dev.address.addr, apply_first->tmnl_dev.entity_id, true, apply_first );
		}
	}
}

typedef struct TMicStateSetPro
{
	bool running;
	tmnl_pdblist micNode;
        bool sendUpperCmpt;
}TMicStateSetPro;

TMicStateSetPro gmicSetPro;
void terminal_after_time_mic_state_node_set( tmnl_pdblist speak_node, uint8_t mic_status, bool send_upper_cmpt)
{
	if ((!gmicSetPro.running) && (NULL != speak_node) )
	{
		gmicSetPro.micNode = speak_node;
                gmicSetPro.micNode->tmnl_dev.tmnl_status.mic_state = mic_status;
		gmicSetPro.running = true; 
                gmicSetPro.sendUpperCmpt = send_upper_cmpt;
		over_time_set( MIC_SET_AFTER, 200);
	}
}

void terminal_after_time_mic_state_pro(void)
{
        if ((gmicSetPro.running) && over_time_listen(MIC_SET_AFTER))
        {
        	if( gmicSetPro.micNode != NULL )
        	{
        	        terminal_set_mic_status( gmicSetPro.micNode->tmnl_dev.tmnl_status.mic_state,
                        gmicSetPro.micNode->tmnl_dev.address.addr, 
                        gmicSetPro.micNode->tmnl_dev.entity_id);
        	}

                if (gmicSetPro.sendUpperCmpt)
                {
                        upper_cmpt_report_mic_state( gmicSetPro.micNode->tmnl_dev.tmnl_status.mic_state,
                                                                gmicSetPro.micNode->tmnl_dev.address.addr);
                }
                
                gmicSetPro.running = false;
                over_time_stop(MIC_SET_AFTER);
        }
}

bool terminal_examine_apply(enum_apply_pro applyOpt) {
    bool ret = false;
    uint16_t addr = 0;
    thost_system_set set_sys;
    ttmnl_discuss_mode disMode;
    tmnl_pdblist applyFirst = (tmnl_pdblist)0;
    uint8_t cFirstIndex;

    disMode = gdisc_flags.edis_mode;
    memcpy(&set_sys, &gset_sys, sizeof(thost_system_set));

    if ((disMode != APPLY_MODE)
        && (disMode != LIMIT_MODE))
    {
        return false;
    }

    switch (applyOpt) {
        case REFUSE_APPLY: {
            cFirstIndex = gdisc_flags.currect_first_index;
            addr = gdisc_flags.apply_addr_list[cFirstIndex];
            
            if (addr_queue_delect_by_value(gdisc_flags.apply_addr_list,
                                    &gdisc_flags.apply_num, addr))
            {
                applyFirst = found_terminal_dblist_node_by_addr(addr);
                if (applyFirst != (tmnl_pdblist)0) {
                    terminal_mic_state_set(MIC_COLSE_STATUS,
                        applyFirst->tmnl_dev.address.addr,
                        applyFirst->tmnl_dev.entity_id,
                        true, applyFirst);
                    
                    if (gdisc_flags.apply_num > 0) {
                        gdisc_flags.currect_first_index %=
                            gdisc_flags.apply_num;
                        cFirstIndex = gdisc_flags.currect_first_index;
                        
                        applyFirst = found_terminal_dblist_node_by_addr(
                                gdisc_flags.apply_addr_list[cFirstIndex]);
                        if (applyFirst != (tmnl_pdblist)0) {
                            terminal_after_time_mic_state_node_set(applyFirst,
                                MIC_FIRST_APPLY_STATUS, true);
                        }
                    }
                    else {
                        gdisc_flags.currect_first_index = MAX_LIMIT_APPLY_NUM;
                    }
                }

                terminal_main_state_send(0, (void *)0, 0);
                ret = true;
            }
            else {
                terminal_pro_debug("delect apply add"
                    " = 0x%04x failed: no such address",
                    addr);
            }

            break;
        }
        case NEXT_APPLY: {
            if (gdisc_flags.apply_num > 1) {
                cFirstIndex = gdisc_flags.currect_first_index;
                addr = gdisc_flags.apply_addr_list[cFirstIndex];
                
                applyFirst = found_terminal_dblist_node_by_addr(addr);
                if (applyFirst != (tmnl_pdblist)0) {
                    terminal_mic_state_set(MIC_OTHER_APPLY_STATUS,
                        applyFirst->tmnl_dev.address.addr,
                        applyFirst->tmnl_dev.entity_id,
                        true, applyFirst );

                    gdisc_flags.currect_first_index++;
                    gdisc_flags.currect_first_index %= gdisc_flags.apply_num;
                    cFirstIndex = gdisc_flags.currect_first_index;
                    addr = gdisc_flags.apply_addr_list[cFirstIndex];
                    
                    applyFirst = found_terminal_dblist_node_by_addr(addr);
                    if (applyFirst != (tmnl_pdblist)0) {
                        terminal_after_time_mic_state_node_set(applyFirst,
                            MIC_FIRST_APPLY_STATUS, true);
                    }
                    else {
                        terminal_pro_debug("no found first apply node!");
                    }
                }
                else {
                    terminal_pro_debug("no found first apply node!");
                }

                terminal_main_state_send(0, (void *)0, 0);
                ret = true;
            }
            
            break;
        }
        case APPROVE_APPLY: {
            if (gdisc_flags.currect_first_index < gdisc_flags.apply_num) {
                cFirstIndex = gdisc_flags.currect_first_index;
                addr = gdisc_flags.apply_addr_list[cFirstIndex];
                applyFirst = found_terminal_dblist_node_by_addr(addr);
                if (applyFirst != (tmnl_pdblist)0) {
#ifdef MIC_PRIOR_MANEGER_ENABLE
#ifdef FIRST_SPEAK_QUEUE_ENABLE
                /* must set APPLY_CB_PERMISSION, because must set
                    conference apply parameter when
                    open micphone successfully */
                if (Terminal_firstSpkRequestConnect(applyFirst,
                        &l_FSpkQueue, &l_FSpkLocker,
                        MAX_CONNECT_FAILURE_TIMES,
                        COMMON_SPK_PERMISSION | APPLY_CB_PERMISSION))
                {
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_DEBUG,
                        "[ APPLY(1) Mode Speak Post First ]"
                        " speaking node(address = 0x%x)"
                        " to Queue Success ]",
                        applyFirst->tmnl_dev.address.addr);
                }
                else {
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_DEBUG,
                        "[ APPLY(1) Mode Speak Post First ]"
                        " speaking node(address = 0x%x)"
                        "to Queue Failed ]",
                        applyFirst->tmnl_dev.address.addr);
                }
#else  
                    /* request for connections */
                    if (Terminal_requestConnect(applyFirst, COMMON_PRIOR,
                            MAX_FAILURE_TIMES, COMMON_SPK_PERMISSION)) {
                        gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                            LOGGING_LEVEL_ERROR,
                            "[ Chairman APPROVE_APPLY(0x04x) Request "
                            "connections success:waiting for connection ]",
                            applyFirst->tmnl_dev.address.addr);
                    }
                    else {
                        gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                            LOGGING_LEVEL_ERROR,
                            "[ Chairman APPROVE_APPLY(0x04x) Request "
                            "connections Failed. ]",
                            applyFirst->tmnl_dev.address.addr);
                    }
#endif /* FIRST_SPEAK_QUEUE_ENABLE */                    
#else
                    if (0 == trans_model_unit_connect(applyFirst->tmnl_dev.entity_id,
                            applyFirst))
                    {
                        terminal_speak_track(
                            applyFirst->tmnl_dev.address.addr, true);
                        
                        addr_queue_delete_by_index(gdisc_flags.apply_addr_list,
                            &gdisc_flags.apply_num,
                            gdisc_flags.currect_first_index);
                        
                        if (gdisc_flags.apply_num > 0) {
                            gdisc_flags.currect_first_index %=
                                gdisc_flags.apply_num;
                            cFirstIndex = gdisc_flags.currect_first_index;
                            
                            applyFirst = found_terminal_dblist_node_by_addr(
                                gdisc_flags.apply_addr_list[cFirstIndex]);
                            if (applyFirst != (tmnl_pdblist)0) {
                                terminal_after_time_mic_state_node_set(
                                    applyFirst, MIC_FIRST_APPLY_STATUS, true);
                            }
                        }
                        else {
                            gdisc_flags.currect_first_index =
                                gdisc_flags.apply_limit;
                        }

                        terminal_main_state_send(0, (void *)0, 0);
                        ret = true;
                    }
#endif                    
                }
            }
            break;
        }
        default: {
            break;
        }
    }

    return ret;
}

void terminal_type_set(tcmpt_data_meeting_authority tmnl_type) {
    uint16_t addr;
    uint8_t tmnlType;
    uint8_t setType;
    int i;
    tmnl_pdblist tmnl_node = (tmnl_pdblist)0;

    addr = ((uint16_t)tmnl_type.addr.high_addr << 8)
                |((uint16_t)tmnl_type.addr.low_addr << 0);
    tmnl_node = found_terminal_dblist_node_by_addr(addr);
    if (tmnl_node == (tmnl_pdblist)0) {
        terminal_pro_debug("no such type addr found!");
        return;
    }

    tmnlType = tmnl_node->tmnl_dev.address.tmn_type;
    setType = tmnl_type.identity;
    if ((tmnlType == TMNL_TYPE_CHM_COMMON
            ||tmnlType == TMNL_TYPE_CHM_EXCUTE)
          && (setType == TMNL_TYPE_CHM_COMMON
            ||setType == TMNL_TYPE_CHM_EXCUTE))
    {
        tmnl_node->tmnl_dev.address.tmn_type = setType;
        terminal_chairman_excute_set(addr,
            (setType == TMNL_TYPE_CHM_EXCUTE) ? true : false);
        if (setType == TMNL_TYPE_CHM_EXCUTE) {
            terminal_chairman_apply_type_set(addr);
        }
        else {
            terminal_chairman_apply_type_clear(addr);
        }
    }
    else if (tmnlType == TMNL_TYPE_COMMON_RPRST
            ||tmnlType == TMNL_TYPE_VIP)
    {
        if (setType == TMNL_TYPE_VIP) {
            tmnl_node->tmnl_dev.address.tmn_type = TMNL_TYPE_VIP;
        }
        else {
            tmnl_node->tmnl_dev.address.tmn_type = TMNL_TYPE_COMMON_RPRST;
        }

        terminal_vip_type_set(addr,
            (setType == TMNL_TYPE_VIP) ? true : false);
    }
    else {
        /* no process in else case */
        return;
    }

    for (i = 0; i < SYSTEM_TMNL_MAX_NUM; i++) {
        if (tmnl_addr_list[i].addr == addr) {
            tmnl_addr_list[i].tmn_type = setType;
            break;
        }
    }

    if (terminal_type_save_to_address_profile(addr, setType) < 0) {
       gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_ERROR,
                "[Can't save terminal Type(0x%04x-%d) Failed]",
                addr, setType);
    }
    else {
       gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_DEBUG,
                "[Save terminal Type(0x%04x-%d) Success]",
                addr, setType);
    }
}

void terminal_chairman_excute_set( uint16_t addr, bool is_set_excute )
{
	tmnl_pdblist tmp = found_terminal_dblist_node_by_addr( addr );
	if( tmp == NULL )
	{
		terminal_option_endpoint( BRDCST_1722_ALL, addr, \
			is_set_excute ? OPT_TMNL_SET_EXCUTE_CHM : OPT_TMNL_CANCEL_EXCUTE_CHM );
	}
	else
	{
		terminal_option_endpoint( tmp->tmnl_dev.entity_id, addr, \
			is_set_excute ? OPT_TMNL_SET_EXCUTE_CHM : OPT_TMNL_CANCEL_EXCUTE_CHM );
	}
}

void terminal_vip_type_set( uint16_t addr, bool is_set_vip )
{
	tmnl_pdblist tmp = found_terminal_dblist_node_by_addr( addr );
	if( tmp == NULL )
	{
		terminal_option_endpoint( BRDCST_1722_ALL, addr,\
			is_set_vip ? OPT_TMNL_SET_VIP : OPT_TMNL_CANCEL_VIP );
	}
	else
	{
		terminal_option_endpoint( tmp->tmnl_dev.entity_id, addr,\
			is_set_vip ? OPT_TMNL_SET_VIP : OPT_TMNL_CANCEL_VIP );
	}
}

int terminal_type_save_to_address_profile(uint16_t addr,
    uint16_t tmnl_type)
{
    FILE *fd = NULL;
    uint16_t crc;
    int i = 0;
    int index = 0;
    uint16_t type_tmp;
    terminal_address_list addr_list[SYSTEM_TMNL_MAX_NUM];

    fd = Fopen(ADDRESS_FILE, "rb+");
    if (fd == NULL) {
        DEBUG_ERR("addr file open Err: %s", ADDRESS_FILE);
        return -1;
    }

    memset(addr_list, 0xff,
        sizeof(terminal_address_list) * SYSTEM_TMNL_MAX_NUM);
    if (terminal_address_list_read_file(fd, addr_list ) == -1) {
        terminal_pro_debug("type save read file Err!");
        return -1;
    }

    for (i = 0; i < SYSTEM_TMNL_MAX_NUM; i++) {
        if (addr_list[i].addr == addr) {
            type_tmp = addr_list[i].tmn_type;
            index = i;
            break;
        }
    }

    if (i >= SYSTEM_TMNL_MAX_NUM) {
        terminal_pro_debug("not found addr in the address profile !");
        return -1;
    }
    
    /* read write crc */
    if (Fseek(fd, 2, SEEK_SET ) == -1) {
        return -1;
    }
    	
    if (Fread( fd, &crc, sizeof(uint16_t), 1 ) < 0) {
        return -1;
    }
    
    crc -= type_tmp;
    crc += tmnl_type;
    
    if (Fseek( fd, 2, SEEK_SET ) == -1) {
        return -1;
    }

    if (Fwrite(fd, &crc, sizeof(uint16_t), 1) < 0) {
        return -1;
    }

    if (Fseek(fd, index *sizeof(terminal_address_list) + 4, SEEK_SET) == -1) {
        return -1;
    }

    addr_list[index].tmn_type = tmnl_type;
    if(Fwrite(fd, &addr_list[index], sizeof(terminal_address_list), 1) != 1) {
        terminal_pro_debug("write terminal type Err!");
        return -1;
    }

    Fflush(fd);
    Fclose(fd);
    return 0;
}

void terminal_send_upper_message( uint8_t *data_msg, uint16_t addr, uint16_t msg_len )
{
	assert( data_msg );
	if( data_msg == NULL )
	{
		return;
	}

	if( msg_len > MAX_UPPER_MSG_LEN )
	{
		return;
	}

	//terminal_pro_debug( "=====>>>>>>>>>>>> messsag target addr = %04x<<<<<<<<<<<<<<==========", addr );
	tmnl_pdblist tmp = found_terminal_dblist_node_by_addr( addr );
	if( tmp == NULL )
	{
		terminal_transmit_upper_cmpt_message( BRDCST_1722_ALL, addr, data_msg, msg_len );
	}
	else
	{
		terminal_transmit_upper_cmpt_message( tmp->tmnl_dev.entity_id, addr, data_msg, msg_len );
	}
	
}

void terminal_tablet_stands_manager( tcmpt_table_card *table_card, uint16_t addr, uint16_t contex_len )// 桌牌管理
{
	assert( table_card );
	if( table_card == NULL )
		return;
	
	uint8_t card_flag = table_card->msg_type;
	tmnl_led_state_show_set card_opt;

	if( card_flag == 0 )
	{
		terminal_socroll_synch();
	}
	else if( card_flag == 1 )// 设置led显示方式
	{
		memcpy( &card_opt, table_card->msg_buf, sizeof(uint16_t));

		tmnl_pdblist tmp = found_terminal_dblist_node_by_addr( addr );
		if( tmp == NULL )
		{
			terminal_set_led_play_stype( BRDCST_1722_ALL, addr, card_opt );// 设置led显示方式
		}
		else
		{
			terminal_set_led_play_stype( tmp->tmnl_dev.entity_id, addr, card_opt );
		}
	}
}

// 开始签到
void terminal_start_sign_in( tcmpt_begin_sign sign_flag )
{
	assert( dev_terminal_list_guard );
	if( dev_terminal_list_guard == NULL )
		return;
	
	uint8_t sign_type = sign_flag.sign_type;
	uint8_t timeouts = sign_flag.retroactive_timeouts;
	tmnl_pdblist tmp = dev_terminal_list_guard->next;
	int i = 0;

	set_terminal_system_state( SIGN_STATE, true );
	gtmnl_signstate = SIGN_IN_ON_TIME;
	gsign_latetime = timeouts;
	gsigned_flag = true;

	for( ; tmp != dev_terminal_list_guard; tmp = tmp->next )
	{
		if( tmp->tmnl_dev.address.addr != 0xffff && tmp->tmnl_dev.tmnl_status.is_rgst )
		{
			tmp->tmnl_dev.tmnl_status.sign_state = TMNL_NO_SIGN_IN;
		}
	}

	for( i = 0; i < TMNL_TYPE_NUM; i++)
	{
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0;
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keyup = 0;
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].MicClose = 0;
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].sys = TMNL_SYS_STA_SIGN;
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].sign_stype = sign_type? CARD_SIGN_IN : KEY_SIGN_IN; // 1插卡；0按键
	}

	terminal_state_set_base_type( BRDCST_ALL, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);
	
	// 设置查询签到投票结果(2016-1-27添加)
	gquery_svote_pro.running = true;
	gquery_svote_pro.index = 0;
        gquery_svote_pro.endQr = true;
	host_timer_start( 500, &gquery_svote_pro.query_timer );
}

// 主席控制终端签到
void terminal_chman_control_start_sign_in( uint8_t sign_type, uint8_t timeouts )
{
	assert( dev_terminal_list_guard );
	if( dev_terminal_list_guard == NULL )
		return;
	
	tmnl_pdblist tmp = dev_terminal_list_guard->next;
	int i = 0;

	set_terminal_system_state( SIGN_STATE, true );
	gtmnl_signstate = SIGN_IN_ON_TIME;
	gsign_latetime = timeouts;
	gsigned_flag = true;

	for( ; tmp != dev_terminal_list_guard; tmp = tmp->next )
	{
		if( tmp->tmnl_dev.address.addr != 0xffff && tmp->tmnl_dev.tmnl_status.is_rgst )
		{
			tmp->tmnl_dev.tmnl_status.sign_state = TMNL_NO_SIGN_IN;
		}
	}

	for( i = 0; i < TMNL_TYPE_NUM; i++)
	{
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0;
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keyup = 0;
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].MicClose = 0;
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].sys = TMNL_SYS_STA_SIGN;
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].sign_stype = sign_type? CARD_SIGN_IN : KEY_SIGN_IN; // 1插卡；0按键
	}

	terminal_state_set_base_type( BRDCST_ALL, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);

	// 设置查询签到投票结果(2016-1-28添加)
	gquery_svote_pro.running = true;
	gquery_svote_pro.index = 0;
        gquery_svote_pro.endQr = true;
	host_timer_start( 500, &gquery_svote_pro.query_timer );
}

void Terminal_arcsStarSign(void) {
	assert( dev_terminal_list_guard );
	if( dev_terminal_list_guard == NULL )
		return;
	gset_sys.sign_type = KEY_SIGN_IN;
	tmnl_pdblist tmp = dev_terminal_list_guard->next;
	int i = 0;

	set_terminal_system_state( SIGN_STATE, true );
	gtmnl_signstate = SIGN_IN_ON_TIME;
	gsign_latetime = 10;
	gsigned_flag = true;

	for( ; tmp != dev_terminal_list_guard; tmp = tmp->next )
	{
		if( tmp->tmnl_dev.address.addr != 0xffff && tmp->tmnl_dev.tmnl_status.is_rgst )
		{
			tmp->tmnl_dev.tmnl_status.sign_state = TMNL_NO_SIGN_IN;
		}
	}

	for( i = 0; i < TMNL_TYPE_NUM; i++)
	{
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0;
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keyup = 0;
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].MicClose = 0;
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].sys = TMNL_SYS_STA_SIGN;
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].sign_stype = KEY_SIGN_IN? CARD_SIGN_IN : KEY_SIGN_IN; // 1插卡；0按键
	}

	terminal_state_set_base_type( BRDCST_ALL, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);

	// 设置查询签到投票结果(2016-1-28添加)
	gquery_svote_pro.running = true;
	gquery_svote_pro.index = 0;
        gquery_svote_pro.endQr = true;
	host_timer_start( 500, &gquery_svote_pro.query_timer );
}

void terminal_begin_vote( tcmp_vote_start vote_start_flag,  uint8_t* sign_flag )
{
	uint8_t vote_type = vote_start_flag.vote_type;
	tmnl_pdblist tmp = NULL;

	assert( sign_flag );
	if( sign_flag == NULL )
		return;
	
	*sign_flag = gsigned_flag ? 0 : 1;// 响应0，正常;响应1异常 
	gfirst_key_flag = vote_start_flag.key_effective?true:false;
	
	gvote_mode = (tevote_type)vote_type;
	if( vote_type ==  VOTE_MODE )
	{
		set_terminal_system_state( VOTE_STATE, true );
	}
	else if( vote_type ==  GRADE_MODE )
	{
		set_terminal_system_state( GRADE_STATE, true );
	}
	else
	{
		set_terminal_system_state( ELECT_STATE, true );
	}

	gvote_flag = VOTE_SET;
	assert( dev_terminal_list_guard );
	if( dev_terminal_list_guard == NULL )
		return;
	
	for( tmp = dev_terminal_list_guard->next ; tmp != dev_terminal_list_guard; tmp = tmp->next )
	{
		if( tmp->tmnl_dev.tmnl_status.is_rgst && (tmp->tmnl_dev.address.addr != 0xffff))
		{
			if( tmp->tmnl_dev.tmnl_status.sign_state != TMNL_NO_SIGN_IN )// 已签到
			{
				tmp->tmnl_dev.tmnl_status.vote_state = TWAIT_VOTE_FLAG;
			}
			else
			{
				tmp->tmnl_dev.tmnl_status.vote_state = TVOTE_SET_FLAG; // 未签到不能投票
			}
            
                        if (vote_type == VOTE_MODE) {
                            tmp->tmnl_dev.tmnl_status.is_vote = false;
                        }
                        else if ( vote_type ==  GRADE_MODE ) {
                            tmp->tmnl_dev.tmnl_status.is_grade = false;
                        }
                        else {
                           tmp->tmnl_dev.tmnl_status.is_select = false;
                        }
		}
	}

	terminal_vote_state_set( BRDCST_ALL );

	// 设置查询签到投票结果 (2016-1-27)
	gquery_svote_pro.running = true;
	gquery_svote_pro.index = 0;
        gquery_svote_pro.endQr = true;
	host_timer_start( 500, &gquery_svote_pro.query_timer );
}

void terminal_chman_control_begin_vote(  uint8_t vote_type, bool key_effective, uint8_t* sign_flag )
{// yasir tested in 2016-4-11
	tmnl_pdblist tmp = NULL;
	
	assert( sign_flag );
	if( sign_flag == NULL )
		return;
	
	gfirst_key_flag = key_effective; // true = 首次按键有效；
	*sign_flag = gsigned_flag; 

	gvote_mode = (tevote_type)vote_type;
	if( vote_type ==  VOTE_MODE )
	{
		set_terminal_system_state( VOTE_STATE, true );
	}
	else if( vote_type ==  GRADE_MODE )
	{
		set_terminal_system_state( GRADE_STATE, true );
	}
	else
	{
		set_terminal_system_state( ELECT_STATE, true );
	}

	gvote_flag = VOTE_SET;
	assert( dev_terminal_list_guard );
	if( dev_terminal_list_guard == NULL )
	{
		return;
	}
	
	for( tmp = dev_terminal_list_guard->next; tmp != dev_terminal_list_guard; tmp = tmp->next )
	{
		if( tmp->tmnl_dev.tmnl_status.is_rgst && (tmp->tmnl_dev.address.addr != 0xffff))
		{
			if( tmp->tmnl_dev.tmnl_status.sign_state != TMNL_NO_SIGN_IN )// 已签到
			{
				tmp->tmnl_dev.tmnl_status.vote_state = TWAIT_VOTE_FLAG;
			}
			else
			{
				tmp->tmnl_dev.tmnl_status.vote_state = TVOTE_SET_FLAG; // 未签到不能投票
			}
                        if (vote_type == VOTE_MODE) {
                            tmp->tmnl_dev.tmnl_status.is_vote = false;
                        }
                        else if ( vote_type ==  GRADE_MODE ) {
                            tmp->tmnl_dev.tmnl_status.is_grade = false;
                        }
                        else {
                           tmp->tmnl_dev.tmnl_status.is_select = false;
                        }
		}
	}
    
        if (gsigned_flag) {
        	terminal_vote_state_set( BRDCST_ALL );
        }

	// 设置查询签到投票结果 (2016-4-8)
	gquery_svote_pro.running = true;
	gquery_svote_pro.index = 0;
        gquery_svote_pro.endQr = true;
	host_timer_start( 500, &gquery_svote_pro.query_timer );
}

void terminal_vote_state_set( uint16_t addr )
{
	tevote_type vote_type = gvote_mode;
	if( addr == 0xffff )
		return;

	gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].one_off = gfirst_key_flag ? 1 : 0;
	gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].VoteType = vote_type;
	gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].MicClose = 0;
	switch( vote_type )
	{
		case VOTE_MODE:
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0x0e; // 2 3 4 键
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keyup = 0;
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].sys = TMNL_SYS_STA_VOTE;
			terminal_state_set_base_type( addr, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);
			terminal_lcd_display_num_send( addr, LCD_OPTION_DISPLAY, VOTE_INTERFACE );
			terminal_led_set_save( addr, TLED_KEY1, TLED_OFF );
			terminal_led_set_save( addr, TLED_KEY2, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY3, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY4, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY5, TLED_OFF );
			fterminal_led_set_send( addr );
			break;
		case GRADE_MODE:
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0x1f; // 1 2 3 4 5 键
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keyup = 0;
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].sys = TMNL_SYS_STA_GRADE;
			terminal_state_set_base_type( addr, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);
			terminal_lcd_display_num_send( addr, LCD_OPTION_DISPLAY, GRADE_1_INTERFACE );
			terminal_led_set_save( addr, TLED_KEY1, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY2, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY3, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY4, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY5, TLED_ON );
			fterminal_led_set_send( addr );
			break;
		case SLCT_2_1:
		case	SLCT_2_2:
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0x03;// 1 2 键
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keyup = 0;
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].sys = TMNL_SYS_STA_SELECT;
			terminal_state_set_base_type( addr, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);
			terminal_lcd_display_num_send( addr, LCD_OPTION_DISPLAY, SLCT_LV_2_INTERFACE );
			terminal_led_set_save( addr, TLED_KEY1, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY2, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY3, TLED_OFF );
			terminal_led_set_save( addr, TLED_KEY4, TLED_OFF );
			terminal_led_set_save( addr, TLED_KEY5, TLED_OFF );
			fterminal_led_set_send( addr );
			break;
		case SLCT_3_1:
		case SLCT_3_2:
		case SLCT_3_3:
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0x07;// 1 2 3 键
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keyup = 0;
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].sys = TMNL_SYS_STA_SELECT;
			terminal_state_set_base_type( addr, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);
			terminal_lcd_display_num_send( addr, LCD_OPTION_DISPLAY, SLCT_LV_3_INTERFACE );
			terminal_led_set_save( addr, TLED_KEY1, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY2, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY3, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY4, TLED_OFF );
			terminal_led_set_save( addr, TLED_KEY5, TLED_OFF );
			fterminal_led_set_send( addr );
			break;
		case SLCT_4_1:
		case SLCT_4_2:
		case SLCT_4_3:
		case SLCT_4_4:
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0x0f; // 1 2 3 键
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keyup = 0;
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].sys = TMNL_SYS_STA_SELECT;
			terminal_state_set_base_type( addr, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);
			terminal_lcd_display_num_send( addr, LCD_OPTION_DISPLAY, SLCT_LV_4_INTERFACE );
			terminal_led_set_save( addr, TLED_KEY1, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY2, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY3, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY4, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY5, TLED_OFF );
			fterminal_led_set_send( addr );
			break;
		case SLCT_5_1:
		case SLCT_5_2:
		case SLCT_5_3:
		case SLCT_5_4:
		case SLCT_5_5:
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0x1f; // 1 2 3 4 5 键
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keyup = 0;
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].sys = TMNL_SYS_STA_VOTE;
			terminal_state_set_base_type( addr, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);
			terminal_lcd_display_num_send( addr, LCD_OPTION_DISPLAY, SLCT_LV_5_INTERFACE );
			terminal_led_set_save( addr, TLED_KEY1, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY2, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY3, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY4, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY5, TLED_ON );
			fterminal_led_set_send( addr );
			break;
		default:
			terminal_pro_debug( "out of bround : vote of type!");
			break;
	}

	terminal_state_all_copy_from_common();
}

void terminal_sign_in_special_event( tmnl_pdblist sign_node ) // 终端特殊事件-签到
{
	assert( sign_node );
	if( sign_node == NULL )
		return;
	
	if( gtmnl_signstate == SIGN_IN_ON_TIME )// 设置签到标志
	{
		sign_node->tmnl_dev.tmnl_status.sign_state = TMNL_SIGN_ON_TIME;
	}
	else if( (gtmnl_signstate == SIGN_IN_BE_LATE) && (sign_node->tmnl_dev.tmnl_status.sign_state == TMNL_NO_SIGN_IN) )
	{
		sign_node->tmnl_dev.tmnl_status.sign_state = SIGN_IN_BE_LATE;
	}
	
	// 设置投票使能
	termianl_vote_enable_func_handle( sign_node );
	// 上报签到情况
	terminal_pro_debug( "sign state = %d", sign_node->tmnl_dev.tmnl_status.sign_state );
	upper_cmpt_report_sign_in_state( sign_node->tmnl_dev.tmnl_status.sign_state, sign_node->tmnl_dev.address.addr );

	assert( dev_terminal_list_guard );
	if( dev_terminal_list_guard == NULL )
		return;
	
	tmnl_pdblist tmp = dev_terminal_list_guard->next;
	int sign_num = 0;
	for( ; tmp != dev_terminal_list_guard; tmp = tmp->next )
	{
		if( tmp->tmnl_dev.address.addr != 0xffff && \
			tmp->tmnl_dev.tmnl_status.is_rgst && \
			tmp->tmnl_dev.tmnl_status.sign_state == TMNL_NO_SIGN_IN )
		{
			break;
		}
		
		sign_num++;
	}

	terminal_pro_debug( "sign num = %d", sign_num );
	if( sign_num >= SYSTEM_TMNL_MAX_NUM )// all sign in
	{
		gtmnl_signstate = SIGN_IN_OVER;
		terminal_option_endpoint( BRDCST_1722_ALL, BRDCST_EXE, OPT_TMNL_ALL_SIGN );
	}
}

void termianl_vote_enable_func_handle( tmnl_pdblist sign_node )
{
	assert( sign_node );
	if( sign_node == NULL )
		return;
	
	sign_node->tmnl_dev.tmnl_status.vote_state |= TVOTE_EN; // TVOTE_SET_FLAG ->TVOTE_EN ->TWAIT_VOTE_FLAG(投票状态流程)
}

void terminal_state_all_copy_from_common( void )
{
	memcpy( &gtmnl_state_opt[TMNL_TYPE_VIP], &gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST], sizeof(tmnl_state_set));
	memcpy( &gtmnl_state_opt[TMNL_TYPE_CHM_COMMON], &gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST], sizeof(tmnl_state_set));
	memcpy( &gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE], &gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST], sizeof(tmnl_state_set));
}

void terminal_broadcast_end_vote_result( uint16_t addr ) // 根据终端的2 3 4键统计结果
{// yasir tested in 2016-4-11
	assert( dev_terminal_list_guard );
	if( dev_terminal_list_guard == NULL )
		return;
	
	tmnl_pdblist tmp = NULL, head_list = dev_terminal_list_guard;
	uint16_t vote_total = 0, neg = 0, abs = 0, aff = 0;
	tmnl_vote_result vote_rslt;

	for( tmp = head_list->next; tmp != head_list; tmp = tmp->next )
	{
		if( (tmp->tmnl_dev.address.addr == 0xffff) || (!tmp->tmnl_dev.tmnl_status.is_rgst) )
		{
			continue;
		}

		if( tmp->tmnl_dev.tmnl_status.sign_state != TMNL_NO_SIGN_IN )
		{
			vote_total++;
		}

		if( (tmp->tmnl_dev.tmnl_status.vote_state & TVOTE_KEY_MARK) == TVOTE_KEY2_ENABLE )// 2键按下 ,反对
		{
			neg++;
		}
		else if( (tmp->tmnl_dev.tmnl_status.vote_state & TVOTE_KEY_MARK) == TVOTE_KEY3_ENABLE )// 3键按下 ,弃权
		{
			abs++;
		}
		else if( (tmp->tmnl_dev.tmnl_status.vote_state & TVOTE_KEY_MARK) == TVOTE_KEY4_ENABLE )// 4键按下 ,赞成
		{
			aff++;
		}
	}

	vote_rslt.total = vote_total;
	vote_rslt.neg = neg;
	vote_rslt.abs = abs;
	vote_rslt.aff = aff;

	gp_log_imp->log.post_log_msg( &gp_log_imp->log, LOGGING_LEVEL_INFO, "------Vote Info------\nvote_total = %d	neg = %d	abs = %d	aff = %d\n", 
								vote_total, 
								neg,
								abs,
								aff );
	if( (tmp = found_terminal_dblist_node_by_addr( addr )) != NULL )
	{
		terminal_send_vote_result( tmp->tmnl_dev.entity_id, addr, vote_rslt );
	}
	else
	{
		terminal_send_vote_result( BRDCST_1722_ALL, addr, vote_rslt );
	}
}

void terminal_vote( uint16_t addr, uint8_t key_num, uint8_t key_value, uint8_t tmnl_state, const uint8_t recvdata )
{
	assert( dev_terminal_list_guard );
	if( dev_terminal_list_guard == NULL )
		return;
	
	tmnl_pdblist tmp_node = NULL, tmp_head = dev_terminal_list_guard;
	if( gvote_flag == NO_VOTE || ( key_num > 5 ) ) // 见协议(2.	终端按键的编号：表决键1~5，发言键6，主席优先键7)
	{
		terminal_pro_debug( "system not ready to vote or key num out of vote key num!" );
		return;
	}

	tmp_node = found_terminal_dblist_node_by_addr( addr );
	if( tmp_node == NULL )
	{
		terminal_pro_debug( "no such tmp_node: addr = %04x", addr );
		return;
	}

	// 保存key值
	if( terminal_key_action_value_judge_can_save( key_num,  tmp_node ) ) 
	{// 特殊响应2
		terminal_key_action_host_special_num2_reply( recvdata, 0, 0, 0, VOID_VOTE_INTERFACE ,tmp_node );
	}
	else
	{
		terminal_key_action_host_common_reply( recvdata, tmp_node );
	}

	upper_cmpt_vote_situation_report( tmp_node->tmnl_dev.tmnl_status.vote_state, tmp_node->tmnl_dev.address.addr );

	// 检查是否所有投票完成
	tmnl_pdblist tmp = tmp_head;
	int vote_num = 0;
	for( ; tmp != tmp_head; tmp = tmp->next )
	{
		if( tmp->tmnl_dev.address.addr != 0xffff && \
			tmp->tmnl_dev.tmnl_status.is_rgst && \
			(tmp->tmnl_dev.tmnl_status.mic_state & TWAIT_VOTE_FLAG ) )
		{
			break;
		}
		
		vote_num++;
	}

	terminal_pro_debug( "vote num = %d", vote_num );
	if( vote_num >= SYSTEM_TMNL_MAX_NUM )
	{
		terminal_option_endpoint( BRDCST_1722_ALL, BRDCST_EXE, OPT_TMNL_ALL_VOTE );
	}
}

bool terminal_key_action_value_judge_can_save( uint8_t key_num,  tmnl_pdblist vote_node )
{
	bool ret = false;
	uint8_t *p_vote_state = NULL;
	
	assert( vote_node );
	if( NULL == p_vote_state )
		return false;
	
	p_vote_state = &vote_node->tmnl_dev.tmnl_status.vote_state;
	assert( p_vote_state );
	if( NULL == p_vote_state )
		return false;

	if( !gfirst_key_flag )	// last key effective 
	{
		*p_vote_state &= (~TVOTE_KEY_MARK);
		*p_vote_state |= (1<< ( key_num -1));
		return ret;
	}

	switch(gvote_mode)
	{
		case VOTE_MODE:
		case GRADE_MODE:
		case SLCT_2_1:
		case SLCT_3_1:
		case SLCT_4_1:
		case SLCT_5_1:
			*p_vote_state |= (1<< ( key_num -1));
			*p_vote_state &= (~TWAIT_VOTE_FLAG);// 设置结束标志
			ret = true;
			break;
		case SLCT_2_2:
		case SLCT_3_2:
		case SLCT_4_2:
		case SLCT_5_2:
			*p_vote_state |= (1<< ( key_num -1));
			if(COUNT_KEY_DOWN_NUM( *p_vote_state & TVOTE_KEY_MARK ) >= 2 )
			{
				*p_vote_state &= (~TWAIT_VOTE_FLAG);// 设置结束标志
				ret = true;
			}
			break;
		case SLCT_3_3:
		case SLCT_4_3:
		case SLCT_5_3:
			*p_vote_state |= (1<< ( key_num -1));
			if(COUNT_KEY_DOWN_NUM( *p_vote_state & TVOTE_KEY_MARK ) >= 3 )
			{
				*p_vote_state &= (~TWAIT_VOTE_FLAG);// 设置结束标志
				ret = true;
			}
			break;
		case SLCT_4_4:
		case SLCT_5_4:
			*p_vote_state |= (1<< ( key_num -1));
			if(COUNT_KEY_DOWN_NUM( *p_vote_state & TVOTE_KEY_MARK ) >= 4 )
			{
				*p_vote_state &= (~TWAIT_VOTE_FLAG);// 设置结束标志
				ret = true;
			}
			break;
		case SLCT_5_5:
			*p_vote_state |= (1<< ( key_num -1));
			if(COUNT_KEY_DOWN_NUM( *p_vote_state & TVOTE_KEY_MARK ) >= 5 )
			{
				*p_vote_state &= (~TWAIT_VOTE_FLAG);// 设置结束标志
				ret = true;
			}
			break;
		default:
			break;
	}

	return ret;
}

void terminal_key_action_host_special_num2_reply( const uint8_t recvdata, uint8_t key_down, uint8_t key_up, uint16_t key_led, uint8_t lcd_num, tmnl_pdblist node )
{
	uint8_t data_len;
	tka_special2_reply reply_data;
	reply_data.recv_data = recvdata;
	reply_data.reply_num = REPLY_SPECAIL_NUM2;
	reply_data.key_down = key_down & TVOTE_KEY_MARK;
	reply_data.key_up = key_up & TVOTE_KEY_MARK;
	reply_data.sys = recvdata & KEY_ACTION_TMN_STATE_MASK;
	reply_data.key_led = key_led & 0x03ff;// 低十位
	reply_data.lcd_num = lcd_num;
	data_len = SPECIAL2_REPLY_KEY_AC_DATA_LEN;

	assert( node );
	if( node == NULL )
		return;
	
	terminal_key_action_host_reply( node->tmnl_dev.entity_id, node->tmnl_dev.address.addr, data_len, NULL, NULL, &reply_data );
}

void terminal_key_action_host_special_num1_reply( const uint8_t recvdata, uint8_t mic_state, tmnl_pdblist node )
{
	uint8_t data_len;
	tka_special1_reply reply_data;
	reply_data.mic_state = mic_state;
	reply_data.reply_num = REPLY_SPECAIL_NUM1;
	reply_data.recv_data = recvdata;
	data_len = SPECIAL1_REPLY_KEY_AC_DATA_LEN;
	
	assert( node );
	if( node == NULL )
		return;
	
	terminal_key_action_host_reply( node->tmnl_dev.entity_id, node->tmnl_dev.address.addr, data_len, NULL, &reply_data, NULL );
}

void terminal_key_action_host_common_reply( const uint8_t recvdata, tmnl_pdblist node )
{
	uint8_t data_len;
	tka_common_reply common_data;
	common_data.recv_data = recvdata;
	data_len = COMMON_REPLY_KEY_AC_DATA_LEN;

	assert( node );
	if( node == NULL )
		return;
	
	terminal_key_action_host_reply( node->tmnl_dev.entity_id, node->tmnl_dev.address.addr, data_len, &common_data, NULL, NULL );
}

void terminal_key_speak( uint16_t addr, uint8_t key_num, uint8_t key_value, uint8_t tmnl_state, const uint8_t recvdata )
{
	tmnl_pdblist tmp_node = NULL;
	
	tmp_node = found_terminal_dblist_node_by_addr( addr );
	if( tmp_node == NULL )
	{
		terminal_pro_debug( "no such tmp_node: addr = %04x", addr );
		return;
	}

	if( key_num == KEY6_SPEAK )
	{
	        uint32_t cur_time = get_current_time();
	        if ((cur_time - tmp_node->tmnl_dev.spk_operate_timp) < SPK_KEY_OPT_TIME)
        	{
        		DEBUG_INFO("speaking key press not time out, do not do that fast");
        		terminal_key_action_host_special_num1_reply( recvdata, tmp_node->tmnl_dev.tmnl_status.mic_state, tmp_node );
        		tmp_node->tmnl_dev.spk_operate_timp = cur_time;
                        return;
        	}
            
		if( key_value )
		{
			// no limit time
			bool speak_track = true;
			if (!trans_model_unit_is_connected(tmp_node->tmnl_dev.entity_id))
			{
				if (0 !=trans_model_unit_connect(tmp_node->tmnl_dev.entity_id, tmp_node))
				{
					terminal_key_action_host_special_num1_reply( recvdata, MIC_COLSE_STATUS, tmp_node );
					speak_track = false;
				}
				else
					terminal_key_action_host_special_num1_reply( recvdata, MIC_OPEN_STATUS, tmp_node );
			}
			else
				terminal_key_action_host_special_num1_reply( recvdata, MIC_OPEN_STATUS, tmp_node );

			if (speak_track)
				terminal_speak_track(tmp_node->tmnl_dev.address.addr, true);
		}
		else
		{
			terminal_key_action_host_special_num1_reply( recvdata, MIC_COLSE_STATUS, tmp_node );
			if (0 == trans_model_unit_disconnect(tmp_node->tmnl_dev.entity_id, tmp_node))// disconnect success!
				terminal_speak_track(tmp_node->tmnl_dev.address.addr, false );
		}
	}
}

void terminal_key_action_chman_interpose( uint16_t addr, uint8_t key_num, uint8_t key_value, uint8_t tmnl_state, const uint8_t recvdata )
{
	if( key_num != KEY7_CHAIRMAN_FIRST )
	{
		terminal_pro_debug( "not valid chairman interpose value key " );
		return;
	}

	tmnl_pdblist tmp_node = found_terminal_dblist_node_by_addr( addr );
	if( tmp_node == NULL )
	{
		terminal_pro_debug( "no such 0x%04x addr chairman!", addr );
		return;
	}

	if( key_value )
	{
		terminal_chairman_interpose( addr, true, tmp_node, recvdata );
	}
	else
	{
		terminal_chairman_interpose( addr, false, tmp_node, recvdata );
	}
}

#ifdef MIC_PRIOR_MANEGER_ENABLE
/*$ terminal_chairman_interpose_success()...................................*/
void terminal_chairman_interpose_success(const tmnl_pdblist chman_node) {
    uint64_t target_id;
    uint16_t address;
    thost_system_set set_sys;
    tmnl_pdblist tmp_node;
    tcmpt_data_mic_status mic_list[MAX_SPK_NUM*3];
    uint16_t report_mic_num;
    bool tmp_close;
    bool reSuccess;
    uint8_t tmnlType;

    /* set other mic status */
    terminal_mic_state_set(MIC_CHM_INTERPOSE_STATUS,
        BRDCST_ALL, BRDCST_1722_ALL, true, (tmnl_pdblist)0);

    /* \Note1: because of not finishing mute function,
        whether temp closing or not, it will disconnect all
        common connect channel */
    memcpy(&set_sys, &gset_sys, sizeof(thost_system_set));
    tmp_close = (set_sys.temp_close != 0) ? true : false;
    report_mic_num = 0;
    
    list_for_terminal_link_list_each(tmp_node, dev_terminal_list_guard) {
        tmnlType = tmp_node->tmnl_dev.address.tmn_type;
        if (tmnlType == TMNL_TYPE_COMMON_RPRST) {
            uint8_t mic_state;

            /* open state */
            target_id = tmp_node->tmnl_dev.entity_id;
            address = tmp_node->tmnl_dev.address.addr;
            if (trans_model_unit_is_connected(target_id)) {
                TEReqQePrior prior;
                if (tmnlType == TMNL_TYPE_VIP) {
                    prior = VIP_PRIOR;
                }
                else {
                    prior = COMMON_PRIOR;
                }
                
                reSuccess = Terminal_requestDisConnect(tmp_node,
                        prior, MAX_FAILURE_TIMES,
                        INTERPOSE_STATE);
                if (reSuccess) {
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_DEBUG,
                        "[Chairman interpose Terminal(0x%04x) Close "
                        " Request disconnection success]",
                        tmp_node->tmnl_dev.address.addr);
                }
                else {
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_ERROR,
                        "[Chairman interpose Terminal(0x%04x) Close"
                        " Request disconnection failed]",
                        tmp_node->tmnl_dev.address.addr);
                }
            }

            /* update apply or other apply state to close */
            mic_state = tmp_node->tmnl_dev.tmnl_status.mic_state;
            if ((mic_state != MIC_COLSE_STATUS)
                  && (mic_state != MIC_OPEN_STATUS))
            {
                if (report_mic_num < (MAX_SPK_NUM * 3)) {
                    mic_list[report_mic_num].addr.low_addr =
                        (uint8_t)((address & 0x00ff) >> 0);
                    mic_list[report_mic_num].addr.high_addr =
                        (uint8_t)((address & 0xff00) >> 0);
                    mic_list[report_mic_num].switch_flag =
                        MIC_COLSE_STATUS;
                    report_mic_num++;
                }

                /* set to close */
                if (!tmp_close) {
                    tmp_node->tmnl_dev.tmnl_status.mic_state =
                        MIC_COLSE_STATUS;
                }
            }
        }
    }
    
    cmpt_miscrophone_status_list_from_set(mic_list,
        report_mic_num);
    
    /* clear apply and speak list */
    gdisc_flags.speak_limit_num = 0;
    if (!tmp_close) {
        gdisc_flags.apply_num = 0;
    }

    terminal_main_state_send(0, (void *)0, 0);
}
#else
void terminal_chairman_interpose_success(const uint8_t recvdata,
    const tmnl_pdblist chman_node)
{
    uint64_t target_id;
    uint16_t address;
    thost_system_set set_sys;
    tmnl_pdblist tmp_node;
    tcmpt_data_mic_status mic_list[MAX_SPK_NUM*3];
    uint16_t report_mic_num;
    bool tmp_close;
    uint8_t tmnlType;
    
    /* reply firstly */
    terminal_key_action_host_special_num1_reply(recvdata,
        MIC_CHM_INTERPOSE_STATUS, chman_node);

    /* track camera */
    address = chman_node->tmnl_dev.address.addr;
    terminal_speak_track(address, true);

    /* set other mic status */
    terminal_mic_state_set(MIC_CHM_INTERPOSE_STATUS,
        BRDCST_ALL, BRDCST_1722_ALL, true, (tmnl_pdblist)0);

    memcpy(&set_sys, &gset_sys, sizeof(thost_system_set));
    tmp_close = (set_sys.temp_close != 0) ? true : false;
    report_mic_num = 0;

    /* \Note1: because of not finishing mute function,
        whether temp closing or not, it will disconnect all
        common connect channel */
    list_for_terminal_link_list_each(tmp_node, dev_terminal_list_guard) {
        tmnlType = tmp_node->tmnl_dev.address.tmn_type;
        if (tmnlType == TMNL_TYPE_COMMON_RPRST) {
            uint8_t mic_state;

            /* open state */
            target_id = tmp_node->tmnl_dev.entity_id;
            address = tmp_node->tmnl_dev.address.addr;
            if (trans_model_unit_is_connected(target_id)) {
                trans_model_unit_disconnect(target_id, tmp_node);
                terminal_speak_track(address, false);
            }

            /* update apply or other apply state to close */
            mic_state = tmp_node->tmnl_dev.tmnl_status.mic_state;
            if ((mic_state != MIC_COLSE_STATUS)
                  && (mic_state != MIC_OPEN_STATUS))
            {
                if (report_mic_num < (MAX_SPK_NUM * 3)) {
                    mic_list[report_mic_num].addr.low_addr =
                        (uint8_t)((address & 0x00ff) >> 0);
                    mic_list[report_mic_num].addr.high_addr =
                        (uint8_t)((address & 0xff00) >> 0);
                    mic_list[report_mic_num].switch_flag =
                        MIC_COLSE_STATUS;
                    report_mic_num++;
                }

                /* set to close */
                if (!tmp_close) {
                    tmp_node->tmnl_dev.tmnl_status.mic_state =
                        MIC_COLSE_STATUS;
                }
            }
        }
    }
    
    cmpt_miscrophone_status_list_from_set(mic_list,
        report_mic_num);
    
    /* clear apply and speak list */
    gdisc_flags.speak_limit_num = 0;
    if (!tmp_close) {
        gdisc_flags.apply_num = 0;
    }

    terminal_main_state_send(0, (void *)0, 0);
}

#endif /* MIC_PRIOR_MANEGER_ENABLE */

typedef struct {
    volatile bool occupation;
    TUserTimer timer;
    uint32_t list[CHANNEL_MUX_NUM];
}TTerminal_commonLateSpk;

static TTerminal_commonLateSpk l_interposeLate = {
    (bool)0, {(bool)1, (bool)0, 0U, 0U}, {0U}
};

static TComQueue l_interposeQueue = {
    0U, 0U, 0U, CHANNEL_MUX_NUM, l_interposeLate.list
};

static volatile bool interposeLate = false;

void terminal_speakInterposeOffPro(void) {
    int ret;
    uint16_t curAddr;
    uint64_t id_;
    uint8_t limitSpkNum;
    uint8_t tmnlType;
    tmnl_pdblist spkNode = (tmnl_pdblist)0;
    static int l_failureTime = 0;
    
    INTERRUPT_LOCK(l_interposeLate.occupation);
    
    if (userTimerTimeout(&l_interposeLate.timer)) {
        uint32_t popAddr = 0;
        if (QueueCom_popFiFo(&l_interposeQueue, &popAddr)) {
            spkNode = (tmnl_pdblist)popAddr;
            if (spkNode == (tmnl_pdblist)0) {
                return;
            }
            
            curAddr = spkNode->tmnl_dev.address.addr;
            id_ = spkNode->tmnl_dev.entity_id;
            if (trans_model_unit_is_connected(id_)) {
                terminal_mic_state_set(MIC_OPEN_STATUS,
                        curAddr, id_, true, spkNode);

                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_DEBUG,
                        "[Resume 0x%x to Open"
                        " Success:Already Connected ]", curAddr);

                return;
            }
            
            ret = trans_model_unit_connect(id_, spkNode);
            if (ret == 0) { /* connect success */                
                terminal_speak_track(curAddr, true);
                
                /* add to speaking list for common */
                tmnlType = spkNode->tmnl_dev.address.tmn_type;
                if (tmnlType == TMNL_TYPE_COMMON_RPRST) {
                    limitSpkNum = gdisc_flags.speak_limit_num;
                    gdisc_flags.speak_addr_list[limitSpkNum] =
                                                             curAddr;
                    gdisc_flags.speak_limit_num++;
                    gdisc_flags.speak_limit_num %= MAX_LIMIT_SPK_NUM;
                }

                l_failureTime = 0;

                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_DEBUG,
                    "[ 1. Resume 0x%x(in Queue)"
                    " to Connect Success ]", curAddr);
            }
            else {
                /* chairman cancel interpose quicker than
                    common connect timeout,post to
                    queue(no address in the queue) again */
                l_failureTime++;
                if (l_failureTime > MAX_CONNECT_FAILURE_TIMES) {
                    l_failureTime = 0;
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_ERROR,
                        "[ Resume 0x%x to Connect"
                        " Failed: Connect Failed)", curAddr);

                    return;
                }
                
                if (QueueCom_postLiFo(&l_interposeQueue,
                            (void *)spkNode))
                {
                    userTimerStart(300U, &l_interposeLate.timer);
                    
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                            LOGGING_LEVEL_DEBUG,
                            "[ Resume Post 0x%x to interpose Queue Again ]",
                            curAddr);
                }
                else {
                    l_failureTime = 0;
                    
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_ERROR,
                        "[ Resume 0x%x to Connect"
                        " Failed:Post to queue Again error) ]"
                        " to Close Status ]", curAddr);
                }    
            }
        }
    }
    
    INTERRUPT_UNLOCK(l_interposeLate.occupation);
}

#ifdef MIC_PRIOR_MANEGER_ENABLE

/*$  Terminal_getDisconnectAddress()........................................*/
bool Terminal_getDisconnectAddress(uint16_t * const addr, TEReqQePrior prior,
    uint16_t const * const speakList, uint16_t const speakListLen)
{
    int i = 0;      /* loop variable */
    uint16_t tAddr; /* temp addres */
    bool noDisTask; /* no Disconnections task? */

    /* must be check prior here */
    if (prior >= PRIOR_PUB_NUM) {
        return (bool)0;
    }

    noDisTask = (bool)0;
    for (i = 0; i < speakListLen; i++) {
        tAddr = speakList[i];
        
        if (tAddr != 0xffff) {
            if (!Terminal_hasEventTask(DISCONNECTOR,
                    prior, tAddr))
            {
                noDisTask = (bool)1; /* once get break, no seach any more */
                break;
            }
        }
    }

    if (noDisTask) {
        /* set output address */
        *addr = tAddr;
    }
    else {
        *addr = 0xffff;
    }

    return noDisTask;
}

/*$  Terminal_chairmanInSpeakSuccess()......................................*/
bool Terminal_chairmanInSpeakSuccess(uint16_t speakNum,
            uint32_t permissions)
{
    int i;
    tmnl_pdblist tn;
    uint16_t *spkList[2], spkListLen[2];
    uint16_t firstLimitSpk;
    bool reqConnect;
    bool reSuccess;
    
    if (speakNum >= CCRU_canUsedInStreams) {
        /* check speaking list */
        spkList[0] = gdisc_flags.speak_addr_list;
        spkList[1] = gdisc_flags.vipSpeakList;
        spkListLen[0] = gdisc_flags.speak_limit_num;
        spkListLen[1] = gdisc_flags.vipSpkNum;
        
        for (i = 0; i < 2; i++) {
            if (!Terminal_getDisconnectAddress(&firstLimitSpk,
                    (TEReqQePrior)(2 - i), spkList[i], spkListLen[i]))
            {/* can't get address */
                continue; /* loop to next */
            }
            
            if (firstLimitSpk != 0xffff) {
                /* First in First out */
                tn = found_terminal_dblist_node_by_addr(
                            firstLimitSpk);
                if (tn != (tmnl_pdblist)0) {
                    reSuccess = Terminal_requestDisConnect(tn,
                            (i == 0) ? COMMON_PRIOR : VIP_PRIOR,
                            MAX_FAILURE_TIMES,
                            permissions);
                    if (reSuccess) {
                        reqConnect = (bool)1;
                        
                        gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                            LOGGING_LEVEL_DEBUG,
                            "[Chairman interpose "
                            "Terminal(0x%04x) Speak "
                            " Request disconnection success]",
                            firstLimitSpk);

                        break; /* found */
                    }
                    else {
                        reqConnect = (bool)0;
                        
                        gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                            LOGGING_LEVEL_ERROR,
                            "[Chairman interpose "
                            "Terminal(0x%04x) Speak "
                            " Request disconnection failed]",
                            firstLimitSpk);
                    }
                }
            }
        }
    }
    else {
        reqConnect = (bool)1;
    }

    return reqConnect;
}

/*$ terminal_chairmanCancelAllTask()........................................*/
void terminal_chairmanCancelAllTask(void) {
    int i = 0;
    TSpkQueueElem *qElem;
    uint32_t qAddr, pos;
    /* cancel first speak task */
    TComQueue * const l_queues[FS_SPEAK_QUEUE_NUM] ={
        &l_chairmanFSpkQueue,
        &l_vipFSpkQueue,
        &l_FSpkQueue
    };

    /* lock */
    INTERRUPT_LOCK(l_FSpkLocker);

    for (i = 0; i < FS_SPEAK_QUEUE_NUM; i++) {
        queue_for_each(l_queues[i], pos, qAddr) {
            qElem = (TSpkQueueElem *)qAddr;
            qElem->permissions = 0U;
        }
    }
    
    /* unlock */
    INTERRUPT_UNLOCK(l_FSpkLocker);
    
    /* cancel manager task */
    Terminal_cancelAllTask();
}
/*$ terminal_chairmanCancelAllTask()........................................*/
bool terminal_firstSpeakQueueHasTask(TComQueue * const spkQueue,
    uint16_t const addr)
{
    TSpkQueueElem *qElem;
    uint32_t qAddr, pos;
    bool has;

    /* lock */
    INTERRUPT_LOCK(l_FSpkLocker);

    has = (bool)0;
    queue_for_each(spkQueue, pos, qAddr) {
        qElem = (TSpkQueueElem *)qAddr;

        /* found? */
        if (qElem->spkNode->tmnl_dev.address.addr == addr) {
            has = (bool)1;
            break;
        }
    }
    
    /* unlock */
    INTERRUPT_UNLOCK(l_FSpkLocker);

    return has;
}

#endif /* MIC_PRIOR_MANEGER_ENABLE */

/*$ terminal_chairman_interpose()...........................................*/
void terminal_chairman_interpose(uint16_t addr, bool key_down,
    tmnl_pdblist chman_node, const uint8_t recvdata)
{
    tmnl_pdblist end_node;
    uint64_t target_id;
    uint16_t chmAddr;
    int i = 0;
#ifdef MIC_PRIOR_MANEGER_ENABLE
    bool reSuccess;
    uint16_t speakNum;
#else
    int disRet = -1;
#endif /* MIC_PRIOR_MANEGER_ENABLE */

    assert(chman_node != (tmnl_pdblist)0);
    if (chman_node == NULL) {
        return;
    }

    assert(dev_terminal_list_guard != (tmnl_pdblist)0);
    if (dev_terminal_list_guard == (tmnl_pdblist)0) {
        return;
    }

    if ((key_down && gchm_int_ctl.is_int)
        || ((!key_down) && (!gchm_int_ctl.is_int))
        || ((!key_down) && (gchm_int_ctl.chmaddr != addr)))
    {
        terminal_key_action_host_common_reply(recvdata, chman_node);
        return;
    }

#ifdef MIC_PRIOR_MANEGER_ENABLE
    /* lock */
    INTERRUPT_LOCK(l_callBackLocker);

    speakNum = terminal_speak_num_count();
    
    /* unlock */
    INTERRUPT_UNLOCK(l_callBackLocker);

#endif /* MIC_PRIOR_MANEGER_ENABLE */

    if ((get_sys_state() != INTERPOSE_STATE)
          && (key_down))
    {
        gchm_int_ctl.is_int = true;
        gchm_int_ctl.chmaddr = addr;
        set_terminal_system_state(INTERPOSE_STATE, true);

        /* open mic, and don't save status */
        target_id = chman_node->tmnl_dev.entity_id;
        if (!trans_model_unit_is_connected(target_id)) {
#ifdef MIC_PRIOR_MANEGER_ENABLE
#ifdef FIRST_SPEAK_QUEUE_ENABLE
            /* cannel all connections task */
            terminal_chairmanCancelAllTask();

            /* check has channel */
            if (!Terminal_chairmanInSpeakSuccess(speakNum,
                    INTERPOSE_STATE))
            {
                terminal_key_action_host_special_num1_reply(recvdata,
                    MIC_COLSE_STATUS, chman_node);
                return;
            }
            
            /* only process in INTERPOSE_STATE */
            if (Terminal_firstSpkRequestConnect(chman_node,
                    &l_chairmanFSpkQueue, &l_FSpkLocker,
                    MAX_CONNECT_FAILURE_TIMES,
                    INTERPOSE_STATE))
            {
                /* reply firstly */
                terminal_key_action_host_special_num1_reply(recvdata,
                    MIC_CHM_INTERPOSE_STATUS, chman_node);
            
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_DEBUG,
                    "[ chairman interpose Speak Post First ]"
                    " speaking node(address = 0x%x)"
                    " to Queue Success ]",
                    chman_node->tmnl_dev.address.addr);
            }
            else {
                /* reply firstly */
                terminal_key_action_host_special_num1_reply(recvdata,
                    MIC_COLSE_STATUS, chman_node);
                
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_ERROR,
                    "[ chairman interpose Speak Post First ]"
                    " speaking node(address = 0x%x)"
                    "to Queue Failed ]",
                    chman_node->tmnl_dev.address.addr);
            }
#else
            /* reply firstly */
            terminal_key_action_host_special_num1_reply(recvdata,
                MIC_CHM_INTERPOSE_STATUS, chman_node);

            /* request connection */
            reSuccess = Terminal_requestConnect(chman_node,
                CHAIRMAN_PRIOR, MAX_FAILURE_TIMES,
                INTERPOSE_STATE);
            if (reSuccess) {
                /* reply firstly */
                terminal_key_action_host_special_num1_reply(recvdata,
                    MIC_CHM_INTERPOSE_STATUS, chman_node);
                
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_DEBUG,
                    "[Chairman interpose Terminal(0x%04x) Speak "
                    " Request connection success: Waitting for"
                    "Connection...]",
                    chman_node->tmnl_dev.address.addr);
            }
            else {
                /* reply firstly */
                terminal_key_action_host_special_num1_reply(recvdata,
                    MIC_COLSE_STATUS, chman_node);
                
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_ERROR,
                    "[Chairman Terminal(0x%04x) Speak "
                    " Request connection Faied]",
                    chman_node->tmnl_dev.address.addr);
            }
#endif /* FIRST_SPEAK_QUEUE_ENABLE */
#else
            int ret = -1;
            ret = trans_model_unit_connect(target_id,
                    chman_node);
            if (ret == 0) { /* connect successfully */
                terminal_chairman_interpose_success(recvdata,
                    chman_node);
            }
            else if (ret != -2) { /* no timeout err */
                uint64_t id = 0;
                tmnl_pdblist pDisNode = (tmnl_pdblist)0;
                
                ret = Ctrans_disLongest(&id, &pDisNode);
                if (ret == 0) { /* disconnect success */
                    terminal_chairman_interpose_success(recvdata,
                        chman_node);
                    terminal_over_time_speak_node_set(chman_node);
                    interposeLate = true;
                }
                else {
                    terminal_key_action_host_special_num1_reply(recvdata,
                        MIC_COLSE_STATUS, chman_node);
                }
            }
            else { /* timeout error */
                /* reply */
                terminal_key_action_host_special_num1_reply(recvdata,
                    MIC_COLSE_STATUS, chman_node);
            }
#endif /* MIC_PRIOR_MANEGER_ENABLE */
        }
        else { /* is connect? */
#ifdef MIC_PRIOR_MANEGER_ENABLE
            terminal_chairman_interpose_success(chman_node);
#else
            terminal_chairman_interpose_success(recvdata,
                chman_node);
#endif /* MIC_PRIOR_MANEGER_ENABLE */
        }
    }
    else if (!key_down) {
        uint8_t chmMicStatus, applyMicState;
        tmnl_pdblist tmp_node;
        uint16_t addr_apply;

        set_terminal_system_state(INTERPOSE_STATE, false);
        
        chmMicStatus = chman_node->tmnl_dev.tmnl_status.mic_state;
        terminal_key_action_host_special_num1_reply(recvdata,
                        chmMicStatus, chman_node);

        target_id = chman_node->tmnl_dev.entity_id;
        chmAddr = chman_node->tmnl_dev.address.addr;

        if (trans_model_unit_is_connected(target_id)
             && (chmMicStatus == MIC_COLSE_STATUS))
        { /* has connect and not open */
#ifdef MIC_PRIOR_MANEGER_ENABLE
            reSuccess = Terminal_requestDisConnect(chman_node,
                    CHAIRMAN_PRIOR, MAX_FAILURE_TIMES,
                    COMMON_SPK_PERMISSION);
            if (reSuccess) {                
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_DEBUG,
                    "[Chairman interpose Terminal(0x%04x) Close "
                    " Request disconnection success]",
                    chmAddr);
            }
            else {
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_ERROR,
                    "[Chairman interpose Terminal(0x%04x) Close"
                    " Request disconnection failed]",
                    chmAddr);
            }
#else
            disRet = trans_model_unit_disconnect(target_id,
                    chman_node);
            if (disRet == 0) { /* disconnect success */
                terminal_speak_track(chmAddr, false);
            }
#endif /* MIC_PRIOR_MANEGER_ENABLE */
        }

        list_for_terminal_link_list_each(end_node, dev_terminal_list_guard) {
            uint16_t tempAddr;
            uint64_t id_;
            bool isRegist;
            uint8_t micState;
            uint8_t tmnlType;
#ifdef MIC_PRIOR_MANEGER_ENABLE
            TComQueue *tQueue;
#else
            uint8_t limitSpkNum;
#endif /* MIC_PRIOR_MANEGER_ENABLE */

            tempAddr = end_node->tmnl_dev.address.addr;
            id_ = end_node->tmnl_dev.entity_id;
            isRegist = end_node->tmnl_dev.tmnl_status.is_rgst;
            micState = end_node->tmnl_dev.tmnl_status.mic_state;
            tmnlType = end_node->tmnl_dev.address.tmn_type;
            if ((tempAddr != 0xffff)
                  && (isRegist)
                  && (micState == MIC_OPEN_STATUS)
                  && ((tmnlType == TMNL_TYPE_COMMON_RPRST)
                          ||(tmnlType == TMNL_TYPE_VIP)))
            {
                if (!trans_model_unit_is_connected(id_)) {
#ifdef MIC_PRIOR_MANEGER_ENABLE
#ifdef FIRST_SPEAK_QUEUE_ENABLE
                    if (tmnlType == TMNL_TYPE_VIP) {
                        tQueue = &l_vipFSpkQueue;
                    }
                    else {
                        tQueue = &l_FSpkQueue;
                    }

                    /* only process in INTERPOSE_STATE, can't set
                        discuss parameter for apply mode */
                    if (Terminal_firstSpkRequestConnect(end_node,
                            tQueue, &l_FSpkLocker,
                            MAX_CONNECT_FAILURE_TIMES,
                            COMMON_SPK_PERMISSION & (~APPLY_CB_PERMISSION)))
                    {
                        gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                            LOGGING_LEVEL_DEBUG,
                            "[ chairman interpose Speak(Resume) Post First ]"
                            " speaking node(address = 0x%x)"
                            " to Queue Success ]",
                            end_node->tmnl_dev.address.addr);
                    }
                    else {
                        gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                            LOGGING_LEVEL_ERROR,
                            "[ chairman interpose Speak(Resume) Post First ]"
                            " speaking node(address = 0x%x)"
                            "to Queue Failed ]",
                            end_node->tmnl_dev.address.addr);
                    }
#else
                    /* request connection */
                    reSuccess = Terminal_requestConnect(end_node,
                        (tmnlType == TMNL_TYPE_VIP)?
                            VIP_PRIOR:COMMON_PRIOR,
                        MAX_FAILURE_TIMES,
                        COMMON_SPK_PERMISSION);
                    if (reSuccess) {
                        gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                            LOGGING_LEVEL_DEBUG,
                            "[Chairman interpose Terminal(0x%04x) Speak "
                            " Request connection success: Waitting for"
                            "Connection...]",
                            end_node->tmnl_dev.address.addr);
                    }
                    else {
                        gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                            LOGGING_LEVEL_ERROR,
                            "[Chairman interpose Terminal(0x%04x) Speak "
                            " Request connection Faied]",
                            end_node->tmnl_dev.address.addr);
                    }
#endif /* FIRST_SPEAK_QUEUE_ENABLE */
#else                     
                    int ret;
                    ret = trans_model_unit_connect(id_, end_node);
                    if (ret == 0) { /* connect success */
                        terminal_speak_track(tempAddr, true);

                        /* add to speaking list for common */
                        if (tmnlType == TMNL_TYPE_COMMON_RPRST) {
                            limitSpkNum = gdisc_flags.speak_limit_num;
                            gdisc_flags.speak_addr_list[limitSpkNum] =
                                                                     tempAddr;
                            gdisc_flags.speak_limit_num++;
                            gdisc_flags.speak_limit_num %= MAX_LIMIT_SPK_NUM;
                        }

                        gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                            LOGGING_LEVEL_DEBUG,
                            "[ 1. Resume 0x%x to Connect Success]", tempAddr);
                    }
                    else {
                        /* resume by queue */
                        end_node->tmnl_dev.tmnl_status.mic_state =
                            MIC_COLSE_STATUS;

                        /* post to queue */
                        INTERRUPT_LOCK(l_interposeLate.occupation);
                        
                        if (QueueCom_postFiFo(&l_interposeQueue,
                            (void *)end_node))
                        {
                            userTimerStart(500, &l_interposeLate.timer);
                            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                                    LOGGING_LEVEL_DEBUG,
                                    "[ Resume Post 0x%x to interpose"
                                    " Queue Success", tempAddr);
                        }
                        else {
                            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                                LOGGING_LEVEL_ERROR,
                                "[ Resume 0x%x to Connect"
                                " Failed:Post to queue error)"
                                " to Close Status ]", tempAddr);
                        }

                        INTERRUPT_UNLOCK(l_interposeLate.occupation);
                    }
#endif /* MIC_PRIOR_MANEGER_ENABLE */
                }
                else {
                    terminal_mic_state_set(MIC_OPEN_STATUS,
                            tempAddr, id_, true, end_node);

                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                            LOGGING_LEVEL_DEBUG,
                            "[ 2. Resume 0x%x to Open Success ]", tempAddr);
                }
            }
        }

        for (i = 0; i < gdisc_flags.apply_num; i++) {
            addr_apply = gdisc_flags.apply_addr_list[i];
            tmp_node = found_terminal_dblist_node_by_addr(addr_apply);
            if (tmp_node == (tmnl_pdblist)0) {
                continue;
            }

            if (i == gdisc_flags.currect_first_index) {
                applyMicState = MIC_FIRST_APPLY_STATUS;
            }
            else {
                applyMicState = MIC_OTHER_APPLY_STATUS;
            }

            terminal_mic_state_set(applyMicState, addr_apply,
                    tmp_node->tmnl_dev.entity_id, true, tmp_node);

            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_DEBUG,
                "[ Resume 0x%x to Apply(%d) Success ]", addr_apply,
                    applyMicState);
        }

        gchm_int_ctl.is_int = false;
        gchm_int_ctl.chmaddr = 0xffff;

        terminal_main_state_send(0, (void *)0, 0);
    }
}

int terminal_key_discuccess( uint16_t addr, uint8_t key_num, uint8_t key_value, uint8_t tmnl_state, uint8_t recv_msg )
{
	tmnl_pdblist dis_node;
	
	dis_node = found_terminal_dblist_node_by_addr( addr );
	if( dis_node == NULL )
	{
		terminal_pro_debug( "not found discuccess termianl key!" );
		return -1;
	}

	if( key_num == KEY6_SPEAK )
	{
		if( key_value )
		{
			terminal_key_speak_proccess( dis_node, true, recv_msg );
		}
		else
		{
			terminal_key_speak_proccess( dis_node, false, recv_msg );
		}
	}
	else if( key_num == 0 && key_value )
	{// 见协议命令bit3-bit0=1000:关闭麦克风
		terminal_key_speak_proccess( dis_node, false, recv_msg );
	}

	return 0;
}

void terminal_chman_vip_control_common_mic(void) {
    uint16_t current_addr;
    tmnl_pdblist first_speak;
    
    if((gdisc_flags.speak_limit_num < gdisc_flags.limit_num)
        && (gdisc_flags.apply_num > 0))
    {
        current_addr = gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index];
        if (addr_queue_delete_by_index(gdisc_flags.apply_addr_list,
            &gdisc_flags.apply_num, gdisc_flags.currect_first_index))
        {
            first_speak = found_terminal_dblist_node_by_addr(current_addr);
            if (first_speak != (tmnl_pdblist)0) {
#ifdef FIRST_SPEAK_QUEUE_ENABLE
            if (Terminal_firstSpkRequestConnect(first_speak, &l_FSpkQueue,
                            &l_FSpkLocker, MAX_CONNECT_FAILURE_TIMES,
                            COMMON_SPK_PERMISSION))
            {
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_DEBUG,
                    "[ Vip Chairman Control Terminal( 0x%04x )"
                    " Request Connection Success ]",
                    first_speak->tmnl_dev.address.addr);
            }
            else {
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_DEBUG,
                    "[ Vip Chairman Control Terminal( 0x%04x )"
                    " Request Connection Failed ]",
                    first_speak->tmnl_dev.address.addr);
            }
#else			
            terminal_over_time_speak_node_set(first_speak);
#endif
            }
        }
        else {
            gdisc_flags.currect_first_index = 0;
        }
    }
}

/*$  terminal_speak_num_count().............................................*/
uint16_t terminal_speak_num_count(void) {
    tmnl_pdblist pos;
    uint16_t speakNum;
    uint8_t micState;
    
    assert(dev_terminal_list_guard != (tmnl_pdblist)0);
    if (dev_terminal_list_guard == (tmnl_pdblist)0) {
        return 0;
    }

    speakNum = 0;
    list_for_terminal_link_list_each(pos, dev_terminal_list_guard) {
        micState = pos->tmnl_dev.tmnl_status.mic_state;
        if (micState == MIC_OPEN_STATUS) {
            speakNum++;
        }  
    }

    return speakNum;
}

#ifdef MIC_PRIOR_MANEGER_ENABLE

/*$ Terminal_connect()......................................................*/
int Terminal_connect(uint16_t openAddr) {
    uint8_t tType;
    TComQueue *pQ;
    tmnl_pdblist openNode;
    uint64_t targetId;
#ifndef FIRST_SPEAK_QUEUE_ENABLE
    bool reSuccess;
#endif /* FIRST_SPEAK_QUEUE_ENABLE */
    TEReqQePrior prior;
    int ret = -1;

    if (openAddr == 0xffff) {
        return -1;
    }

    openNode =
        found_terminal_dblist_node_by_addr(openAddr);

    /* no such address node */
    if (openNode == (tmnl_pdblist)0) {
            return -1;
    }
    
    targetId = openNode->tmnl_dev.entity_id;
    tType = openNode->tmnl_dev.address.tmn_type;
    
    if (tType == TMNL_TYPE_CHM_COMMON) {
        prior = COMMON_PRIOR;
        pQ = &l_FSpkQueue;
    }
    else if (tType == TMNL_TYPE_VIP) {
        prior = VIP_PRIOR;
        pQ = &l_vipFSpkQueue;
    }
    else if ((tType == TMNL_TYPE_CHM_COMMON)
                || (tType == TMNL_TYPE_CHM_EXCUTE))
    {
        prior = CHAIRMAN_PRIOR;
        pQ = &l_chairmanFSpkQueue;
    }
    else { /* error type */
        return -1;
    }

    /* check whether terminal has connections */
    if (!trans_model_unit_is_connected(targetId)) {
#ifdef FIRST_SPEAK_QUEUE_ENABLE
        if (Terminal_firstSpkRequestConnect(openNode,
                pQ, &l_FSpkLocker,
                MAX_CONNECT_FAILURE_TIMES,
                get_sys_state()))
        {
            ret = 0;
        
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_DEBUG,
                "[ Terminal_connect() Speak Post First"
                " speaking node(address = 0x%x)"
                " to Queue Success ]",
                openAddr);
        }
        else {            
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_ERROR,
                "[ Terminal_connect() Speak Post First"
                " speaking node(address = 0x%x)"
                "to Queue Failed ]",
                openAddr);
        }
#else            
        /* request connection */
        reSuccess = Terminal_requestConnect(openNode,
            prior, MAX_FAILURE_TIMES,
            get_sys_state());
        if (reSuccess) {
            ret = 0;
            
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
            LOGGING_LEVEL_DEBUG,
            "[Terminal_connect(), Terminal(0x%04x) Speak "
            " Request connection success: Waitting for"
            "Connection...]", openAddr);
        }
        else {
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
            LOGGING_LEVEL_ERROR,
            "[Terminal_connect(), Terminal(0x%04x) Speak "
            " Request connection Faied]", openAddr);
        }
#endif
    }                    

    return ret;
}

/*$ Terminal_disconnect()...................................................*/
int Terminal_disconnect(uint16_t closeAddr) {
    uint8_t tType;
    tmnl_pdblist closeNode;
    uint64_t targetId;
    bool reSuccess;
    TEReqQePrior prior;
    int ret = -1;

    if (closeAddr == 0xffff) {
        return -1;
    }

    closeNode =
        found_terminal_dblist_node_by_addr(closeAddr);

    /* no such address node */
    if (closeNode == (tmnl_pdblist)0) {
            return -1;
    }
    
    targetId = closeNode->tmnl_dev.entity_id;
    tType = closeNode->tmnl_dev.address.tmn_type;
    
    if (tType == TMNL_TYPE_CHM_COMMON) {
        prior = COMMON_PRIOR;
    }
    else if (tType == TMNL_TYPE_VIP) {
        prior = VIP_PRIOR;
    }
    else if ((tType == TMNL_TYPE_CHM_COMMON)
                || (tType == TMNL_TYPE_CHM_EXCUTE))
    {
        prior = CHAIRMAN_PRIOR;
    }
    else { /* error type */
        return -1;
    }    
    
    /* check whether terminal has connections */
    if (trans_model_unit_is_connected(targetId)) {
        reSuccess = Terminal_requestDisConnect(closeNode,
                prior, MAX_FAILURE_TIMES,
                get_sys_state());
            if (reSuccess) {
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_DEBUG,
                    "[[Terminal_disconnect() Terminal(0x%04x) Close "
                    " Request disconnection success: Waitting for"
                    " disConnection...]", closeAddr);
            }
            else {
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_ERROR,
                    "[Terminal_disconnect() Terminal(0x%04x) Close "
                    " Request disconnection Faied]", closeAddr);
            }
    }

    return ret;
}

/*$\ define callback function after request connect finish */
/*$ Terminal_chairmanOpenMicCallback()......................................*/
int Terminal_chairmanOpenMicCallback(bool isSuccess,
                tmnl_pdblist user, uint32_t permissions)
{
    uint8_t micState;
    uint8_t disMode;
    uint8_t sysState;
    uint64_t entityId;
    uint16_t tAddr;

    if (user == (tmnl_pdblist)0) {
        return 0;
    }
    
    /* for muti pthread access global data,
        must lock */
    INTERRUPT_LOCK(l_callBackLocker);
    
    micState = user->tmnl_dev.tmnl_status.mic_state;
    disMode = gdisc_flags.edis_mode;
    sysState = get_sys_state();
    entityId = user->tmnl_dev.entity_id;
    tAddr = user->tmnl_dev.address.addr;

    /* open mic failed? */
    if (!isSuccess) {
        /* error discuss mode */
        if (sysState & permissions) {
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_ERROR,
                "[Chairman Terminal(0x%04x) Speak Failed: Out of"
                "connect times]",
                user->tmnl_dev.address.addr);
        }
        else {
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_ERROR,
                "[Chairman Terminal(0x%04x) Speak Failed: No"
                "Executable permissions]",
                user->tmnl_dev.address.addr);
        }
    }
    else {
        gp_log_imp->log.post_log_msg(&gp_log_imp->log,
            LOGGING_LEVEL_DEBUG,
            "[Chairman Terminal(0x%04x) Speak Success]",
            user->tmnl_dev.address.addr);

        /* set mic status */
        terminal_mic_status_set_callback((bool)1, user);

        /* track camera */
        terminal_speak_track(tAddr, true);

        if (sysState == INTERPOSE_STATE) {
            terminal_chairman_interpose_success(user);
       }
    }

    /* for muti pthread access global data,
        must unlock */
    INTERRUPT_UNLOCK(l_callBackLocker);
    
    return 0;
}

/*$ Terminal_vipOpenMicCallback()...........................................*/
int Terminal_vipOpenMicCallback(bool isSuccess,
                tmnl_pdblist user, uint32_t permissions)
{
    uint8_t micState;
    uint8_t disMode;
    uint8_t sysState;
    uint64_t entityId;
    uint16_t tAddr;

    if (user == (tmnl_pdblist)0) {
        return 0;
    }
    
    /* for muti pthread access global data,
        must lock */
    INTERRUPT_LOCK(l_callBackLocker);
    
    micState = user->tmnl_dev.tmnl_status.mic_state;
    disMode = gdisc_flags.edis_mode;
    sysState = get_sys_state();
    entityId = user->tmnl_dev.entity_id;
    tAddr = user->tmnl_dev.address.addr;

    /* open mic failed? */
    if (!isSuccess) {
        /* error discuss mode */
        if (sysState & permissions) {
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_ERROR,
                "[VIP Terminal(0x%04x) Speak Failed: Out of"
                "connect times]",
                user->tmnl_dev.address.addr);
        }
        else {
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_ERROR,
                "[VIP Terminal(0x%04x) Speak Failed: No"
                "Executable permissions]",
                user->tmnl_dev.address.addr);
        }
    }
    else {
        gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_DEBUG,
                "[Vip Terminal(0x%04x) Speak Success]",
                user->tmnl_dev.address.addr);
                
        /* set mic status */
        terminal_mic_status_set_callback((bool)1, user);

        /* track camera */
        terminal_speak_track(tAddr, true);

        /* set to vip speaking list */
        gdisc_flags.vipSpeakList[gdisc_flags.vipSpkNum] = tAddr;
        gdisc_flags.vipSpkNum++;
        gdisc_flags.vipSpkNum %= MAX_LIMIT_SPK_NUM;
    }

    /* for muti pthread access global data,
        must unlock */
    INTERRUPT_UNLOCK(l_callBackLocker);
    
    return 0;
}

/*$ Terminal_commonOpenMicCallback()........................................*/
int Terminal_commonOpenMicCallback(bool isSuccess,
                tmnl_pdblist user, uint32_t permissions)
{
    uint8_t micState;
    uint8_t disMode;
    uint8_t sysState;
    uint64_t entityId;
    uint16_t tAddr;
    uint16_t cSpkNum;
    uint8_t cLimitSpkNum;
    uint8_t applyNum;
    uint8_t applyMaxNum;
    uint8_t micTempState;
    uint8_t cFirstApplyIndex;
    tmnl_pdblist applyFirst;

    if (user == (tmnl_pdblist)0) {
        return 0;
    }
    
    /* for muti pthread access global data,
        must lock */
    INTERRUPT_LOCK(l_callBackLocker);

    micState = user->tmnl_dev.tmnl_status.mic_state;
    disMode = gdisc_flags.edis_mode;
    sysState = get_sys_state();
    entityId = user->tmnl_dev.entity_id;
    tAddr = user->tmnl_dev.address.addr;
    cSpkNum = terminal_speak_num_count();
    cLimitSpkNum = gdisc_flags.speak_limit_num;
    applyNum = gdisc_flags.apply_num;
    applyMaxNum = gdisc_flags.apply_limit;   

    /* set mic state */
    /* open mic failed? */
    if (!isSuccess) {
        /* error discuss mode */
        if (sysState & permissions) {
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_ERROR,
                "[Common Terminal(0x%04x) Speak Failed: Out of"
                " connect times]",
                user->tmnl_dev.address.addr);

            /* special process:
                    set to apply for limit(), because
                this terminal is the first for
                appling  speaking */
            if (disMode == LIMIT_MODE) {
                if ((cSpkNum >= CCRU_canUsedInStreams)
                      && (applyNum < applyMaxNum))
                {
                    micTempState = MIC_OTHER_APPLY_STATUS;
                    if (applyNum == 0) {
                        gdisc_flags.currect_first_index = 0;
                        micTempState = MIC_FIRST_APPLY_STATUS;
                    }

                    /* post to apply list */
                    gdisc_flags.apply_addr_list[applyNum] = tAddr;
                    gdisc_flags.apply_num++;

                    /* set and report mic state */
                    terminal_mic_state_set(micTempState,
                        tAddr, entityId, true, user);
                    
                    terminal_main_state_send(0, (void *)0, 0);

                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_DEBUG,
                        "[Common Terminal(0x%04x) Speak(LIMIT) "
                        " Failed: to Appling....]", tAddr);
                }
            }
        }
        else {
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_ERROR,
                "[Common Terminal(0x%04x) Speak Failed: No"
                "Executable permissions]",
                user->tmnl_dev.address.addr);
        }
    }
    else {
        gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_DEBUG,
                "[Common Terminal(0x%04x) Speak Success]",
                user->tmnl_dev.address.addr);
        
        /* set mic status */
        terminal_mic_status_set_callback((bool)1, user);

        /* track camera */
        terminal_speak_track(tAddr, true);

        if (sysState != INTERPOSE_STATE) {
            /* set conference
                parameter */
            gdisc_flags.speak_addr_list[cLimitSpkNum] = tAddr;
            gdisc_flags.speak_limit_num++;
            gdisc_flags.speak_limit_num %= MAX_LIMIT_SPK_NUM;
        }

        /* process for apply mode */
        if ((disMode == APPLY_MODE)
              && (permissions & APPLY_CB_PERMISSION))
        {
            addr_queue_delete_by_index(gdisc_flags.apply_addr_list,
                &gdisc_flags.apply_num, gdisc_flags.currect_first_index);
            
            if (gdisc_flags.apply_num > 0) {
                gdisc_flags.currect_first_index %=
                    gdisc_flags.apply_num;
                cFirstApplyIndex = gdisc_flags.currect_first_index;
                
                applyFirst = found_terminal_dblist_node_by_addr(
                    gdisc_flags.apply_addr_list[cFirstApplyIndex]);
                if (applyFirst != (tmnl_pdblist)0) {
                    terminal_after_time_mic_state_node_set(
                        applyFirst, MIC_FIRST_APPLY_STATUS, true);
                }
            }
            else {
                gdisc_flags.currect_first_index =
                    gdisc_flags.apply_limit;
            }

            /* set main state */
            terminal_main_state_send(0, (void *)0, 0);
        }
        else {
            /* do noting */
        }
    }

    /* for muti pthread access global data,
        must unlock */
    INTERRUPT_UNLOCK(l_callBackLocker);
    
    return 0;
}

/*$ Terminal_limitModeApplyTSpeak().........................................*/
static void Terminal_limitModeApplyTSpeak(void) {
    uint16_t curFirstAlyAddr;
    uint8_t curFirstApplyIndex;
    uint16_t cSpkNum;
    tmnl_pdblist firstSpkNode;
    uint8_t applyNum;
    uint8_t cLimitSpkNum;
    uint8_t maxLimitSpkNum;

    applyNum = gdisc_flags.apply_num;
    cSpkNum = terminal_speak_num_count();
    maxLimitSpkNum = gdisc_flags.limit_num; 
    cLimitSpkNum = gdisc_flags.speak_limit_num;
    curFirstApplyIndex = gdisc_flags.currect_first_index;
    curFirstAlyAddr = gdisc_flags.apply_addr_list[curFirstApplyIndex];

    /* can't Preempt from higher priority queue task? */
    if ((!QueueCom_isEmpty(&l_vipFSpkQueue))
          || (!QueueCom_isEmpty(&l_chairmanFSpkQueue)))
    {
        return;
    }
    
    if ((cLimitSpkNum < maxLimitSpkNum)
         && (cSpkNum < CCRU_canUsedInStreams)
         && (applyNum > 0))
    {
        if (addr_queue_delete_by_index(gdisc_flags.apply_addr_list,
                &gdisc_flags.apply_num,
                gdisc_flags.currect_first_index))
        {
            firstSpkNode =
              found_terminal_dblist_node_by_addr(curFirstAlyAddr);
            if (firstSpkNode != (tmnl_pdblist)0) {
#ifdef FIRST_SPEAK_QUEUE_ENABLE
                if (Terminal_firstSpkRequestConnect(firstSpkNode,
                        &l_FSpkQueue, &l_FSpkLocker,
                        MAX_CONNECT_FAILURE_TIMES,
                        COMMON_SPK_PERMISSION))
                {
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_DEBUG,
                        "[ Limit(1) Mode Speak Post First apply]"
                        " speaking node(address = 0x%x) to Queue Success ]",
                        firstSpkNode->tmnl_dev.address.addr);
                }
                else {
                    /* set close, because it not in apply list */
                    terminal_mic_state_set(MIC_COLSE_STATUS,
                        firstSpkNode->tmnl_dev.address.addr,
                        firstSpkNode->tmnl_dev.entity_id, true, firstSpkNode);
                    
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_DEBUG,
                        "[ Limit(1) Mode Speak Post First apply]"
                        " speaking node(address = 0x%x)to Queue Failed ]",
                        firstSpkNode->tmnl_dev.address.addr);
                }

                /* set first apply whether current speak
                    successfully or not */
                terminal_apply_list_first_speak();
#else
                /* request connections */
                if (Terminal_requestConnect(firstSpkNode,
                        COMMON_PRIOR,
                        MAX_FAILURE_TIMES,
                        COMMON_SPK_PERMISSION))
                {
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_ERROR,
                        "[Limit First Aplly(0x04x) Request "
                        "connections success ]", curFirstAlyAddr);                    
                }
                else {
                    /* set close, because it not in apply list */
                    terminal_mic_state_set(MIC_COLSE_STATUS,
                        firstSpkNode->tmnl_dev.address.addr,
                        firstSpkNode->tmnl_dev.entity_id, true, firstSpkNode);
                    
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_ERROR,
                        "[Limit First Aplly(0x04x) Request "
                        "connections Failed ]", curFirstAlyAddr);
                }

                /* set first apply whether current speak
                    successfully or not */
                terminal_apply_list_first_speak();
#endif 
            }               
        }

        /* send main state */
        terminal_main_state_send(0, (void *)0, 0);
    }
}

/*$ Terminal_chairmanCloseMicCallback().....................................*/
int Terminal_chairmanCloseMicCallback(bool isSuccess,
            tmnl_pdblist user, uint32_t permissions)
{
    uint8_t micState;
    uint8_t disMode;
    uint8_t sysState;
    uint64_t entityId;
    uint16_t tAddr;
    uint8_t cLimitSpkNum;

    if (user == (tmnl_pdblist)0) {
        return 0;
    }
    
    /* for muti pthread access global data,
        must lock */
    INTERRUPT_LOCK(l_callBackLocker);
    
    micState = user->tmnl_dev.tmnl_status.mic_state;
    disMode = gdisc_flags.edis_mode;
    sysState = get_sys_state();
    entityId = user->tmnl_dev.entity_id;
    tAddr = user->tmnl_dev.address.addr;
    cLimitSpkNum = gdisc_flags.speak_limit_num;

    /* Close mic failed? */
    if (!isSuccess) {
        /* error discuss mode */
        if (sysState & permissions) {
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_ERROR,
                "[Chairman Terminal(0x%04x) Mic Close Failed: Out of"
                "disconnect times]",
                user->tmnl_dev.address.addr);
        }
        else {
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_ERROR,
                "[Chairman Terminal(0x%04x) Mic Close Failed: No"
                "Executable permissions]",
                user->tmnl_dev.address.addr);
        }
    }
    else {
        gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_DEBUG,
                "[Chairman Terminal(0x%04x) Mic Close Success]",
                user->tmnl_dev.address.addr);
        
        /* set mic status */
        terminal_mic_status_set_callback((bool)0, user);

        /* track camera */
        terminal_speak_track(tAddr, false);
        
        if (disMode == LIMIT_MODE) {
            Terminal_limitModeApplyTSpeak();
        }
    }

    /* for muti pthread access global data,
        must unlock */
    INTERRUPT_UNLOCK(l_callBackLocker);
    
    return 0;
}

/*$ Terminal_vipCloseMicCallback()..........................................*/
int Terminal_vipCloseMicCallback(bool isSuccess,
            tmnl_pdblist user, uint32_t permissions)
{
    uint8_t micState;
    uint8_t disMode;
    uint8_t sysState;
    uint64_t entityId;
    uint16_t tAddr;
    uint8_t cLimitSpkNum;

    if (user == (tmnl_pdblist)0) {
        return 0;
    }
    
    /* for muti pthread access global data,
        must lock */
    INTERRUPT_LOCK(l_callBackLocker);
    
    micState = user->tmnl_dev.tmnl_status.mic_state;
    disMode = gdisc_flags.edis_mode;
    sysState = get_sys_state();
    entityId = user->tmnl_dev.entity_id;
    tAddr = user->tmnl_dev.address.addr;
    cLimitSpkNum = gdisc_flags.speak_limit_num;
    
    /* Close mic failed? */
    if (!isSuccess) {
        /* error discuss mode */
        if (sysState & permissions) {
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_ERROR,
                "[Vip Terminal(0x%04x) Mic Close Failed: Out of"
                "disconnect times]",
                user->tmnl_dev.address.addr);
        }
        else {
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_ERROR,
                "[Vip Terminal(0x%04x) Mic Close Failed: No"
                "Executable permissions]",
                user->tmnl_dev.address.addr);
        }
    }
    else {
        gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_DEBUG,
                "[Vip Terminal(0x%04x) Mic Close Success]",
                user->tmnl_dev.address.addr);
        
        /* set mic status */
        terminal_mic_status_set_callback((bool)0, user);

        /* track camera */
        terminal_speak_track(tAddr, false);

        /* decrement for all discuss mode */
        if (gdisc_flags.vipSpkNum > 0) {
            addr_queue_delect_by_value(gdisc_flags.vipSpeakList,
                &gdisc_flags.vipSpkNum, tAddr);
        }
        
        if (disMode == LIMIT_MODE) {
            Terminal_limitModeApplyTSpeak();
        }
    }

    /* for muti pthread access global data,
        must unlock */
    INTERRUPT_UNLOCK(l_callBackLocker);
    
    return 0;
}

/*$ Terminal_commonCloseMicCallback().......................................*/
int Terminal_commonCloseMicCallback(bool isSuccess,
            tmnl_pdblist user, uint32_t permissions)
{
    uint8_t micState;
    uint8_t disMode;
    uint8_t sysState;
    uint64_t entityId;
    uint16_t tAddr;
    uint8_t cLimitSpkNum;

    if (user == (tmnl_pdblist)0) {
        return 0;
    }
    
    /* for muti pthread access global data,
        must lock */
    INTERRUPT_LOCK(l_callBackLocker);
    
    micState = user->tmnl_dev.tmnl_status.mic_state;
    disMode = gdisc_flags.edis_mode;
    sysState = get_sys_state();
    entityId = user->tmnl_dev.entity_id;
    tAddr = user->tmnl_dev.address.addr;
    cLimitSpkNum = gdisc_flags.speak_limit_num;
    
    /* Close mic failed? */
    if (!isSuccess) {
        /* error discuss mode */
        if (sysState & permissions) {
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_ERROR,
                "[Common Terminal(0x%04x) Mic Close Failed: Out of"
                "disconnect times]",
                user->tmnl_dev.address.addr);
        }
        else {
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_ERROR,
                "[Common Terminal(0x%04x) Mic Close Failed: No"
                "Executable permissions]",
                user->tmnl_dev.address.addr);
        }
    }
    else {
        gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_DEBUG,
                "[Common Terminal(0x%04x) Mic Close Success]",
                user->tmnl_dev.address.addr);
        
        /* set mic status */
        terminal_mic_status_set_callback((bool)0, user);

        /* track camera */
        terminal_speak_track(tAddr, false);

        /* decrement for all discuss mode */
        if (cLimitSpkNum > 0) {
            addr_queue_delect_by_value(gdisc_flags.speak_addr_list,
                &gdisc_flags.speak_limit_num, tAddr);
        }
        
        if (disMode == LIMIT_MODE) {
            Terminal_limitModeApplyTSpeak();
        }
    }

    /* for muti pthread access global data,
        must unlock */
    INTERRUPT_UNLOCK(l_callBackLocker);
    
    return 0;
}

/*$ terminal_common_speak().................................................*/
void terminal_common_speak(tmnl_pdblist dis_node, bool key_down) {
    uint8_t disMode;
    thost_system_set setSys;
    uint16_t cSpkNum;          /* current total speaking number in system */
    uint8_t cLimitSpkNum;      /*!current total limit person speaking number */
    uint8_t maxLimitSpkNum;    /*! system max limit speaking number  */
    bool isConnect, reSuccess; /*! request connect successfully? */
    uint64_t entityId;
    uint8_t micState;
    uint16_t tAddr;
    uint8_t applyNum;
    uint8_t applyMaxNum;
    uint8_t micTempState;
    uint16_t firstLimitSpk;
    tmnl_pdblist tn; /* temp node */
    bool fifoReqConnect = (bool)0;
    uint16_t firstApplyAddr;

    /* for muti pthread access global data, must lock */
    INTERRUPT_LOCK(l_callBackLocker);

    /* get conference pragma */
    maxLimitSpkNum = gdisc_flags.limit_num;
    cSpkNum = terminal_speak_num_count();
    cLimitSpkNum = gdisc_flags.speak_limit_num;
    disMode = gdisc_flags.edis_mode;
    memcpy(&setSys, &gset_sys, sizeof(thost_system_set));
    applyNum = gdisc_flags.apply_num;
    applyMaxNum = gdisc_flags.apply_limit;

    /* get terminal information */
    entityId = dis_node->tmnl_dev.entity_id;
    tAddr = dis_node->tmnl_dev.address.addr;
    micState = dis_node->tmnl_dev.tmnl_status.mic_state;

    /* apply for speaking? */
    if (key_down) {
        isConnect = trans_model_unit_is_connected(entityId);
        if (isConnect) {
            terminal_mic_state_set(MIC_OPEN_STATUS,
                        tAddr, entityId, true, dis_node);

            /* for muti pthread access global data,
                must unlock before return */
            INTERRUPT_UNLOCK(l_callBackLocker);
            
            /* process finish, return */
            return;
        }

        if (disMode == PPT_MODE) { /* ppt mode */
            if (cSpkNum >= CCRU_canUsedInStreams) {
                /* error discuss mode */
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_ERROR,
                    "[Common Terminal(0x%04x) Speak(PPT) Failed: Out of"
                    " max speak number"
                    "(maxLimitSpkNum = %d - cSpkNum = %d)]",
                    tAddr, CCRU_canUsedInStreams, cSpkNum);
            }
            else {
#ifdef FIRST_SPEAK_QUEUE_ENABLE
                if (Terminal_firstSpkRequestConnect(dis_node,
                        &l_FSpkQueue, &l_FSpkLocker,
                        MAX_CONNECT_FAILURE_TIMES,
                        COMMON_SPK_PERMISSION))
                {
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_DEBUG,
                        "[ PPT(1) Mode Speak Post First ]"
                        " speaking node(address = 0x%x)"
                        " to Queue Success ]",
                        dis_node->tmnl_dev.address.addr);
                }
                else {
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_DEBUG,
                        "[ PPT(1) Mode Speak Post First ]"
                        " speaking node(address = 0x%x)"
                        "to Queue Failed ]",
                        dis_node->tmnl_dev.address.addr);
                }
#else                
                reSuccess = Terminal_requestConnect(dis_node,
                    COMMON_PRIOR, MAX_FAILURE_TIMES,
                    COMMON_SPK_PERMISSION);
                if (reSuccess) {
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_DEBUG,
                        "[Common Terminal(0x%04x) Speak(PPT) "
                        " Request connection success: Waitting for"
                        "Connection...]", tAddr);
                }
                else {
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_ERROR,
                        "[Common Terminal(0x%04x) Speak(PPT) "
                        " Request connection Faied]");
                }
#endif /* FIRST_SPEAK_QUEUE_ENABLE */
            }
        }
        else if (disMode == LIMIT_MODE) { /* limit mode */
            if ((cLimitSpkNum < maxLimitSpkNum)
                    && (cSpkNum < CCRU_canUsedInStreams))
            {
#ifdef FIRST_SPEAK_QUEUE_ENABLE
                if (Terminal_firstSpkRequestConnect(dis_node,
                        &l_FSpkQueue, &l_FSpkLocker,
                        MAX_CONNECT_FAILURE_TIMES,
                        COMMON_SPK_PERMISSION))
                {
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_DEBUG,
                        "[ LIMIT(1) Mode Speak Post First ]"
                        " speaking node(address = 0x%x)"
                        " to Queue Success ]",
                        dis_node->tmnl_dev.address.addr);
                }
                else {
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_DEBUG,
                        "[ LIMIT(1) Mode Speak Post First ]"
                        " speaking node(address = 0x%x)"
                        "to Queue Failed ]",
                        dis_node->tmnl_dev.address.addr);
                }
#else        
                reSuccess = Terminal_requestConnect(dis_node,
                        COMMON_PRIOR, MAX_FAILURE_TIMES,
                        COMMON_SPK_PERMISSION);
                if (reSuccess) {
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_DEBUG,
                        "[Common Terminal(0x%04x) Speak(LIMIT) "
                        " Request connection success: Waitting for"
                        "Connection...]", tAddr);
                }
                else {
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_DEBUG,
                        "[Common Terminal(0x%04x) Speak(LIMIT) "
                        " Request connection Faied]", tAddr);
                }
#endif /* FIRST_SPEAK_QUEUE_ENABLE */
            }
            else {
#ifdef FIRST_SPEAK_QUEUE_ENABLE                
                /* set mic for apply state */
                if ((applyNum < applyMaxNum)
                      && (QueueCom_isEmpty(&l_FSpkQueue))
                      && (QueueCom_isEmpty(&l_vipFSpkQueue))
                      && (QueueCom_isEmpty(&l_chairmanFSpkQueue)))
                {
                    micTempState = MIC_OTHER_APPLY_STATUS;
                    if (applyNum == 0) {
                        gdisc_flags.currect_first_index = 0;
                        micTempState = MIC_FIRST_APPLY_STATUS;
                    }

                    /* post to apply list */
                    gdisc_flags.apply_addr_list[applyNum] = tAddr;
                    gdisc_flags.apply_num++;

                    /* set and report mic state */
                    terminal_mic_state_set(micTempState,
                        tAddr, entityId, true, dis_node);
                    
                    terminal_main_state_send(0, (void *)0, 0);

                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_DEBUG,
                        "[Common Terminal(0x%04x) Speak(LIMIT) "
                        " Appling ]", tAddr);
                }
                else if (applyNum < applyMaxNum) {
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_ERROR,
                        "[Common Terminal(0x%04x) Speak(LIMIT) "
                        " Apply Failed: first apply queue has"
                        "speaking task]", tAddr);
                }
                else {
                    /* do nothing */
                }
#else
                /* set mic for apply state */
                if (applyNum < applyMaxNum) {
                    micTempState = MIC_OTHER_APPLY_STATUS;
                    if (applyNum == 0) {
                        gdisc_flags.currect_first_index = 0;
                        micTempState = MIC_FIRST_APPLY_STATUS;
                    }

                    /* post to apply list */
                    gdisc_flags.apply_addr_list[applyNum] = tAddr;
                    gdisc_flags.apply_num++;

                    /* set and report mic state */
                    terminal_mic_state_set(micTempState,
                        tAddr, entityId, true, dis_node);
                    
                    terminal_main_state_send(0, (void *)0, 0);

                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_DEBUG,
                        "[Common Terminal(0x%04x) Speak(LIMIT) "
                        " Appling ]", tAddr);
                }
#endif /* FIRST_SPEAK_QUEUE_ENABLE */
            }
        }
        else if (disMode == FIFO_MODE) { /* Fifo mode */
            if ((cLimitSpkNum < maxLimitSpkNum)
                  && (cSpkNum < CCRU_canUsedInStreams))
            {
                /* request connection */
                fifoReqConnect = (bool)1;

                firstLimitSpk = tAddr;
            }
            else {
                /* check speaking list */
                firstLimitSpk = gdisc_flags.speak_addr_list[0];
                if (firstLimitSpk != 0xffff) {
                    /* First in First out */
                    tn = found_terminal_dblist_node_by_addr(firstLimitSpk);
                    if (tn != (tmnl_pdblist)0) {
                        reSuccess = Terminal_requestDisConnect(tn,
                                COMMON_PRIOR, MAX_FAILURE_TIMES,
                                COMMON_SPK_PERMISSION);
                        if (reSuccess) {
                            fifoReqConnect = (bool)1;
                            
                            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                                LOGGING_LEVEL_DEBUG,
                                "[Common Terminal(0x%04x) Speak(FIFO) "
                                " Request disconnection success]",
                                firstLimitSpk);
                        }
                        else {
                            fifoReqConnect = (bool)0;
                            
                            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                                LOGGING_LEVEL_ERROR,
                                "[Common Terminal(0x%04x) Speak(FIFO) "
                                " Request disconnection failed]",
                                firstLimitSpk);
                        }
                    }
                }
            }

            if (fifoReqConnect) {
#ifdef FIRST_SPEAK_QUEUE_ENABLE
                if (Terminal_firstSpkRequestConnect(dis_node,
                        &l_FSpkQueue, &l_FSpkLocker,
                        MAX_CONNECT_FAILURE_TIMES,
                        COMMON_SPK_PERMISSION))
                {
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_DEBUG,
                        "[ FIFO(1) Mode Speak Post First ]"
                        " speaking node(address = 0x%x)"
                        " to Queue Success ]",
                        dis_node->tmnl_dev.address.addr);
                }
                else {
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_DEBUG,
                        "[ FIFO(1) Mode Speak Post First ]"
                        " speaking node(address = 0x%x)"
                        "to Queue Failed ]",
                        dis_node->tmnl_dev.address.addr);
                }
#else        
                /* request */
                reSuccess = Terminal_requestConnect(dis_node,
                        COMMON_PRIOR, MAX_FAILURE_TIMES,
                        COMMON_SPK_PERMISSION);
                if (reSuccess) {
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_DEBUG,
                        "[Common Terminal(0x%04x) Speak(FIFO) "
                        " Request connection success: Waitting for"
                        "Connection...]", firstLimitSpk);
                }
                else {
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_ERROR,
                        "[Common Terminal(0x%04x) Speak(FIFO) "
                        " Request connection Faied]", firstLimitSpk);
                }
#endif /* FIRST_SPEAK_QUEUE_ENABLE  */
            }
            else {
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_ERROR,
                    "[Common Terminal(0x%04x) Speak(FIFO) "
                    " Speak Failed: No request connection]",
                    firstLimitSpk);
            }
        }
        else if (disMode == APPLY_MODE) { /* apply mode */
            if (applyNum < applyMaxNum) {
                micTempState = MIC_OTHER_APPLY_STATUS;
                if (applyNum == 0) {
                    gdisc_flags.currect_first_index = 0;
                    micTempState = MIC_FIRST_APPLY_STATUS;
                }

                /* post to apply list */
                gdisc_flags.apply_addr_list[applyNum] = tAddr;
                gdisc_flags.apply_num++;

                /* set and report mic state */
                terminal_mic_state_set(micTempState,
                    tAddr, entityId, true, dis_node);
                
                terminal_main_state_send(0, (void *)0, 0);

                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_DEBUG,
                    "[Common Terminal(0x%04x) Speak(Apply) "
                    " apply Success ]");
            }
            else {
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_DEBUG,
                    "[Common Terminal(0x%04x) Speak(Apply) "
                    " apply Failed: Out of max apply number]");
            }
        }
        else {
            /* error discuss mode */
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_ERROR,
                "[Common Terminal Speak Failed: Error Discuss Mode]");
        }
    }
    else { /* cancel speaking */
        /* chek whether connected? */
        isConnect = trans_model_unit_is_connected(entityId);
        if (!isConnect) {
            /* set current addr to close */
            terminal_mic_state_set(MIC_COLSE_STATUS,
                            tAddr, entityId, true, dis_node);
            
            /* special process in apply mode */
            if (disMode == APPLY_MODE) {
                uint8_t curFindex = gdisc_flags.currect_first_index;
                
                firstApplyAddr = gdisc_flags.apply_addr_list[curFindex];
                if(addr_queue_delect_by_value(gdisc_flags.apply_addr_list,
                    &gdisc_flags.apply_num, tAddr))
                {   /* terminal apply */
                    addr_queue_delect_by_value(gdisc_flags.apply_addr_list,
                        &gdisc_flags.apply_num, tAddr);
                    
                    if ((gdisc_flags.apply_num > 0)
                        && (firstApplyAddr == tAddr))
                    {/* current cancel address is first apply terminal */
                        gdisc_flags.currect_first_index %= gdisc_flags.apply_num;
                        
                        tn = found_terminal_dblist_node_by_addr(
                                gdisc_flags.apply_addr_list[curFindex]);
                        if (tn != (tmnl_pdblist)0) {
                            /* set next to first apply */
                            terminal_mic_state_set(MIC_FIRST_APPLY_STATUS,
                                tn->tmnl_dev.address.addr,
                                tn->tmnl_dev.entity_id, true, tn);
                        }
                    }

                    terminal_main_state_send(0, (void *)0, 0);
                }
            }
            else if (disMode == LIMIT_MODE) {
                uint8_t curFindex = gdisc_flags.currect_first_index;
                
                firstApplyAddr = gdisc_flags.apply_addr_list[curFindex];
                if ((micState == MIC_FIRST_APPLY_STATUS)
                      || (micState == MIC_OTHER_APPLY_STATUS))
                {
                    addr_queue_delect_by_value(gdisc_flags.apply_addr_list,
                        &gdisc_flags.apply_num, tAddr);

                    terminal_mic_state_set(MIC_COLSE_STATUS,
                                    tAddr, entityId, true, dis_node);
                    
                    if ((gdisc_flags.apply_num > 0)
                          && (firstApplyAddr == firstApplyAddr))
                    {
                        gdisc_flags.currect_first_index %=
                        gdisc_flags.apply_num;

                        /* get next to first apply */
                        tn = found_terminal_dblist_node_by_addr(
                        gdisc_flags.apply_addr_list[curFindex]);
                        if (tn != (tmnl_pdblist)0) {
                            terminal_mic_state_set(MIC_FIRST_APPLY_STATUS,
                                    tn->tmnl_dev.address.addr,
                                    tn->tmnl_dev.entity_id, true, tn);
                        }
                    }

                    terminal_main_state_send(0, (void *)0, 0);
                }
            }
            else {
                /* do nothing */
            }
        }
        else {
            reSuccess = Terminal_requestDisConnect(dis_node,
                    COMMON_PRIOR, MAX_FAILURE_TIMES,
                    COMMON_SPK_PERMISSION);
            if (reSuccess) {                
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_DEBUG,
                    "[Common Terminal(0x%04x) Speak(FIFO) "
                    " Request disconnection success]",
                    tAddr);
            }
            else {
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_ERROR,
                    "[Common Terminal(0x%04x) Speak(FIFO) "
                    " Request disconnection failed]",
                    tAddr);
            }
        }
    }
    
    /* for muti pthread access global data,
        must unlock */
    INTERRUPT_UNLOCK(l_callBackLocker);
}

/*$ terminal_vip_speak()....................................................*/
void terminal_vip_speak(tmnl_pdblist dis_node, bool key_down) {
    uint8_t disMode;
    thost_system_set setSys;
    uint16_t cSpkNum;           /* current total speaking number in system */
    uint8_t cLimitSpkNum;       /*!current total limit person speaking number */
    uint8_t maxLimitSpkNum;     /*! system limit max speaking number  */
    bool reSuccess;             /*! request connect successfully? */
    uint64_t entityId;
    uint8_t micState;
    uint16_t tAddr;
    uint8_t applyNum;
    uint8_t applyMaxNum;
    uint16_t firstLimitSpk;
    tmnl_pdblist tn; /* temp node */
    bool reqConnect = (bool)0;

    /* for muti pthread access global data, must lock */
    INTERRUPT_LOCK(l_callBackLocker);

    /* get conference pragma */
    maxLimitSpkNum = gdisc_flags.limit_num;
    cSpkNum = terminal_speak_num_count();
    cLimitSpkNum = gdisc_flags.speak_limit_num;
    disMode = gdisc_flags.edis_mode;
    memcpy(&setSys, &gset_sys, sizeof(thost_system_set));
    applyNum = gdisc_flags.apply_num;
    applyMaxNum = gdisc_flags.apply_limit;

    /* get terminal information */
    entityId = dis_node->tmnl_dev.entity_id;
    tAddr = dis_node->tmnl_dev.address.addr;
    micState = dis_node->tmnl_dev.tmnl_status.mic_state;    

    if (key_down) {
        if ((cSpkNum < CCRU_canUsedInStreams)
            && (!trans_model_unit_is_connected(entityId)))
        {
            reqConnect = (bool)1;
        }
        else if ((cSpkNum >= CCRU_canUsedInStreams)
            && (!trans_model_unit_is_connected(entityId)))
        {
            /* get disconnection address */
            Terminal_getDisconnectAddress(&firstLimitSpk, COMMON_PRIOR,
                    gdisc_flags.speak_addr_list, gdisc_flags.speak_limit_num);
            
            /* check value */
            if (firstLimitSpk != 0xffff) {
                /* First in First out */
                tn = found_terminal_dblist_node_by_addr(firstLimitSpk);
                if (tn != (tmnl_pdblist)0) {
                    reSuccess = Terminal_requestDisConnect(tn,
                            COMMON_PRIOR, MAX_FAILURE_TIMES,
                            COMMON_SPK_PERMISSION);
                    if (reSuccess) {
                        reqConnect = (bool)1;
                        
                        gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                            LOGGING_LEVEL_DEBUG,
                            "[VIP Terminal(0x%04x) Speak "
                            " Request disconnection(address = 0x%x)success]",
                            tAddr, firstLimitSpk);
                    }
                    else {
                        reqConnect = (bool)0;
                        
                        gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                            LOGGING_LEVEL_ERROR,
                            "[VIP Terminal(0x%04x) Speak "
                            " Request disconnection failed]",
                            firstLimitSpk);
                    }
                }
            }      
        }
        else {/* has connected */
            /* open mic */
            terminal_mic_state_set(MIC_OPEN_STATUS,
                        tAddr, entityId, true, dis_node);
            
            reqConnect = (bool)0;
        }

        if (reqConnect) {
#ifdef FIRST_SPEAK_QUEUE_ENABLE
            if (Terminal_firstSpkRequestConnect(dis_node,
                    &l_vipFSpkQueue, &l_FSpkLocker,
                    MAX_CONNECT_FAILURE_TIMES,
                    COMMON_SPK_PERMISSION))
            {
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_DEBUG,
                    "[ VIP(1) Speak Post First"
                    " speaking node(address = 0x%x)"
                    " to Queue Success ]",
                    dis_node->tmnl_dev.address.addr);
            }
            else {
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_ERROR,
                    "[ VIP(1) Speak Post First"
                    " speaking node(address = 0x%x)"
                    "to Queue Failed ]",
                    dis_node->tmnl_dev.address.addr);
            }
#else
            /* request connection */
            reSuccess = Terminal_requestConnect(dis_node,
                VIP_PRIOR, MAX_FAILURE_TIMES,
                COMMON_SPK_PERMISSION);
            if (reSuccess) {
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_DEBUG,
                    "[VIP Terminal(0x%04x) Speak "
                    " Request connection success: Waitting for"
                    "Connection...]", tAddr);
            }
            else {
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_ERROR,
                    "[VIP Terminal(0x%04x) Speak "
                    " Request connection Faied]", tAddr);
            }
#endif            
        }
        else {
            /* error discuss mode */
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_ERROR,
                "[VIP Terminal(0x%04x) Speak Failed: Out of"
                " max speak number, or is connected "
                "(maxLimitSpkNum = %d - cSpkNum = %d)]",
                tAddr, CCRU_canUsedInStreams, cSpkNum);
        }
    }
    else  {
        if (trans_model_unit_is_connected(entityId)) {
            /* request connection */
            reSuccess = Terminal_requestDisConnect(dis_node,
                VIP_PRIOR, MAX_FAILURE_TIMES,
                COMMON_SPK_PERMISSION);
            if (reSuccess) {
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_DEBUG,
                    "[VIP Terminal(0x%04x) Close "
                    " Request disconnection success: Waitting for"
                    " disConnection...]", tAddr);
            }
            else {
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_ERROR,
                    "[VIP Terminal(0x%04x) Close "
                    " Request disconnection Faied]", tAddr);
            }
        }
        else {/* has not connected */
            /* close mic */
            terminal_mic_state_set(MIC_COLSE_STATUS,
                        tAddr, entityId, true, dis_node);
        }
    }

    /* for muti pthread access global data, must unlock */
    INTERRUPT_UNLOCK(l_callBackLocker);
}

/*$ terminal_chairman_speak()...............................................*/
void terminal_chairman_speak(tmnl_pdblist dis_node, bool key_down) {
    uint8_t disMode;
    thost_system_set setSys;
    uint16_t cSpkNum;           /* current total speaking number in system */
    uint8_t cLimitSpkNum;       /*!current total limit person speaking number */
    uint8_t maxLimitSpkNum;     /*! system max limit speaking number  */
    bool reSuccess;             /*! request connect successfully? */
    uint64_t entityId;
    uint8_t micState;
    uint16_t tAddr;
    uint8_t applyNum;
    uint8_t applyMaxNum;
    bool reqConnect = (bool)0;

    /* for muti pthread access global data, must lock */
    INTERRUPT_LOCK(l_callBackLocker);

    /* get conference pragma */
    maxLimitSpkNum = gdisc_flags.limit_num;
    cSpkNum = terminal_speak_num_count();
    cLimitSpkNum = gdisc_flags.speak_limit_num;
    disMode = gdisc_flags.edis_mode;
    memcpy(&setSys, &gset_sys, sizeof(thost_system_set));
    applyNum = gdisc_flags.apply_num;
    applyMaxNum = gdisc_flags.apply_limit;

    /* get terminal information */
    entityId = dis_node->tmnl_dev.entity_id;
    tAddr = dis_node->tmnl_dev.address.addr;
    micState = dis_node->tmnl_dev.tmnl_status.mic_state;    

    if (key_down) {
        if (!trans_model_unit_is_connected(entityId)) {
            /* check has channel */
            reqConnect = Terminal_chairmanInSpeakSuccess(cSpkNum,
                                    COMMON_SPK_PERMISSION);
        }
        else {/* has connected */
            /* open mic */
            terminal_mic_state_set(MIC_OPEN_STATUS,
                        tAddr, entityId, true, dis_node);
            
            reqConnect = (bool)0;
        }

        if (reqConnect) {
#ifdef FIRST_SPEAK_QUEUE_ENABLE
            if (Terminal_firstSpkRequestConnect(dis_node,
                    &l_chairmanFSpkQueue, &l_FSpkLocker,
                    MAX_CONNECT_FAILURE_TIMES,
                    COMMON_SPK_PERMISSION))
            {
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_DEBUG,
                    "[ Chairman(1) Speak Post First ]"
                    " speaking node(address = 0x%x)"
                    " to Queue Success ]",
                    dis_node->tmnl_dev.address.addr);
            }
            else {
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_ERROR,
                    "[ Chairman(1) Speak Post First ]"
                    " speaking node(address = 0x%x)"
                    "to Queue Failed ]",
                    dis_node->tmnl_dev.address.addr);
            }
#else            
            /* request connection */
            reSuccess = Terminal_requestConnect(dis_node,
                CHAIRMAN_PRIOR, MAX_FAILURE_TIMES,
                COMMON_SPK_PERMISSION);
            if (reSuccess) {
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_DEBUG,
                    "[Chairman Terminal(0x%04x) Speak "
                    " Request connection success: Waitting for"
                    "Connection...]", tAddr);
            }
            else {
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_ERROR,
                    "[Chairman Terminal(0x%04x) Speak "
                    " Request connection Faied]", tAddr);
            }
#endif            
        }
        else {
            /* error discuss mode */
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_ERROR,
                "[Chairman Terminal(0x%04x) Speak Failed: Out of"
                " max speak number, or is connected "
                "(maxLimitSpkNum = %d - cSpkNum = %d)]",
                tAddr, CCRU_canUsedInStreams, cSpkNum);
        }
    }
    else  {
        if (trans_model_unit_is_connected(entityId)) {
            /* request connection */
            reSuccess = Terminal_requestDisConnect(dis_node,
                CHAIRMAN_PRIOR, MAX_FAILURE_TIMES,
                COMMON_SPK_PERMISSION);
            if (reSuccess) {
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_DEBUG,
                    "[Chairman Terminal(0x%04x) Close "
                    " Request disconnection success: Waitting for"
                    "disConnection...]", tAddr);
            }
            else {
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_ERROR,
                    "[Chairman Terminal(0x%04x) Close "
                    " Request disconnection Faied]", tAddr);
            }
        }
        else {/* has not connected */
            /* close mic */
            terminal_mic_state_set(MIC_COLSE_STATUS,
                        tAddr, entityId, true, dis_node);
        }
    }
    
    /* for muti pthread access global data, must unlock */
    INTERRUPT_UNLOCK(l_callBackLocker);
}

/*$ terminal_key_speak_proccess()...........................................*/
bool terminal_key_speak_proccess(tmnl_pdblist dis_node,
    bool key_down, uint8_t recv_msg)
{
#ifndef DISABLE_KEY_OPT_TIMEOUT
    uint32_t cur_time, lastTime;
#endif /* DISABLE_KEY_OPT_TIMEOUT */
    uint8_t tType; /* terminal type */
    uint8_t user;
    TEReqQePrior prior;
    uint16_t tAddr;
    bool rightType, isConnect, willCancel;
    uint64_t entityId;
    TComQueue *pQueue;

    if (!dis_node->tmnl_dev.tmnl_status.is_rgst) {
        return false;
    }
    
    /* reply same message for all dismode and terminal */
    terminal_key_action_host_common_reply(recv_msg, dis_node);

#ifndef DISABLE_KEY_OPT_TIMEOUT
    cur_time = get_current_time();
    lastTime = dis_node->tmnl_dev.spk_operate_timp;
    dis_node->tmnl_dev.spk_operate_timp = cur_time;
    if (((cur_time - lastTime) < SPK_KEY_OPT_TIME)
          && (gdisc_flags.edis_mode != PPT_MODE))
    {
        gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_ERROR,
                "[Terminal speak failed: key operation too fast]");
        
        return false;
    }
#endif /* DISABLE_KEY_OPT_TIMEOUT */

    entityId = dis_node->tmnl_dev.entity_id;
    tAddr = dis_node->tmnl_dev.address.addr;
    tType = dis_node->tmnl_dev.address.tmn_type;
    
    rightType = (bool)1;
    if (tType == TMNL_TYPE_COMMON_RPRST) {
        prior = COMMON_PRIOR;
        pQueue = &l_FSpkQueue;
    }
    else if (tType == TMNL_TYPE_VIP) {
        prior = VIP_PRIOR;
        pQueue = &l_vipFSpkQueue;
    }
    else if ((tType == TMNL_TYPE_CHM_EXCUTE)
                  || (tType == TMNL_TYPE_CHM_COMMON))
    {
        prior = CHAIRMAN_PRIOR;
        pQueue = &l_chairmanFSpkQueue;
    }
    else {
        /* error type */
        rightType = (bool)0;
    }

    /* if no task in the queue(connector or disconnector)
        process, otherwise discard this key command */
    if ((!rightType)
          || (Terminal_hasTask(prior, tAddr))
          || (terminal_firstSpeakQueueHasTask(pQueue, tAddr)))
    {
        /* cancel task? */
        willCancel = (bool)0;
        
        if (rightType) {
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_ERROR,
                "[Terminal(0x%04x) has Task in"
                "  prior(%d) Queue ]", tAddr, prior);

            isConnect = trans_model_unit_is_connected(entityId);
            
            if (key_down) {
                /* canecl disconnect task */
                user = DISCONNECTOR;

                if (isConnect) {
                    willCancel = (bool)1;
                }
            }
            else {
                /* canecl connect task */
                user = CONNECTOR;
                
                if (!isConnect) {
                    willCancel = (bool)1;
                }
            }

            if (willCancel) {
                if (Terminal_cancelTask(user, prior, tAddr)) {
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_ERROR,
                        "[Terminal(0x%04x) cancel user(%d) Task"
                        "  prior(%d) Queue success ]", tAddr, user, prior);
                }
                else {
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_ERROR,
                        "[Terminal(0x%04x) cancel user(%d) Task"
                        "  prior(%d) Queue Error ]", tAddr, user, prior);
                }

                /* cancel task, zero meaning cancel */
                if (Terminal_changeFirstSpkTaskPer(tAddr,
                        pQueue, &l_FSpkLocker, 0U))
                {
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_ERROR,
                        "[Terminal(0x%04x) cancel connections Task"
                        "  prior(%d) Queue success ]", tAddr, prior);
                }
                else {
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_ERROR,
                        "[Terminal(0x%04x) cancel connections Task"
                        "  prior(%d) Queue Error ]", tAddr, prior);
                }
            }
        }
        else {
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_ERROR,
                "[ No Right Type ]");
        }

        return false;
    }
    
    switch (tType) {
        /* common preset terminal frequency operation */
        case TMNL_TYPE_COMMON_RPRST: {
            terminal_common_speak(dis_node, key_down);
            break;
        }
        case TMNL_TYPE_VIP: {
            terminal_vip_speak(dis_node, key_down);
            break;
        }
        case TMNL_TYPE_CHM_EXCUTE:
        case TMNL_TYPE_CHM_COMMON: {
            terminal_chairman_speak(dis_node, key_down);
            break;
        }
        default: {
            /* error type of terminal */
            break;
        }
    }
    
    return true;
}
#else
bool terminal_key_speak_proccess( tmnl_pdblist dis_node, bool key_down, uint8_t recv_msg )
{
	uint8_t dis_mode = gdisc_flags.edis_mode;
	thost_system_set set_sys; // 系统配置文件的格式
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));
	uint32_t cur_time = get_current_time();

	if( dis_node == NULL )
		return false;

	if( !dis_node->tmnl_dev.tmnl_status.is_rgst )
		return false;
	
	if ((cur_time - dis_node->tmnl_dev.spk_operate_timp) < SPK_KEY_OPT_TIME)
	{
		DEBUG_INFO("speaking key press not time out, do not do that fast");
		terminal_key_action_host_special_num1_reply( recv_msg, dis_node->tmnl_dev.tmnl_status.mic_state, dis_node );
		dis_node->tmnl_dev.spk_operate_timp = cur_time;
		return false;
	}
	
	dis_node->tmnl_dev.spk_operate_timp = cur_time;
	terminal_pro_debug( "dis mode = %d ", dis_mode );
	DEBUG_INFO("spking num = %d apply num = %d=====================", gdisc_flags.speak_limit_num, gdisc_flags.apply_num);
	if( dis_mode == PPT_MODE ||\
			(dis_node->tmnl_dev.address.tmn_type == TMNL_TYPE_VIP) ||\
			(dis_node->tmnl_dev.address.tmn_type == TMNL_TYPE_CHM_COMMON)||\
			(dis_node->tmnl_dev.address.tmn_type == TMNL_TYPE_CHM_EXCUTE))
	{	
		if(key_down)
		{
			if (trans_model_unit_is_connected(dis_node->tmnl_dev.entity_id))
			{// already connected
				if (dis_node->tmnl_dev.tmnl_status.mic_state != MIC_OPEN_STATUS)
					dis_node->tmnl_dev.tmnl_status.mic_state = MIC_OPEN_STATUS;
				
				terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, dis_node );
			}
			else
			{
				/*主席在发言上限里*/
				uint16_t speak_num = terminal_speak_num_count();
				int ret_cnnt = -1;
				if (speak_num < gdisc_flags.limit_num)
				{
					ret_cnnt = trans_model_unit_connect(dis_node->tmnl_dev.entity_id, dis_node);
				}
				
				if ((speak_num < gdisc_flags.limit_num) && (ret_cnnt == 0))
				{
                                        terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, dis_node );
                                        terminal_speak_track(dis_node->tmnl_dev.address.addr, true );
				}
				else if ((speak_num < gdisc_flags.limit_num) && (ret_cnnt != -2))
				{//has timeout for operation transmit ouput channel
					terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, dis_node );

					/*
					  *1\断开连接时间最长的
					  *2\超时后连接
					  */
					tmnl_pdblist disconnect_lnode = NULL;
					uint64_t id = 0;
					int ret = -1;
					bool if_common = false;

					ret = Ctrans_disLongest(&id, &disconnect_lnode);
					if (0 == ret)
					{
						if (disconnect_lnode != NULL && \
							disconnect_lnode->tmnl_dev.address.tmn_type == TMNL_TYPE_COMMON_RPRST)
						{
							if_common = true;
						}
						else if (disconnect_lnode == NULL && id != 0)
						{
							disconnect_lnode = found_terminal_dblist_node_by_endtity_id(id);
							if (disconnect_lnode != NULL && disconnect_lnode->tmnl_dev.address.tmn_type == TMNL_TYPE_COMMON_RPRST)
							{
								if_common = true;
							}
						}

						if (if_common)
						{
							if (dis_mode == FIFO_MODE)
								addr_queue_delect_by_value(gdisc_flags.speak_addr_list, &gdisc_flags.speak_limit_num, disconnect_lnode->tmnl_dev.address.addr);
							else if (dis_mode == LIMIT_MODE)
								gdisc_flags.speak_limit_num--;
						}

						terminal_over_time_speak_node_set(dis_node);
						if (NULL != disconnect_lnode)
							DEBUG_INFO("tmnl type = %d disconnect add = 0x%02x  spking num = %d====================1", 
								disconnect_lnode->tmnl_dev.address.tmn_type, 
								disconnect_lnode->tmnl_dev.address.addr, 
								gdisc_flags.speak_limit_num);
					}
				}
				else if (speak_num >= gdisc_flags.limit_num)
				{
                                        if (0 != speak_num)
                                        {
                                                terminal_key_action_host_special_num1_reply(recv_msg, MIC_COLSE_STATUS, dis_node);
                                        }
                                        else
                                        {
                                                terminal_key_action_host_special_num1_reply(recv_msg, MIC_COLSE_STATUS, dis_node);  
                                        }
					/*
					  *1\断开连接时间最长的
					  *2\超时后连接
					  */
					tmnl_pdblist disconnect_lnode = NULL;
					uint64_t id = 0;
					int ret = -1;
					bool if_common = false;
					ret = Ctrans_disLongest(&id, &disconnect_lnode);
					if (0 == ret)
					{
						if (disconnect_lnode != NULL && \
							disconnect_lnode->tmnl_dev.address.tmn_type == TMNL_TYPE_COMMON_RPRST)
						{
							if_common = true;
						}
						else if (disconnect_lnode == NULL && id != 0)
						{
							disconnect_lnode = found_terminal_dblist_node_by_endtity_id(id);
							if (disconnect_lnode != NULL && disconnect_lnode->tmnl_dev.address.tmn_type == TMNL_TYPE_COMMON_RPRST)
							{
								if_common = true;
							}
						}

						if (if_common)
						{
							if (dis_mode == FIFO_MODE)
								addr_queue_delect_by_value(gdisc_flags.speak_addr_list, &gdisc_flags.speak_limit_num, disconnect_lnode->tmnl_dev.address.addr);
							else if (dis_mode == LIMIT_MODE)
								gdisc_flags.speak_limit_num--;
						}

						terminal_over_time_speak_node_set(dis_node);
						if (NULL != disconnect_lnode)
							DEBUG_INFO("tmnl type = %d disconnect add = 0x%02x  spking num = %d====================1", 
								disconnect_lnode->tmnl_dev.address.tmn_type, 
								disconnect_lnode->tmnl_dev.address.addr, 
								gdisc_flags.speak_limit_num);
					}
				}
			}
		}
		else
		{
			if (trans_model_unit_is_connected(dis_node->tmnl_dev.entity_id))
			{
				if (0 == trans_model_unit_disconnect( dis_node->tmnl_dev.entity_id, dis_node ))
				{
#if 0
                                        terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, dis_node );
#else
                                        terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, dis_node );
#endif
                                        terminal_speak_track(dis_node->tmnl_dev.address.addr, false );

					/*主席\vip控制限制模式下的普通终端的mic状态*/
					if (dis_mode == LIMIT_MODE)
					{
						terminal_chman_vip_control_common_mic();
					}
				}
				else
				{
					terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, dis_node );
				}
			}
			else
			{
				terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, dis_node );
			}
		}
	}
	else
	{
		switch( dis_mode )
		{
			case LIMIT_MODE:
			{
				terminal_limit_disccuss_mode_pro( key_down, set_sys.spk_limtime, dis_node, recv_msg );
				break;
			}
			case FIFO_MODE:
			{
				terminal_fifo_disccuss_mode_pro( key_down, set_sys.spk_limtime, dis_node, recv_msg );
				break;
			}
			case APPLY_MODE:
			{
				terminal_apply_disccuss_mode_pro( key_down, set_sys.spk_limtime, dis_node, recv_msg );
				break;
			}
			default:
			{
				terminal_pro_debug( " out of discuss mode bound!" );
				break;
			}
		}
	}
	
	return true;
}
#endif /* MIC_PRIOR_MANEGER_ENABLE */

int terminal_chairman_apply_reply(uint8_t tmnl_type, uint16_t addr,
                                                                     uint8_t key_num, uint8_t key_value, 
                                                                     uint8_t tmnl_state, uint8_t recv_msg)
{
	tmnl_pdblist apply_node;
	
	apply_node = found_terminal_dblist_node_by_addr( addr );
	if( apply_node == NULL )
	{
		return -1;
	}
	
	if( tmnl_type != COMMAND_TMN_CHAIRMAN )
	{
		return -1;
	}

	if ((tmnl_state == TMNL_SYS_STA_VOTE)
              || (tmnl_state == TMNL_SYS_STA_GRADE)
              || (tmnl_state == TMNL_SYS_STA_SELECT))
	{
		return -1;
	}

	enum_apply_pro apply_flag = REFUSE_APPLY;
	if( (key_num == KEY2_VOTE) && key_value )
	{
		apply_flag = REFUSE_APPLY;
		terminal_key_action_host_common_reply( recv_msg, apply_node );
		terminal_examine_apply( apply_flag );
	}
	else if( (key_num == KEY3_VOTE) && key_value )
	{
		apply_flag = NEXT_APPLY;
		terminal_key_action_host_common_reply( recv_msg, apply_node );
		terminal_examine_apply( apply_flag );
	}
	else if( (key_num == KEY4_VOTE) && key_value )
	{
		apply_flag = APPROVE_APPLY;
		terminal_key_action_host_common_reply( recv_msg, apply_node );
		terminal_examine_apply( apply_flag );
	}

	return 0;
}

void terminal_free_disccuss_mode_pro(bool key_down, uint8_t limit_time,
                                                                             tmnl_pdblist speak_node,
                                                                             uint8_t recv_msg)
{
	assert( speak_node );
	if( speak_node == NULL )
		return;
	
	if( key_down )
	{
		if( gdisc_flags.speak_limit_num < FREE_MODE_SPEAK_MAX )
		{
			terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, speak_node );
			trans_model_unit_connect( speak_node->tmnl_dev.entity_id, speak_node );
			terminal_speak_track(speak_node->tmnl_dev.address.addr, true );
		}
	}
	else
	{
		terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );
		trans_model_unit_disconnect( speak_node->tmnl_dev.entity_id, speak_node );
		terminal_speak_track(speak_node->tmnl_dev.address.addr, false );
	}

	terminal_main_state_send( 0, NULL, 0 );
}

void terminal_apply_list_first_speak(void) {
    tmnl_pdblist first_apply = (tmnl_pdblist)0;

    if (gdisc_flags.apply_num > 0 ) {
        gdisc_flags.currect_first_index %= gdisc_flags.apply_num;
        
        first_apply = found_terminal_dblist_node_by_addr(
            gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index]);
        if (first_apply != NULL) {
            terminal_mic_state_set(MIC_FIRST_APPLY_STATUS,
                first_apply->tmnl_dev.address.addr,
                first_apply->tmnl_dev.entity_id, true, first_apply);
        }
        else {
            terminal_pro_debug(" no such tmnl dblist node!");
        }
    }
    else {
        gdisc_flags.currect_first_index = gdisc_flags.apply_num;
    }
}

#ifdef MIC_PRIOR_MANEGER_ENABLE
#ifdef FIRST_SPEAK_QUEUE_ENABLE
void terminal_firstSpkingPro(uint32_t sysTick) { /* 1ms */
    static uint32_t l_lastTick;
    uint32_t qAddr;
    TSpkQueueElem *qElem;
    tmnl_pdblist spk;
    uint16_t tAddr;
    uint8_t failureTimes;
    uint64_t entityId;
    int i, j;
    bool hasHighTask;
    /* terminal speaking queues */
    static TComQueue * const l_queues[FS_SPEAK_QUEUE_NUM] ={
        &l_chairmanFSpkQueue,
        &l_vipFSpkQueue,
        &l_FSpkQueue
    };

    if ((sysTick - l_lastTick) < 100U) { /* 100ms */
        return;
    }

    /* update last tick */
    l_lastTick = sysTick;
    
    /* lock queue */
    INTERRUPT_LOCK(l_FSpkLocker);

    for (i = 0; i < FS_SPEAK_QUEUE_NUM; i++) {
        /* check whether has higher tast */
        hasHighTask = (bool)0;
        for (j = 0; j < i; j++) {
            if (!QueueCom_isEmpty(l_queues[j])) {
                hasHighTask = (bool)1;
                break;
            }
        }

        /* if has higher task, process later */
        if (hasHighTask) {
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_ERROR,
                "[ Current Aplly queue(%d)  has higher"
                "task queue(%d):Breaking...... ]", i, j);

            break;
        }

        /* initial to zero, must be*/
        qAddr = 0U;
        if (QueueCom_popFiFo(l_queues[i], &qAddr)) {
            /* get information */
            qElem = (TSpkQueueElem *)qAddr;
            
            spk = qElem->spkNode;            
            if (spk == (tmnl_pdblist)0) {
                QueueCom_itemFree(qElem);
                
                continue; /* continue 'for' */
            }

            failureTimes = qElem->failureTimes;
            entityId = qElem->spkNode->tmnl_dev.entity_id;
            tAddr = spk->tmnl_dev.address.addr;

            /* check permission */
            if (!((qElem->permissions)
                    & ((uint32_t)get_sys_state())))
            {
                /* release event */
                QueueCom_itemFree(qElem);
                
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_ERROR,
                    "[Terminal (0x04x)Request Connections Task "
                    "in the First Aplly in queue  "
                    "Cancel by User]", tAddr);

                continue; /* continue 'for' */
            }
            
            /* has timeout and no diconnect task\
                process this task */
            if (trans_model_unit_connect_timeout(entityId)
                && (!Terminal_hasTaskInQueue(DISCONNECTOR)))
            {
                /* request connections */
                if (Terminal_requestConnect(spk,
                        (TEReqQePrior)i,
                        MAX_FAILURE_TIMES,
                        qElem->permissions))
                {
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_ERROR,
                        "[ First Aplly(0x%04x) in queue Request "
                        "connections success,waiting connection.... ]",
                        tAddr);
                }
                else {
                    terminal_mic_state_set(MIC_COLSE_STATUS,
                        spk->tmnl_dev.address.addr,
                        spk->tmnl_dev.entity_id, true, spk);
                    
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_ERROR,
                        "[First Aplly(0x%04x) in queue Request "
                        "connections Failed ]", tAddr);
                }

                /* release event */
                QueueCom_itemFree(qElem);
            }
            else {
                /* post to queue again */
                QueueCom_postLiFo(l_queues[i], (void *)qElem);
            }
        }  
    }    

    /* unlock queue */
    INTERRUPT_UNLOCK(l_FSpkLocker);
}
#else
void terminal_firstSpkingPro(uint32_t sysTick) {
    (void)sysTick;
}
#endif /* FIRST_SPEAK_QUEUE_ENABLE */
#else
#ifdef FIRST_SPEAK_QUEUE_ENABLE
void terminal_firstSpkingPro(uint32_t sysTick) { /* 1ms */
    static uint32_t l_lastTick;
    uint32_t qAddr;
    
    if ((sysTick - l_lastTick) > 300U) {
        INTERRUPT_LOCK(l_FSpkLocker);
        
        if (QueueCom_popFiFo(&l_FSpkQueue, &qAddr)) {
            TSpkQueueElem *qElem;
            tmnl_pdblist spk;
            int ret;
            uint64_t id;
            uint16_t tAddr;
            uint8_t failureTimes;
            
            qElem = (TSpkQueueElem *)qAddr;
            if (qElem == (TSpkQueueElem *)0) {
                free(qElem);
                INTERRUPT_UNLOCK(l_FSpkLocker);
                return;
            }

            failureTimes = qElem->failureTimes;
            spk = qElem->spkNode;
            if (spk == (tmnl_pdblist)0) {
                free(qElem);
                INTERRUPT_UNLOCK(l_FSpkLocker);
                return;
            }
            
            id = spk->tmnl_dev.entity_id;
            tAddr = spk->tmnl_dev.address.addr;
            ret = trans_model_unit_connect(id, spk);
            if (0 == ret) {
                gdisc_flags.speak_limit_num++;
                terminal_speak_track(spk->tmnl_dev.address.addr, true);
                
                terminal_apply_list_first_speak();

                /* release space */
                free(qElem);
                qElem = (TSpkQueueElem *)0;
                
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_DEBUG,
                    "[ First Apply terminal(addr = 0x%x)"
                    " speak Successfully ]", tAddr);
            }
            else { /* can't connect, not timeout of speaker or Error */
                qElem->failureTimes--;
                if (failureTimes == 0) {
                    /* release space */
                    free(qElem);
                    qElem = (TSpkQueueElem *)0;
                    INTERRUPT_UNLOCK(l_FSpkLocker);
                    return;
                }
                
                if (QueueCom_postLiFo(&l_FSpkQueue, (void *)qElem)) {
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_DEBUG,
                        "[ First Apply terminal(addr = 0x%x)"
                        " speak processing: Post to Queue Again Success. ]",
                        tAddr);
                }
                else {
                    gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                        LOGGING_LEVEL_DEBUG,
                        "[ First Apply terminal(addr = 0x%x)"
                        " speak Failed: Post to Queue Again Failed. ]", tAddr);
                }
            }
        }

        l_lastTick = sysTick;
        INTERRUPT_UNLOCK(l_FSpkLocker);
    }
}
#else
void terminal_firstSpkingPro(uint32_t sysTick) {
    (void)sysTick;
}
#endif /* FIRST_SPEAK_QUEUE_ENABLE */

#endif /* MIC_PRIOR_MANEGER_ENABLE */

#ifdef FIRST_SPEAK_QUEUE_ENABLE
/*$ Terminal_firstSpkRequestConnect().......................................*/
bool Terminal_firstSpkRequestConnect(tmnl_pdblist const spk,
    TComQueue * const pRestQueue, int * const locker,
    int failureTimes, uint32_t permissions)
{
/*\ connections request by terminal
    \ */
    bool bRet;
    TSpkQueueElem *qElem;
    
    if ((spk == (tmnl_dblist const * const)0)
          || (pRestQueue == (TComQueue * const)0)
          || (locker == (int *)0))
    {
        return (bool)0;
    }
    
    INTERRUPT_LOCK(*locker);
    
    qElem = (TSpkQueueElem *)malloc(sizeof(TSpkQueueElem));
    if (qElem != (TSpkQueueElem *)0) {
        qElem->spkNode = spk;
        qElem->failureTimes = failureTimes;
        qElem->permissions = permissions;
        if (QueueCom_postFiFo(pRestQueue, (void *)qElem)) {
            bRet = (bool)1;
        }
    }

    INTERRUPT_UNLOCK(*locker);

    return (bool)bRet;
}

/*$ Terminal_changeFirstSpkTaskPer()........................................*/
bool Terminal_changeFirstSpkTaskPer(uint16_t addr,
    TComQueue * const pRestQueue, int * const locker, uint32_t permissions)
{
/*\ connections change permission request by terminal
    \ */
    bool bRet;
    TSpkQueueElem *qElem;
    uint32_t pos, qAddr;
    
    if ((pRestQueue == (TComQueue * const)0)
          || (locker == (int *)0))
    {
        return (bool)0;
    }
    
    INTERRUPT_LOCK(*locker);

    bRet = (bool)0;
    queue_for_each(pRestQueue, pos, qAddr) {
        qElem = (TSpkQueueElem *)qAddr;
        
        if (qElem->spkNode->tmnl_dev.address.addr == addr) {
            qElem->permissions = permissions;
            bRet = (bool)1;
            break;
        }
    }

    INTERRUPT_UNLOCK(*locker);

    return (bool)bRet;
}

#endif /* FIRST_SPEAK_QUEUE_ENABLE */

// 已测试(2016-3-16)
bool terminal_limit_disccuss_mode_pro( bool key_down, uint8_t limit_time,tmnl_pdblist speak_node, uint8_t recv_msg )
{
	bool ret = false;
	
	assert( speak_node );
	if( speak_node == NULL )
	{
		return ret;
	}
		
	uint16_t addr = speak_node->tmnl_dev.address.addr;
	uint16_t current_addr = 0;
	uint8_t cc_state = 0;
	tmnl_pdblist first_apply = NULL; // 首位申请发言
	int  dis_ret = -1;

	terminal_pro_debug("speak_limit_num = %d limit_num = %d apply_num = %d apply_limit = %d", gdisc_flags.speak_limit_num,
				gdisc_flags.limit_num, gdisc_flags.apply_num, gdisc_flags.apply_limit);
	if( key_down ) // 打开麦克风
	{
		if (trans_model_unit_is_connected(speak_node->tmnl_dev.entity_id))
		{
			terminal_key_action_host_special_num1_reply(recv_msg, MIC_OPEN_STATUS, speak_node);
		}
		else
		{		        
			uint16_t speak_num = terminal_speak_num_count();
			if (gdisc_flags.speak_limit_num < gdisc_flags.limit_num \
				&& speak_num < gdisc_flags.limit_num) // 打开麦克风
			{
				dis_ret = trans_model_unit_connect(speak_node->tmnl_dev.entity_id, speak_node);
                                if (dis_ret == 0)
				{
                                        terminal_key_action_host_special_num1_reply(recv_msg, MIC_COLSE_STATUS, speak_node);
					gdisc_flags.speak_limit_num++;
					terminal_speak_track(speak_node->tmnl_dev.address.addr, true );
					ret = true;
				}
				else if (gdisc_flags.apply_num < gdisc_flags.apply_limit && (dis_ret != -2)) // 申请发言,且操作超时
				{//has timeout for operation transmit ouput channel
					uint8_t state = MIC_OTHER_APPLY_STATUS;
					if (0 == gdisc_flags.apply_num)
					{
						gdisc_flags.currect_first_index = 0;
						state = MIC_FIRST_APPLY_STATUS;
					}
					gdisc_flags.apply_addr_list[gdisc_flags.apply_num] = speak_node->tmnl_dev.address.addr;
					gdisc_flags.apply_num++;

					terminal_pro_debug("apply_num = %d", gdisc_flags.apply_num);
					terminal_key_action_host_special_num1_reply( recv_msg, state, speak_node );
					terminal_mic_state_set(state, speak_node->tmnl_dev.address.addr, speak_node->tmnl_dev.entity_id, true, speak_node);
					terminal_main_state_send( 0, NULL, 0 );
					ret = true;
				}
                                else
                                {
                                        terminal_key_action_host_special_num1_reply(recv_msg, MIC_COLSE_STATUS, speak_node);
                                }
			}
			else if (gdisc_flags.apply_num < gdisc_flags.apply_limit) // 申请发言
			{
				uint8_t state = MIC_OTHER_APPLY_STATUS;
				if (0 == gdisc_flags.apply_num)
				{
					gdisc_flags.currect_first_index = 0;
					state = MIC_FIRST_APPLY_STATUS;
				}
				gdisc_flags.apply_addr_list[gdisc_flags.apply_num] = speak_node->tmnl_dev.address.addr;
				gdisc_flags.apply_num++;

				terminal_pro_debug("apply_num = %d", gdisc_flags.apply_num);
				terminal_key_action_host_special_num1_reply( recv_msg, state, speak_node );
				terminal_mic_state_set(state, speak_node->tmnl_dev.address.addr, speak_node->tmnl_dev.entity_id, true, speak_node);
				terminal_main_state_send( 0, NULL, 0 );
				ret = true;
			}
			else
				terminal_key_action_host_special_num1_reply(recv_msg, MIC_COLSE_STATUS, speak_node);
		}
	}
	else
        {
                /* get current state must be here,
                    because trans_model_unit_disconnect() is muti pthread funtion,
                    mic_state change probably after being invoked */
                cc_state = speak_node->tmnl_dev.tmnl_status.mic_state;
                int ret = trans_model_unit_disconnect(speak_node->tmnl_dev.entity_id, speak_node);
                if (0 == ret)
    		{
		        terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, speak_node );
                        terminal_speak_track(speak_node->tmnl_dev.address.addr, false );
    		}
                else if ((-2 == ret)) {
		        terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, speak_node );
                }                   
                else {
                    terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );
                }

                if (ret != -2) {
                        current_addr = gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index];
        		if( cc_state == MIC_FIRST_APPLY_STATUS || cc_state == MIC_OTHER_APPLY_STATUS )
        		{
        			addr_queue_delect_by_value( gdisc_flags.apply_addr_list, &gdisc_flags.apply_num, addr );
                                speak_node->tmnl_dev.tmnl_status.mic_state = MIC_COLSE_STATUS;
        			//terminal_mic_state_set(MIC_COLSE_STATUS, speak_node->tmnl_dev.address.addr, speak_node->tmnl_dev.entity_id, true, speak_node);
        			if( gdisc_flags.apply_num > 0 && current_addr == addr )// 置下一个申请为首位申请状态
        			{
        				gdisc_flags.currect_first_index %= gdisc_flags.apply_num;
        				first_apply = found_terminal_dblist_node_by_addr( gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index]);
        				if( first_apply != NULL )
        				{
        				        #if 0
        					terminal_mic_state_set( MIC_FIRST_APPLY_STATUS, first_apply->tmnl_dev.address.addr, first_apply->tmnl_dev.entity_id, true, first_apply );
                                                #else
                                                DEBUG_INFO("set first apply(0x%04x)", first_apply->tmnl_dev.address.addr);
                                                terminal_over_time_firstapply_node_set(first_apply);
                                                #endif
                                        }
        			}

        			terminal_main_state_send( 0, NULL, 0 );
        			ret = true;
        		}
        		else if( cc_state == MIC_OPEN_STATUS )
        		{
        			if( gdisc_flags.speak_limit_num > 0 )
        			{
        				gdisc_flags.speak_limit_num--;
        			}

        			if(gdisc_flags.speak_limit_num < gdisc_flags.limit_num && gdisc_flags.apply_num > 0 )// 结束发言,并开始下一个申请终端的发言
        			{
        				if( addr_queue_delete_by_index( gdisc_flags.apply_addr_list, &gdisc_flags.apply_num, gdisc_flags.currect_first_index) )// 开启下一个申请话筒
        				{
        					tmnl_pdblist first_speak = found_terminal_dblist_node_by_addr( current_addr );
        					if( first_speak != NULL )
        					{
#ifdef FIRST_SPEAK_QUEUE_ENABLE
                                                    if (Terminal_firstSpkRequestConnect(first_speak,
                                                        &l_FSpkQueue, &l_FSpkLocker,
                                                        MAX_CONNECT_FAILURE_TIMES,
                                                        COMMON_SPK_PERMISSION))
                                                    {
                                                            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                                                                LOGGING_LEVEL_DEBUG,
                                                                "[ Limit Mode Speak Post First apply]"
                                                                " speaking node(address = 0x%x) to Queue Success ]",
                                                                first_speak->tmnl_dev.address.addr);
                                                    }
                                                    else {
                                                         gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                                                                LOGGING_LEVEL_DEBUG,
                                                                "[ Limit Mode Speak Post First apply]"
                                                                " speaking node(address = 0x%x)to Queue Failed ]",
                                                                first_speak->tmnl_dev.address.addr);
                                                    }
#else
        						if (0 == trans_model_unit_connect( first_speak->tmnl_dev.entity_id, first_speak ))
        						{// connect success
        							gdisc_flags.speak_limit_num++;
                                                                terminal_speak_track(first_speak->tmnl_dev.address.addr, true);
        							terminal_apply_list_first_speak();
        						}
        						else
        						{
        							/*
        							  *当前的mic断开成功后会留出一个通道
        							  */
        							terminal_over_time_speak_node_set(first_speak);
        						}
#endif                                
        					}
        					else
        					{
        						terminal_pro_debug( " no such tmnl dblist node!");
        					}
        				}
        				else
        				{
        					gdisc_flags.currect_first_index = 0;
        				}
        			}

        			terminal_main_state_send( 0, NULL, 0 );
        			ret = true;
        		}
                }
	}
		
	return ret;
}

// 已测试(2016-3-16)
bool terminal_fifo_disccuss_mode_pro( bool key_down, uint8_t limit_time,tmnl_pdblist speak_node, uint8_t recv_msg )
{
	bool ret = false;
	uint16_t addr = speak_node->tmnl_dev.address.addr;
	int dis_ret = -1;

	assert( speak_node );
	if( speak_node == NULL )
	{
		terminal_pro_debug( " NULL speak node!" );
		return false;
	}

	uint8_t speak_limit_num = gdisc_flags.speak_limit_num;
	uint16_t speak_num = terminal_speak_num_count();
	terminal_pro_debug( " speak NUM = %d limit speak NUM = %d", speak_limit_num, gdisc_flags.limit_num );
	if( key_down ) // 打开话筒
	{
		if( addr_queue_find_by_value( gdisc_flags.speak_addr_list, speak_limit_num, addr, NULL))
		{
			if (trans_model_unit_is_connected(speak_node->tmnl_dev.entity_id))
			{
				terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, speak_node );
				ret = true;
			}
			else
			{
				dis_ret = trans_model_unit_connect( speak_node->tmnl_dev.entity_id, speak_node );
				ret = (dis_ret == 0)? true:false;
				if (ret)
                                {
        				terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );      
                			terminal_speak_track(speak_node->tmnl_dev.address.addr, true );
                               }
                               else
                               {
        				terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );                                   
                               }
                        }
		}
		else if( speak_num < gdisc_flags.limit_num && speak_limit_num < gdisc_flags.limit_num)
		{
			speak_node->tmnl_dev.tmnl_status.mic_state = MIC_COLSE_STATUS;
			dis_ret = trans_model_unit_connect( speak_node->tmnl_dev.entity_id, speak_node );
                        if (dis_ret == 0)
			{
        			terminal_key_action_host_special_num1_reply(recv_msg, MIC_COLSE_STATUS, speak_node);
				terminal_speak_track(speak_node->tmnl_dev.address.addr, true );
				gdisc_flags.speak_addr_list[speak_limit_num] = addr;
				gdisc_flags.speak_limit_num++;
                                gdisc_flags.speak_limit_num %= MAX_LIMIT_SPK_NUM;
				ret = true;
			}
			else if (dis_ret != -2)//  has no input channel to connect, delect address from fifo speaking list and opt not timeout
			{//has timeout for operation transmit ouput channel
				terminal_pro_debug( "=============connect Failed,Will disconnect first speak================" );
        		        terminal_key_action_host_special_num1_reply(recv_msg, MIC_COLSE_STATUS, speak_node);
                                if( gdisc_flags.speak_addr_list[0] != 0xffff ) // 先进先出
				{
					tmnl_pdblist first_speak = found_terminal_dblist_node_by_addr(gdisc_flags.speak_addr_list[0]);
					if( first_speak != NULL )
					{
						dis_ret = trans_model_unit_disconnect (first_speak->tmnl_dev.entity_id, first_speak);
						if (0 == dis_ret)
						{
							terminal_speak_track(first_speak->tmnl_dev.address.addr, false );
							addr_queue_delete_by_index( gdisc_flags.speak_addr_list, &gdisc_flags.speak_limit_num, 0 );// 首位发言删除
							terminal_over_time_speak_node_set(speak_node);
						}
					}
					else
					{
						terminal_pro_debug( "fifo not found tmnl list node!");
					}			
				}
                                else
                                {
                                        terminal_key_action_host_special_num1_reply(recv_msg, MIC_COLSE_STATUS, speak_node);
                                }
			}
                        else
                        {
                                terminal_key_action_host_special_num1_reply(recv_msg, MIC_COLSE_STATUS, speak_node);
                        }
		}
		else // 发言人数大于或等于限制人数
		{
			if (!trans_model_unit_is_connected(speak_node->tmnl_dev.entity_id))
			{
				if( gdisc_flags.speak_addr_list[0] != 0xffff ) // 先进先出
				{
					tmnl_pdblist first_speak = found_terminal_dblist_node_by_addr( gdisc_flags.speak_addr_list[0] );
					if( first_speak != NULL )
					{
						if (trans_model_unit_is_connected(first_speak->tmnl_dev.entity_id))
						{
							dis_ret = trans_model_unit_disconnect( first_speak->tmnl_dev.entity_id, first_speak );
							if (0 == dis_ret)
							{
                                                                terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );
								terminal_speak_track(first_speak->tmnl_dev.address.addr, false );
								addr_queue_delete_by_index( gdisc_flags.speak_addr_list, &gdisc_flags.speak_limit_num, 0 );// 首位发言删除
								terminal_over_time_speak_node_set(speak_node);
							}
                                                        else
                                                                terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );
						}
                                                else
                                                        terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );
					}
					else
					{
					        terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );
						terminal_pro_debug( "fifo not found tmnl list node!");
					}			
				}
                                else
                                        terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );

			}
			else
			{
				if (speak_node->tmnl_dev.tmnl_status.mic_state != MIC_OPEN_STATUS)
					speak_node->tmnl_dev.tmnl_status.mic_state = MIC_OPEN_STATUS;
				
				terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, speak_node );
			}
		}
	}
	else
	{
		if (trans_model_unit_is_connected(speak_node->tmnl_dev.entity_id))
		{
			if (0 == trans_model_unit_disconnect( speak_node->tmnl_dev.entity_id, speak_node ))
			{
                        	terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, speak_node );
                                addr_queue_delect_by_value(gdisc_flags.speak_addr_list, &gdisc_flags.speak_limit_num, speak_node->tmnl_dev.address.addr);
				terminal_speak_track(speak_node->tmnl_dev.address.addr, false );
				ret = true;
			}
			else
			{
				terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, speak_node );
			}
		}
		else
		{
			terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );
		}
	}

	return ret;
}

bool terminal_apply_disccuss_mode_pro( bool key_down, uint8_t limit_time,tmnl_pdblist speak_node, uint8_t recv_msg )
{
	assert( speak_node );
	if( speak_node == NULL )
	{
		return false;
	}

	bool ret = false;
	uint16_t addr = speak_node->tmnl_dev.address.addr;
	uint16_t current_addr = 0;
	tmnl_pdblist first_apply = NULL;

	if( key_down ) /* 申请发言,加地址入申请列表*/
	{
		uint8_t state = MIC_OTHER_APPLY_STATUS;
		if(  gdisc_flags.apply_num < gdisc_flags.apply_limit )
		{
			if( 0 == gdisc_flags.apply_num )
			{
				gdisc_flags.currect_first_index = 0;
				state = MIC_FIRST_APPLY_STATUS;
			}
			
			gdisc_flags.apply_addr_list[gdisc_flags.apply_num] = speak_node->tmnl_dev.address.addr;
			gdisc_flags.apply_num++;

			terminal_key_action_host_special_num1_reply( recv_msg, state, speak_node );
			terminal_mic_state_set_send_terminal( false, state, speak_node->tmnl_dev.address.addr, speak_node->tmnl_dev.entity_id, true, speak_node );
			terminal_main_state_send( 0, NULL, 0 );
			ret = true;
		}
		else
		{
			state = MIC_COLSE_STATUS;
			terminal_key_action_host_special_num1_reply( recv_msg, state, speak_node );
		}
	}
	else /* 取消申请发言*/
	{
		current_addr = gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index];
		if(addr_queue_delect_by_value( gdisc_flags.apply_addr_list, &gdisc_flags.apply_num, addr ))
		{/* terminal apply */
			terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );
			terminal_mic_state_set_send_terminal( false, MIC_COLSE_STATUS, speak_node->tmnl_dev.address.addr, speak_node->tmnl_dev.entity_id, true, speak_node );// 上报mic状态
			if( gdisc_flags.apply_num > 0 && current_addr == addr )// 置下一个申请为首位申请状态
			{
				gdisc_flags.currect_first_index %= gdisc_flags.apply_num;
				first_apply = found_terminal_dblist_node_by_addr( gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index] );
				if( first_apply != NULL )
				{
					terminal_mic_state_set( MIC_FIRST_APPLY_STATUS, first_apply->tmnl_dev.address.addr, first_apply->tmnl_dev.entity_id, true, first_apply );
				}
			}
			
			terminal_main_state_send( 0, NULL, 0 );
		}	
		else
		{/* terminal speaking */
			if (trans_model_unit_is_connected(speak_node->tmnl_dev.entity_id))
			{
				if ( 0 == trans_model_unit_disconnect( speak_node->tmnl_dev.entity_id, speak_node ))
                                {
                                        terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, speak_node );
                                        terminal_speak_track(speak_node->tmnl_dev.address.addr, false );
                                }
                                else
                                        terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );

			}
                        else
                                terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );
		}
		
		ret = true;
	}

	return ret;
}

void terminal_key_preset( uint8_t tmnl_type, uint16_t tmnl_addr, uint8_t tmnl_state, uint8_t key_num, uint8_t key_value )
{
	uint16_t addr;
	if( key_value )
	{
		addr = tmnl_addr;
		find_func_command_link( SYSTEM_USE, SYS_PRESET_ADDR, 0, (uint8_t*)&addr, sizeof(uint16_t) );
	}
}

int terminal_speak_track( uint16_t addr, bool track_en )/* 摄像跟踪接口*/
{
	uint16_t temp;
	uint16_t i;
	uint16_t index;
	uint8_t cmr_track;

	cmr_track = gset_sys.camara_track;
 	if( !cmr_track )
	{
		return -1;
	}
	
	for( i = 0; i <gspeaker_track.spk_num; i++)
	{
		if( addr == gspeaker_track.spk_addrlist[i])
		{
			break;
		}
	}
	
	index = i;
	if( index < gspeaker_track.spk_num )
	{
		if( gspeaker_track.spk_num > 1)
		{
			for( i = index; i < (gspeaker_track.spk_num-1); i++)
			{
				gspeaker_track.spk_addrlist[i] = gspeaker_track.spk_addrlist[i+1];
			}
			
			gspeaker_track.spk_num--;
		}
		else if( gspeaker_track.spk_num == 1 )
		{
			gspeaker_track.spk_addrlist[0] = 0xFFFF;
			gspeaker_track.spk_num = 0;
		}
	}
	
	if( track_en )
	{
		if( gspeaker_track.spk_num < MAX_SPK_NUM )
		{
			gspeaker_track.spk_addrlist[gspeaker_track.spk_num] = addr;
			gspeaker_track.spk_num++;
			temp = addr;
			find_func_command_link( SYSTEM_USE, SYS_GET_PRESET, 0, (uint8_t*)&temp,sizeof(uint16_t) );
		}
	}
	else
	{
		if( 0 == gspeaker_track.spk_num )
		{
			temp = FULL_VIEW_ADDR;
		}
		else
		{
			temp = gspeaker_track.spk_addrlist[gspeaker_track.spk_num-1];
		}
		
		find_func_command_link( SYSTEM_USE, SYS_GET_PRESET, 0, (uint8_t*)&temp,sizeof(uint16_t) );
	}

	return 0;
}

/*************************************************************
*==开始投票处理-->处理未签到的终端的投票
*/
void terminal_vote_proccess( void )
{
	if( gvote_flag == VOTE_SET )
	{
		uint16_t index = gvote_index;
		uint16_t addr = 0xffff;
		tmnl_pdblist tmp = NULL;
		bool waiting_query = false;
		
		do
		{
			addr = tmnl_addr_list[index].addr;
			if( addr != 0xffff )
			{
				tmp = found_terminal_dblist_node_by_addr( addr );
				if( (tmp != NULL) && (tmp->tmnl_dev.address.addr != 0xffff)\
					&& ( tmp->tmnl_dev.tmnl_status.is_rgst) &&\
					( tmp->tmnl_dev.tmnl_status.vote_state & TVOTE_SET_FLAG ) &&\
					( tmp->tmnl_dev.tmnl_status.vote_state & TVOTE_EN ))
				{/* 等待投票条件TVOTE_EN成立(即签到成功) */
					waiting_query = true;
					break;
				}
			}

			index++;
			index %= SYSTEM_TMNL_MAX_NUM;
		}while( index != gvote_index );

		if( waiting_query )
		{
			if( tmp != NULL )
			{
				tmp->tmnl_dev.tmnl_status.vote_state &= (~TVOTE_SET_FLAG);
				tmp->tmnl_dev.tmnl_status.vote_state |= (TWAIT_VOTE_FLAG);// 设置成可查询状态
				gvote_index++;
				gvote_index %= SYSTEM_TMNL_MAX_NUM;
			}
		}
		else
		{/* 查看系统是否投票完成*/
			int i = 0;
			for( i = 0; i < SYSTEM_TMNL_MAX_NUM; i++ )
			{
				addr = tmnl_addr_list[i].addr;
				if( addr != 0xffff )
				{
					tmp = found_terminal_dblist_node_by_addr( addr );
					if( (tmp != NULL) && (tmp->tmnl_dev.address.addr != 0xffff)\
						&& (tmp->tmnl_dev.tmnl_status.is_rgst) &&\
						( tmp->tmnl_dev.tmnl_status.vote_state & TVOTE_SET_FLAG))
					{
						break;
					}
				}
			}

			if( i >= SYSTEM_TMNL_MAX_NUM )
			{
				gvote_flag = VOTE_SET_OVER;
				gvote_index = 0;
			}
		}
	}
}

/*************************************************************
*==结束投票处理
*/

/*************************************************************
*==开始查询处理
*/
/*************************************************************
*Date:2016/1/27
*Name:terminal_query_vote_ask
*功能:保存终端签到、表决结果
*Param:
*	address :terminal application address
*	vote_state:终端签到表决按键结果
*Return:None
**************************************************************/
void terminal_query_vote_ask( uint16_t address, uint8_t vote_state )
{
        tmnl_pdblist vote_node;
        if (!gquery_svote_pro.endQr) {
            gquery_svote_pro.endQr = true;
        }
	vote_node = found_terminal_dblist_node_by_addr( address );
	if( NULL == vote_node )
	{
		terminal_pro_debug( "no such address 0x%04x node ", address );
		return;
	}

	uint8_t sys_state = get_sys_state();
	if( (SIGN_STATE == sys_state ) && (vote_state & 0x80) )/* sign complet? */
	{                                                                                           
		if( gtmnl_signstate == SIGN_IN_ON_TIME )            /* 设置签到标志*/
		{
			vote_node->tmnl_dev.tmnl_status.sign_state = TMNL_SIGN_ON_TIME;
		}
		else if( gtmnl_signstate == SIGN_IN_BE_LATE && (vote_node->tmnl_dev.tmnl_status.sign_state == TMNL_NO_SIGN_IN) )
		{
			vote_node->tmnl_dev.tmnl_status.sign_state = SIGN_IN_BE_LATE;
		}
		
		upper_cmpt_report_sign_in_state( vote_node->tmnl_dev.tmnl_status.sign_state, vote_node->tmnl_dev.address.addr );
	}
	else if ( ((VOTE_STATE == sys_state) || (GRADE_STATE == sys_state) \
		||(ELECT_STATE == sys_state)) && ((vote_state & TVOTE_KEY_MARK )))
	{
		if ( gfirst_key_flag )
		{
			int i = 0, vote_num = 0;
			for ( i = 0; i < 5; i++ )
			{
				if( vote_node->tmnl_dev.tmnl_status.vote_state& (1<<i) )
				{
					vote_num++;
				}
			}
#if 1
			uint8_t key_num = 0;
			terminal_vote_mode_max_key_num( &key_num, gvote_mode );
			terminal_pro_debug( "max key num = %d-------vote num = %d", key_num, vote_num );
			if ( vote_num >= key_num )
			{/* 投票完成，相应的终端停止查询投票结果*/
				vote_node->tmnl_dev.tmnl_status.vote_state &= (~TWAIT_VOTE_FLAG);
			}
#else
			if ( vote_num >= gvote_mode )
			{// 投票完成，相应的终端停止查询投票结果
				vote_node->tmnl_dev.tmnl_status.vote_state &= (~TWAIT_VOTE_FLAG);
			}
#endif
		}
		
		vote_node->tmnl_dev.tmnl_status.vote_state &= (~TVOTE_KEY_MARK);
		vote_node->tmnl_dev.tmnl_status.vote_state |= (vote_state & TVOTE_KEY_MARK);
                if (VOTE_STATE == sys_state) {
                    if (!vote_node->tmnl_dev.tmnl_status.is_vote) {
                        vote_node->tmnl_dev.tmnl_status.is_vote = true;
                    }
                }
                else if (GRADE_STATE == sys_state) {
                    if (!vote_node->tmnl_dev.tmnl_status.is_grade) {
                        vote_node->tmnl_dev.tmnl_status.is_grade = true;
                    }   
                }
                else if (ELECT_STATE == sys_state) {
                    if (!vote_node->tmnl_dev.tmnl_status.is_select) {
                        vote_node->tmnl_dev.tmnl_status.is_select = true;
                    }   
                }
                else {
                    /* nerver came this case */
                }

		upper_cmpt_vote_situation_report( vote_node->tmnl_dev.tmnl_status.vote_state, vote_node->tmnl_dev.address.addr );
	}
}

void terminal_vote_mode_max_key_num( uint8_t *key_num, tevote_type vote_mode  )
{
	assert( key_num );
	if( key_num == NULL )
		return;
	
	switch( vote_mode )
	{
		case VOTE_MODE:
		case GRADE_MODE:
		case SLCT_2_1:
		case SLCT_3_1:
		case SLCT_4_1:
		case SLCT_5_1:
		{
			*key_num = 1;
			break;
		}
		
		case SLCT_2_2:
		case SLCT_3_2:
		case SLCT_4_2:
		case SLCT_5_2:
		{
			*key_num = 2;
			break;
		}
		case SLCT_3_3:
		case SLCT_4_3:
		case SLCT_5_3:
		{
			*key_num = 3;
			break;
		}
		case SLCT_4_4:
		case SLCT_5_4:
		{
			*key_num = 4;
			break;
		}
		case SLCT_5_5:
		{
			*key_num = 5;
			break;
		}
		default:
		{
			*key_num = 0;
			break;
		}
	}
}

void terminal_over_time_speak_node_set(tmnl_pdblist speak_node) {
    if ((!gdisc_flags.over_speak.running)
        && (NULL != speak_node))
    {
        gdisc_flags.over_speak.speak_node = speak_node;
        gdisc_flags.over_speak.running = true;
        over_time_set(DISCUSS_MODE_SPEAK_AFTER, 500);
    }
}

void terminal_over_time_firstapply_node_set( tmnl_pdblist speak_node )
{
    if ((!gdisc_flags.overApply.running) && (NULL != speak_node)) {
        gdisc_flags.overApply.speak_node = speak_node;
        gdisc_flags.overApply.running = true;
        over_time_set(DISCUSS_MODE_APPLY_AFTER, 500);
    }
}

void terminal_over_time_speak_pro(void) {
    uint8_t type_;
    tmnl_pdblist speak_node;
    ttmnl_discuss_mode mode_;
    int ret = -1;
    
    if ((gdisc_flags.over_speak.running)
        && (over_time_listen(DISCUSS_MODE_SPEAK_AFTER)))
    {
        speak_node = gdisc_flags.over_speak.speak_node;
        if (speak_node == (tmnl_pdblist)0) {
            return;
        }
        
        mode_ = gdisc_flags.edis_mode;
        type_ = speak_node->tmnl_dev.address.tmn_type;
        if ((mode_ == PPT_MODE)
              || (type_ == TMNL_TYPE_VIP)
              || (type_ == TMNL_TYPE_CHM_COMMON)
              || (type_ == TMNL_TYPE_CHM_EXCUTE))
        {
            if ((interposeLate)
                  && (get_sys_state() != INTERPOSE_STATE)
                  && ((type_ == TMNL_TYPE_CHM_COMMON)
                          || (type_ == TMNL_TYPE_CHM_EXCUTE)))
            {
                interposeLate = false;
            }
            else {
                ret = trans_model_unit_connect(speak_node->tmnl_dev.entity_id,
                                        speak_node);
                if (ret == 0) {
                    terminal_speak_track(speak_node->tmnl_dev.address.addr, true);
                }
            }
        }
        else if (mode_ == FIFO_MODE) {
            ret = trans_model_unit_connect(speak_node->tmnl_dev.entity_id,
                            speak_node);
            if (ret == 0) {
                if (type_ == TMNL_TYPE_COMMON_RPRST) {
                    gdisc_flags.speak_addr_list[gdisc_flags.speak_limit_num] =
                        speak_node->tmnl_dev.address.addr;
                    gdisc_flags.speak_limit_num++;
                }

                terminal_speak_track(speak_node->tmnl_dev.address.addr, true);
            }
        }
        else if (mode_ == LIMIT_MODE) {
            ret = trans_model_unit_connect(speak_node->tmnl_dev.entity_id,
                                    speak_node);
            if (ret == 0) {/* connect success */
                if (type_ == TMNL_TYPE_COMMON_RPRST) {
                    gdisc_flags.speak_limit_num++;
                }

                terminal_speak_track(speak_node->tmnl_dev.address.addr, true);
                terminal_apply_list_first_speak();
            }
        }

        gdisc_flags.over_speak.running = false;
        gdisc_flags.over_speak.speak_node = NULL;
        over_time_stop(DISCUSS_MODE_SPEAK_AFTER);
    }
}

void terminal_over_time_firstapply_pro(void)
{
	if (gdisc_flags.overApply.running && over_time_listen(DISCUSS_MODE_APPLY_AFTER))
	{
		tmnl_pdblist apply_node = gdisc_flags.overApply.speak_node;
		terminal_mic_state_set(MIC_FIRST_APPLY_STATUS, 
                    apply_node->tmnl_dev.address.addr,
                    apply_node->tmnl_dev.entity_id, true, apply_node);
		gdisc_flags.overApply.running = false;
		gdisc_flags.overApply.speak_node = NULL;
		over_time_stop(DISCUSS_MODE_APPLY_AFTER);
	}
}


/* 主机查询签到投票结果*/
void terminal_query_sign_vote_pro( void )
{
	bool sending = false;
	uint16_t index;
	uint8_t sys_state = get_sys_state();
	tmnl_pdblist tmp_node = NULL;
	uint16_t addr = 0xffff;

	index = gquery_svote_pro.index;
	if(index > (SYSTEM_TMNL_MAX_NUM - 1))
	{
		terminal_pro_debug( "out of system terminal list bank!" );
		return;
	}
	
	if ( (gquery_svote_pro.running)
                && (/*host_timer_timeout(&gquery_svote_pro.query_timer)
                        || (*/gquery_svote_pro.endQr/*)*/))
	{
		host_timer_update( 60, &gquery_svote_pro.query_timer );
		if( sys_state == SIGN_STATE )
		{
			do
			{
				addr = tmnl_addr_list[index].addr;
				if( addr != 0xffff )
				{
					tmp_node = found_terminal_dblist_node_by_addr( addr );
					if( (tmp_node != NULL) && (tmp_node->tmnl_dev.address.addr != 0xffff) &&\
						(tmp_node->tmnl_dev.tmnl_status.is_rgst) &&\
						(tmp_node->tmnl_dev.tmnl_status.sign_state == TMNL_NO_SIGN_IN ) )
					{
						terminal_query_vote_sign_result( tmp_node->tmnl_dev.entity_id, addr );
						sending = true;
                                                gquery_svote_pro.endQr = false;
						break;
					}
				}

				index++;
				index %= SYSTEM_TMNL_MAX_NUM;
			}while( index != gquery_svote_pro.index );

			if( sending )
			{
				gquery_svote_pro.index = (index + 1)%SYSTEM_TMNL_MAX_NUM;
			}
			else
			{
				gquery_svote_pro.running = false;
			}
		}
		else if( (sys_state == VOTE_STATE ) || (sys_state == GRADE_STATE) ||(sys_state == ELECT_STATE))
		{
			do
			{
				addr = tmnl_addr_list[index].addr;
				if( addr != 0xffff )
				{
					tmp_node = found_terminal_dblist_node_by_addr( addr );
					if( (tmp_node != NULL) && (tmp_node->tmnl_dev.address.addr != 0xffff) &&\
						(tmp_node->tmnl_dev.tmnl_status.is_rgst) &&\
						(tmp_node->tmnl_dev.tmnl_status.vote_state & TWAIT_VOTE_FLAG ) )
					{
						terminal_query_vote_sign_result( tmp_node->tmnl_dev.entity_id, addr );
						sending = true;
                                                gquery_svote_pro.endQr = false;
						break;
					}
				}

				index++;
				index %= SYSTEM_TMNL_MAX_NUM;
			}while( index != gquery_svote_pro.index );

			if( sending )
			{
				gquery_svote_pro.index = (index + 1)%SYSTEM_TMNL_MAX_NUM;
			}
			else
			{
				terminal_option_endpoint( BRDCST_1722_ALL, BRDCST_EXE, OPT_TMNL_ALL_VOTE );
				gquery_svote_pro.running = false;
			}
		}
	}
}

void terminal_query_proccess_init( void )
{
	gquery_svote_pro.index = 0;
	gquery_svote_pro.running = false;
        gquery_svote_pro.endQr = true;
	host_timer_stop(&gquery_svote_pro.query_timer );
}

/*************************************************************
*==结束查询处理
*/

/*************************************************************
*==开始签到处理
*/
void terminal_sign_in_pro( void )
{
	uint8_t sign_type;
	int i = 0;

	if( (gtmnl_signstate == SIGN_IN_BE_LATE)  )
	{
		if(over_time_listen( SIGN_IN_LATE_HANDLE ))
		{
			gtmnl_signstate = SIGN_IN_OVER;
			sign_type = gset_sys.sign_type;
			if( sign_type )
			{
				terminal_pro_debug( "over time sign in sign type is card sign in " );
				return;
			}
			
			for( i = 0; i < SYSTEM_TMNL_MAX_NUM; i++)
			{
				uint16_t addr = tmnl_addr_list[i].addr;
				if( addr == 0xffff )
					continue;
				else if( addr != 0xffff )
				{
					tmnl_pdblist tmp = found_terminal_dblist_node_by_addr( addr );
					if( (tmp != NULL) && ( tmp->tmnl_dev.address.addr != 0xffff) &&\
						(tmp->tmnl_dev.tmnl_status.is_rgst) && \
							(tmp->tmnl_dev.tmnl_status.sign_state == TMNL_NO_SIGN_IN ))
					{
						terminal_led_set_save( addr, TLED_KEY5, TLED_OFF );
						fterminal_led_set_send( addr );
					}
				}
			}

			over_time_stop( SIGN_IN_LATE_HANDLE );
		}
	}
}

/*************************************************************
*==结束签到处理
*/

/*===================================================
终端处理流程@}
=====================================================*/

/*==================================================
	start reallot address
====================================================*/
/* 清除终端链表*/
tmnl_pdblist terminal_system_dblist_except_free( void )
{
	tmnl_pdblist p_node = NULL;
	
	p_node = terminal_dblist_except_free( dev_terminal_list_guard );
	if( p_node == dev_terminal_list_guard )
                gcur_tmnl_list_node = dev_terminal_list_guard;

	return p_node;
}

/* 清除除了target_id 终端链表节点*/
void terminal_system_clear_node_info_expect_target_id( void )
{
	tmnl_pdblist p_node = NULL;
	
	for (p_node = dev_terminal_list_guard->next;\
            p_node != dev_terminal_list_guard; \
            p_node = p_node->next)
	{/* clear info expect target_id */
	    p_node->tmnl_dev.address.addr = 0xffff;
            p_node->tmnl_dev.address.tmn_type = 0xffff;
            p_node->tmnl_dev.spk_operate_timp = 0;
            host_timer_stop(&p_node->tmnl_dev.spk_timeout);
            memset(&p_node->tmnl_dev.tmnl_status, 0, sizeof(terminal_state));
	}
}

void terminal_open_addr_file_wt_wb( void )
{
	if( addr_file_fd != NULL )
		Fclose( addr_file_fd );// 先关闭
		
	addr_file_fd = Fopen( ADDRESS_FILE, "wb+");
	if( addr_file_fd == NULL )
	{
		DEBUG_ERR( "terminal_open_addr_file_wt_wb open fd  Err!" );
		assert( NULL != addr_file_fd );
		if( NULL == addr_file_fd )
			return;
	}	
}

/* 摧毁终端链表*/
void terminal_system_dblist_destroy( void )
{
	tmnl_pdblist p_node = destroy_terminal_dblist( dev_terminal_list_guard );
	if( NULL != p_node )
	{
		terminal_pro_debug( "destroy terminal double list not success!" );
	}
}
	
/*===================================================
end reallot address
=====================================================*/

uint16_t terminal_pro_get_address( int get_flags, uint16_t addr_cur )
{
	uint16_t addr = 0xffff;
	static int person = 0;/* 1 链表最后一个；-1链表前一个可用节点*/

	assert( gcur_tmnl_list_node );
	if( gcur_tmnl_list_node != NULL )
	{
		if( get_flags == 1 )
		{
			if( gcur_tmnl_list_node->next != dev_terminal_list_guard )
			{
				if( gcur_tmnl_list_node->next->tmnl_dev.address.addr != 0xffff &&\
					gcur_tmnl_list_node->next->tmnl_dev.tmnl_status.is_rgst )
				{/* gcur_tmnl_list_node 只移到最后一个已注册终端(有效节点) */
					addr = gcur_tmnl_list_node->next->tmnl_dev.address.addr;
					gcur_tmnl_list_node = gcur_tmnl_list_node->next;
				}
				else
					person = 1;
			}
			else 
			{
				person = 1;
			}
		}
		else if( get_flags == -1 )
		{
			if( person && gcur_tmnl_list_node->tmnl_dev.address.addr != 0xffff &&
				 gcur_tmnl_list_node->tmnl_dev.tmnl_status.is_rgst )
			{/* 最后一个可用节点*/
				addr = gcur_tmnl_list_node->tmnl_dev.address.addr;
				person = 0;
			}
			else if( gcur_tmnl_list_node->prior != dev_terminal_list_guard && gcur_tmnl_list_node->prior->tmnl_dev.address.addr != 0xffff )
			{
				if(  gcur_tmnl_list_node->prior->tmnl_dev.tmnl_status.is_rgst )
				{/* gcur_tmnl_list_node 只移到最前一个已注册终端*/
					addr = gcur_tmnl_list_node->prior->tmnl_dev.address.addr;
					gcur_tmnl_list_node = gcur_tmnl_list_node->prior;
				}
			}
			else if(gcur_tmnl_list_node->prior != dev_terminal_list_guard && gcur_tmnl_list_node->prior->tmnl_dev.address.addr == 0xffff )
			{/* 不可用节点0xffff */
				gcur_tmnl_list_node = gcur_tmnl_list_node->prior;
				if( gcur_tmnl_list_node->tmnl_dev.address.addr != 0xffff &&
				 	gcur_tmnl_list_node->tmnl_dev.tmnl_status.is_rgst)
				{/* 最后一个可用节点*/
					addr = gcur_tmnl_list_node->tmnl_dev.address.addr;
				}
			}
			else if( gcur_tmnl_list_node->prior == dev_terminal_list_guard )
			{
				if( gcur_tmnl_list_node->tmnl_dev.address.addr != 0xffff &&\
					gcur_tmnl_list_node->tmnl_dev.tmnl_status.is_rgst )
				{/* gcur_tmnl_list_node 只移到最前一个已注册终端*/
					addr = gcur_tmnl_list_node->tmnl_dev.address.addr;
				}
			}
		}
	}

	return addr;
}

void terminal_pro_init_cur_terminal_node( void )
{
	gcur_tmnl_list_node = dev_terminal_list_guard;
}

