/*
* @file uos.h
* @brief user os define
* @ingroup user os define
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
#ifndef __UOS_H__
#define __UOS_H__

#if defined __linux__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>

#define vsprintf_s vsnprintf
#define InterlockedExchangeAdd __sync_fetch_and_add
/* for linux */
#define OS_PTHREAD_MUTEX_RECURSIVE_TYPE PTHREAD_MUTEX_RECURSIVE_NP

typedef uint32_t Timestamp;
typedef pthread_t *Thread;
typedef sem_t *Semaphore;
typedef pthread_mutex_t CriticalSection;

#elif defined _WIN32 || defined _WIN64

#include <windows.h>
typedef LONGLONG Timestamp;
typedef HANDLE Thread;
typedef HANDLE Semaphore;
typedef CRITICAL_SECTION CriticalSection;

#endif /*defined __linux__ || defined __MACH__*/

#endif /* __UOS_H__ */

