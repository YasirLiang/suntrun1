/*
* @file queue_com.c
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
/* Including file-----------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "queue_com.h"
/*${QueueCom::count}........................................................*/
/*${QueueCom::count}*/
uint32_t QueueCom_count(TComQueue * const queue) {
    return queue->count;
}
/*${QueueCom::isEmpty}......................................................*/
/*${QueueCom::isEmpty}*/
bool QueueCom_isEmpty(TComQueue * const queue) {
    if (queue == (TComQueue * const)0) {
        return (bool)1;
    }
    return queue->head == queue->trail;
}
/*${QueueCom::isFull}.......................................................*/
/*${QueueCom::isFull}*/
bool QueueCom_isFull(TComQueue * const queue) {
    if (queue == (TComQueue * const)0) {
        return (bool)1;
    }
    return (queue->trail + 1) % queue->size == queue->head;
}
/*${QueueCom::postFiFo}.....................................................*/
/*${QueueCom::postFiFo}*/
bool QueueCom_postFiFo(TComQueue * const queue,
                                     void **node)
{
    if (!QueueCom_isFull(queue)) {
        queue->pBuf[queue->trail] = (uint32_t)node;
        queue->trail = (queue->trail + 1U) % queue->size;
        queue->count++;
        return (bool)1;
    }
    return (bool)0;
}
/*${QueueCom::popFiFo}......................................................*/
/*${QueueCom::popFiFo}*/
bool QueueCom_popFiFo(TComQueue * const queue,
                                     uint32_t *node) /* get pointer */
{
    if (!QueueCom_isEmpty(queue)) {
        *node = queue->pBuf[queue->head];
        queue->head = (queue->head + 1U) % queue->size;
        queue->count--;
        return (bool)1;
    }
    return (bool)0;
}
/*${QueueCom::itemFree}......................................................*/
/*${QueueCom::itemFree}*/
void QueueCom_itemFree(void *p) {
    free(p);
}
