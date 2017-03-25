/*
* @file extern_port.h
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
#ifndef __EXTERN_PORT_H__
#define __EXTERN_PORT_H__

/* c function decleration */
#ifdef __cplusplus /* for c++ transfer */
    extern "C" {
#endif /* __cplusplus */

/*! extern port virtual table */
typedef struct TExternPortVtbl {
    /*! initial function for specific port */
    void (*init)(void);
    
    /*! send data function for specific port */
    int (*send)(void const * const, int);
    
    /*! recv data function for specific port */
    int (*recv)(void * const, int);
    
    /*! port destroy function for specific port */
    int (*destroy)(void);
}TExternPortVtbl;

/*! ExternPort::init()......................................................*/
void ExternPort_init(TExternPortVtbl const * const ptr);

/*! ExternPort::send()......................................................*/
int ExternPort_send(TExternPortVtbl const * const ptr,
    void const * const buf, int len);

/*! ExternPort::recv()......................................................*/
int ExternPort_recv(TExternPortVtbl const * const ptr,
    void * const buf, int len);

/*! ExternPort::destroy()...................................................*/
int ExternPort_destroy(TExternPortVtbl const * const ptr);

#ifdef __cplusplus
} /*end extern "C" */
#endif /* __cplusplus */

#endif /* __EXTERN_PORT_H__ */

