/*
* @file ring_buffer.c
* @brief ring buffer of char
* @ingroup ring buffer of char
* @cond
******************************************************************************
* Build Date on  2016-11-23
* Last updated for version 1.0.0
* Last updated on  2016-12-1
*
*                    Moltanisk Liang
*                    ---------------------------
*                    Avdecc Reception Control System
*
* Copyright (C) Moltanisk Liang, GuangZhou Suntron. All rights reserved.
******************************************************************************
* @endcond
*/
/*Including File------------------------------------------------------------*/
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "ring_buffer.h"
/*$ RingBuffer::initial()...................................................*/
void RingBuffer_initial(TCharRingBuf *rBuf,
    uint32_t size, uint8_t *pBuf)
{
    rBuf->empty = (bool)1;
    rBuf->bufSize = size;
    rBuf->head = (uint32_t)0;
    rBuf->trail = (uint32_t)0;
    rBuf->pBuf = pBuf;
}
/*$ RingBuffer::isFull()....................................................*/
bool RingBuffer_isFull(TCharRingBuf *rBuf) {
    bool bret = (bool)0;;
    if (rBuf == (TCharRingBuf *)0) {
        return (bool)0;
    }
    /* test condition */
    if ((!rBuf->empty)
          && (rBuf->head == rBuf->trail))
    {
        bret = (bool)1;
    }
    /* return value */
    return bret;
}
/*$ RingBuffer::saveChar()..................................................*/
bool RingBuffer_saveChar(TCharRingBuf *rBuf, uint8_t saveCh) {
    bool bret = (bool)0;
    if (rBuf == (TCharRingBuf *)0) {
        return (bool)0;
    }
    /* save condition satisfied ?*/
    if ((rBuf->empty)
          || (rBuf->head != rBuf->trail))
    {
        rBuf->pBuf[rBuf->trail++] = saveCh;
        rBuf->trail %= rBuf->bufSize;
        /* save char ,buffer is not empty */
        if (rBuf->empty) {
            rBuf->empty = (bool)0;
        }
        /* set get char succussfully */
        bret = (bool)1;
    }
    /* return value */
    return bret;
}
/*$ RingBuffer::getChar()...................................................*/
bool RingBuffer_getChar(TCharRingBuf *rBuf, uint8_t *pGetCh) {
    bool bret = (bool)0;
    if ((rBuf == (TCharRingBuf *)0)
          || (pGetCh == (uint8_t *)0))
    {
        return (bool)0;
    }
    /* ring buffer not empty ? */
    if (!rBuf->empty) {
        *pGetCh = rBuf->pBuf[rBuf->head++];
        rBuf->head %= rBuf->bufSize;
        if (rBuf->head == rBuf->trail) {
            rBuf->empty = (bool)1;
        }
        /* set get char succussfully */
        bret = (bool)1;
    }
    /* return value */
    return bret;
}

