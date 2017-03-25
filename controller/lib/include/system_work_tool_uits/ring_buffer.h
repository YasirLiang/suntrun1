/*
* @file ring_buffer.h
* @brief ring buffer of char
* @ingroup ring buffer of char
* @cond
******************************************************************************
* Build Date on  2016-11-23
* Last updated for version 1.0.0
* Last updated on  2017-03-25
*
*                    Moltanisk Liang
*                    ---------------------------
*                    Avdecc Reception Control System
*
* Copyright (C) Moltanisk Liang, GuangZhou Suntron. All rights reserved.
******************************************************************************
* @endcond
*/
#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

/* c function decleration */
#ifdef __cplusplus /* for c++ transfer */
    extern "C" {
#endif /* __cplusplus */

/*! ring buffer dataType */
typedef struct TCharRingBuf {
	bool empty;         /*! ring buffer empty flags */
	uint8_t *pBuf;      /*! ring buffer pointer */
	uint32_t head;      /*! the head of ring buffer */
	uint32_t trail;     /*! the trail of ring buffer */
	uint32_t bufSize;  /*! the size of ring buffer */
}TCharRingBuf;

/*! RingBuffer_initial------------------------------------------------------*/
void RingBuffer_initial(TCharRingBuf *rBuf,
    uint32_t size, uint8_t *pBuf);

/*! RingBuffer_isFull-------------------------------------------------------*/
bool RingBuffer_isFull(TCharRingBuf *rBuf);

/*! RingBuffer_saveChar-----------------------------------------------------*/
bool RingBuffer_saveChar(TCharRingBuf *rBuf, uint8_t saveCh);

/*! RingBuffer_getChar------------------------------------------------------*/
bool RingBuffer_getChar(TCharRingBuf *rBuf, uint8_t *pGetCh);

#ifdef __cplusplus
} /*end extern "C" */
#endif /* __cplusplus */

#endif /* __RING_BUFFER_H__ */

