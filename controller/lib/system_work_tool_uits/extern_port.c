/*
* @file extern_port.c
* @brief exern port supper
* @ingroup extern port supper
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
#include "extern_port.h"

/*$ ExternPort::init()......................................................*/
void ExternPort_init(TExternPortVtbl const * const ptr) {
    ptr->init();
}

/*$ ExternPort::send()......................................................*/
int ExternPort_send(TExternPortVtbl const * const ptr,
    void const * const buf, int len)
{
    return ptr->send(buf, len);
}

/*$ ExternPort::recv()......................................................*/
int ExternPort_recv(TExternPortVtbl const * const ptr,
    void * const buf, int len)
{
    return ptr->recv(buf, len);
}

/*$ ExternPort::destroy()...................................................*/
int ExternPort_destroy(TExternPortVtbl const * const ptr) {
    return ptr->destroy();
}

