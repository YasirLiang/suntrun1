/*
* @file host_controller_debug.h
* @brief controller debug
* @ingroup controller debug
* @cond
******************************************************************************
* Last updated for version 1.0.0
* Last updated on  2016-09-22
*
*                    Moltanisk Liang
*                    ---------------------------
*                    avb auto control system
*
* Copyright (C) Moltanisk Liang, GuangZhou Suntron. All rights reserved.
******************************************************************************
* @endcond
*/
#ifndef __HOST_CONTROLLER_DEBUG_H__
#define __HOST_CONTROLLER_DEBUG_H__
/*Including files-----------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include "jdksavdecc_world.h"

#ifndef __DEBUG__
/*! Macro Debug Switch */
/*#define __DEBUG__*/
#ifdef __DEBUG__/*! Debug information printf */
#include <assert.h>	/*! Enable assert program */
/*! Debug line */
#define DEBUG_LINE() fprintf(stdout,"\033[32m[%s:%s] line = %d\033[0m\r\n",\
                                                 __FILE__, __func__, __LINE__)
/*! Debug Err */
#define DEBUG_ERR(fmt, args...) fprintf(stdout,\
                             "\033[46;31m[%s:%s:%d]\033[0m "fmt" errno=%d\t",\
                     __FILE__, __func__, __LINE__, ##args, errno); perror(fmt)
/*! Debug Info */
#define DEBUG_INFO(fmt, args...)\
                       fprintf(stdout,"\033[32m %s-%s-%d:\033[0m "fmt" \r\n",\
                                         __FILE__, __func__, __LINE__, ##args)
/*! Debug Only Info */
#define DEBUG_ONINFO(fmt, args...)\
                           fprintf(stdout, "\033[34m"fmt"\033[0m\r\n", ##args)
/*! Debug Message */
#define DEBUG_MSGINFO(fmt, args...)\
   fprintf(stdout,"%s:%s:%d: "fmt"\r\n", __FILE__, __func__, __LINE__, ##args)
/*! Debug recieve Buffer */
#define DEBUG_RECV(buf, n, arg) \
    do {\
        int i;\
        uint8_t *p = (uint8_t*)(buf);\
        fprintf( stdout,"\033[32m[%s:%d]\033[0m %s:\t",\
                                                    __func__, __LINE__, arg);\
        for (i = 0;i < n; i++) {\
            fprintf(stdout, "%02x ", p[i]);\
        }\
        fprintf(stdout, "\n");\
    }while (0)
/*! Debug send Buffer */
#define DEBUG_SEND( buf, n, arg )\
    do {\
        int i;\
        uint8_t *p = (uint8_t*)buf;\
        fprintf( stdout,"\033[32m[%s:%d]\033[0m %s:\t",\
                                                    __func__, __LINE__, arg);\
        for (i = 0; i < n; i++) {\
            fprintf(stdout, "%02x ", p[i]);\
        }\
        fprintf(stdout, "\n");\
    }while (0)
/*! Debug message */
#define MSGINFO(fmt, args...) \
                            fprintf(stdout,""fmt"", ##args)
/*! cond DABORT */	
#define DABORT(cond)\
    if (!(cond)) {\
        fprintf(stdout, "Aborting at line %d in source file %s\n",\
                        __LINE__,__FILE__);\
        abort();\
    }
#else/*! release version */
/*! Assert switch Macro, if it is defined, assert will not excute */
/* #define NDEBUG */
#include <assert.h>	/*! include assert file */	
/*! Debug line */
#define DEBUG_LINE()
/*! Debug Err */
#define DEBUG_ERR(fmt, ...)
/*! Debug Info */
#define DEBUG_INFO(fmt, ...)
/*! Debug Only Info */
#define DEBUG_ONINFO(fmt, args...)
/*! Debug recieve Buffer */
#define DEBUG_RECV(buf, n, arg)
/*! Debug send Buffer */
#define DEBUG_SEND(buf, n, arg)
/*! Debug message */
#define DEBUG_MSGINFO(fmt, args...)
/*! Debug message */
#define MSGINFO(fmt, args...) fprintf(stdout,""fmt"", ##args)
/*! cond DABORT */
#define DABORT(cond)
#endif /* __DEBUG__ */
#endif /*__DEBUG__*/
#endif /* __HOST_CONTROLLER_DEBUG_H__ */

