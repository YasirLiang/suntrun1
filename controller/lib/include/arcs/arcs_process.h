/*
* @file arcs_process.h
* @brief avdecc reception control system processing
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
#ifndef __ARCS_PROCESS_H__
#define __ARCS_PROCESS_H__
/*$ */
extern int ArcsCmd_queryId(uint16_t cmd, void *data, uint32_t data_len);
/*$ */
extern int ArcsCmd_switchMatrix(uint16_t cmd, void *data, uint32_t data_len);
/*$ */
extern int ArcsCmd_optTmnl(uint16_t cmd, void *data, uint32_t data_len);
/*$ */
extern int ArcsCmd_sysSet(uint16_t cmd, void *data, uint32_t data_len);
/*$ */
extern int ArcsCmd_cmrCtl(uint16_t cmd, void *data, uint32_t data_len);

#endif /* __ARCS_PROCESS_H__ */
