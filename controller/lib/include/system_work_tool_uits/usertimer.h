/*
* @file usertimer.h
* @brief user timer
* @ingroup user timer
* @cond
******************************************************************************
* Build Date on  2016-10-25
* Last updated for version 1.0.0
* Last updated on  2016-10-25
*
*                    Moltanisk Liang
*                    ---------------------------
*                    Avdecc Reception Control System
*
* Copyright (C) Moltanisk Liang, GuangZhou Suntron. All rights reserved.
******************************************************************************
* @endcond
*/
#ifndef __USERTIMER_H__
#define __USERTIMER_H__

#include "uos.h"

typedef struct TUserTimer {
    bool running;
    bool elapsed;
    uint32_t count;
    Timestamp start_time;
}TUserTimer;

/* c function decleration */
#ifdef __cplusplus /* for c++ transfer */
    extern "C" {
#endif /* __cplusplus */

extern void userTimerStart(uint32_t duration_ms, TUserTimer *timer);
extern void userTimerStop(TUserTimer *timer);
extern bool userTimerTimeout(TUserTimer *timer);

#ifdef __cplusplus
} /*end extern "C" */
#endif

#endif /* __USERTIME_H__ */

