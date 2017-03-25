/*
* @file arcs_extern_port.c
* @brief avdecc reception control system extern port
* @ingroup avdecc reception control system module
* @cond
******************************************************************************
* Build Date on  2016-12-5
* Last updated for version 1.0.0
* Last updated on  2016-03-25
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
#include "jdksavdecc_world.h"
#include <sys/types.h>      /* begin open function */
#include <sys/stat.h>
#include <fcntl.h>          /* end open function */
#include "arcs_extern_port.h"
#include "host_controller_debug.h"

/*$ define arcs port--------------------------------------------------------*/
typedef struct TArcsPort {
    TExternPortVtbl vTable;
    bool avail; /*! avail flags to use port */
}TArcsPort;

/* PERROR */
#define PERROR(ret, str) do {\
    if (ret == -1) {\
        perror(str);\
        exit(-1);\
    }\
}while(0)

#define RDFIFO_NAME "/tmp/AReadFifo" /* to write */
#define WRFIFO_NAME "/tmp/AWriteFifo" /*$ to read */

/*$ Local function declaration----------------------------------------------*/
static void ArcsEp_initial(void);

static int ArcsEp_send(void const * const buf, int len);

static int ArcsEp_recv(void * const buf, int len);

static int ArcsEp_destroy(void);

/*$ Local arcs port table define--------------------------------------------*/
static TArcsPort l_portTable = {
    .vTable = {
        &ArcsEp_initial,
        &ArcsEp_send,
        &ArcsEp_recv,
        &ArcsEp_destroy
    },
    .avail = (bool)0
};

/*$ Global vairable --------------------------------------------------------*/
TExternPortVtbl *EP0_arcs = (TExternPortVtbl *)&l_portTable;

/*$ fifo decriptor----------------------------------------------------------*/
static int l_fifoRdFd; /* read fifo */
static int l_fifoWrFd; /* write fifo */
int EP0_readFd = -1;

/*! ArcsEp_initial()........................................................*/
static void ArcsEp_initial(void) {
    /* set no block */
    l_fifoRdFd = open(WRFIFO_NAME,
                O_RDONLY | O_NONBLOCK);
    PERROR(l_fifoRdFd, "open Wr fifo");
    
    /* set write no block, Notificaton: write port open must
        be read port Open */
    l_fifoWrFd = open(RDFIFO_NAME,
                O_WRONLY | O_NONBLOCK);
    PERROR(l_fifoRdFd, "open Rd fifo");
    
    l_portTable.avail = (bool)1;
    EP0_readFd = l_fifoRdFd;
    
    DEBUG_INFO("open fifo read fd = %d file = %s",
            EP0_readFd,
            WRFIFO_NAME);
}

/*! ArcsEp_send()...........................................................*/
static int ArcsEp_send(void const * const buf, int len) {
/* \write to write fifo */
    int sendLen = -1;
    if (l_portTable.avail) {
        sendLen = write(l_fifoWrFd, buf, len);
    }
    else {
        /* log Error */
    }
    
    return sendLen;
}

/*! ArcsEp_recv()...........................................................*/
static int ArcsEp_recv(void * const buf, int len) {
/* \recv from read fifo */
    int recvLen = -1;
    if (l_portTable.avail) {
        recvLen = read(l_fifoRdFd, buf, len);
    }
    else {
        /* log Error */
    }
    
    return recvLen;
}

/*! ArcsEp_destroy()........................................................*/
static int ArcsEp_destroy(void) {
    l_portTable.avail = (bool)0;
    close(l_fifoRdFd);
    close(l_fifoWrFd);
    
    return 0;
}

