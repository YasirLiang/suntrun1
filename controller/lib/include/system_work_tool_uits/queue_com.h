/*
* @file queue_com.h
* @brief queue data type
* @ingroup queue
* @cond
******************************************************************************
* Build Date on  2016-12-12
* Last updated for version 1.0.0
* Last updated on  2016-12-12
*
*                    Moltanisk Liang
*                    ---------------------------
*                    avb auto control system
*
* Copyright (C) Moltanisk Liang, GuangZhou Suntron. All rights reserved.
******************************************************************************
* @endcond
*/
#ifndef __QUEUE_COM_H__
#define __QUEUE_COM_H__

/* c function decleration */
#ifdef __cplusplus /* for c++ transfer */
    extern "C" {
#endif /* __cplusplus */

/*$ queue define------------------------------------------------------------*/
typedef struct TComQueue {
    uint32_t head;     /*! head of queue*/
    uint32_t trail;    /*! trail of queue*/
    uint32_t count;    /*! count of queue */
    uint32_t size;     /*! size of queue*/
    /*! const point to buf of queue
       \ element in buf is pointer for can
       \ operation in common function */
    uint32_t *pBuf;
}TComQueue;
/*! QueueCom_isEmpty--------------------------------------------------------*/
extern bool QueueCom_isEmpty(TComQueue * const queue);
/*! QueueCom_isFull---------------------------------------------------------*/
extern bool QueueCom_isFull(TComQueue * const queue);
/*! QueueCom_postFiFo-------------------------------------------------------*/
extern bool QueueCom_postFiFo(TComQueue * const queue,
                                     void **node);
/*! QueueCom_popFiFo--------------------------------------------------------*/
extern bool QueueCom_popFiFo(TComQueue * const queue,
                                     uint32_t *addr);
/*! QueueCom_itemFree-------------------------------------------------------*/
extern void QueueCom_itemFree(void *p);
/*! QueueCom_count----------------------------------------------------------*/
uint32_t QueueCom_count(TComQueue * const queue);

#ifdef __cplusplus
} /*end extern "C" */
#endif /* __cplusplus */

#endif /* __EXTERN_PORT_H__ */

