/*
* @file arcs_extern_port.h
* @brief avdecc reception control system extern port
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
#ifndef __ARCS_EXTERN_PORT_H__
#define __ARCS_EXTERN_PORT_H__

/*$ */ 
#include "extern_port.h"

TExternPortVtbl *EP0_arcs;
extern int EP0_readFd;

#endif /* __ARCS_EXTERN_PORT_H__ */

