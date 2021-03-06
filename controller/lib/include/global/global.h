/*
* @file
* @brief system model name Macros
* @ingroup system model name
* @cond
******************************************************************************
* First Create on 2016-5-17
* Last updated for version 1.0.0
* Last updated on  2016-09-04
*
*                    Moltanisk Liang
*                    ---------------------------
*                    avb auto control system
*
* Copyright (C) Moltanisk Liang, GuangZhou Suntron. All rights reserved.
******************************************************************************
* @endcond
*/
#ifndef __GLOBAL_H__
#define __GLOBAL_H__

/*! enable arm version for define of arm back track*/
#define __ARM_BACK_TRACE__                                    

/*! system ccu control unit num*/
#define central_control_unit_max_num            (3)

/*! system ccu connect max number*/
#define PER_CCU_CONNECT_MAX_NUM                 (2)

/*! Max channel */
#define CHANNEL_MUX_NUM \
    (central_control_unit_max_num * PER_CCU_CONNECT_MAX_NUM)

/*! the name of central control uint name*/
#define central_control_unit_name "AVB 4in/4out"

/*! the name of transmit central control unit name*/
#define central_control_unit_transmit_name "DCS8000"

/*! the name of avb switch name */
#define avb_switch_name "default_entity_name"

/*! stream ouput of central control transmit unit*/
#define central_control_transmit_uint_output    (0)

/*! conference unit name*/
#define conference_uint_name "AVB_R 4in/4out"

/*! stream input of conference recieve uint*/
#define conference_uint_recieve_uint_input      (0)

/*! stream ouput of conference transmit unit*/
#define conference_uint_transmit_uint_ouput     (0)

/*! software version Macro*/
#define software_version "DCS6000_HC_V1.0.0"

/*! DISABLE MUTICASTOR*/
#define DISABLE_MUTI

/*! Max System Terminal's Number */
#define SYSTEM_TMNL_MAX_NUM 1024

/*! System Max Speaking Number */
#if CHANNEL_MUX_NUM
#define MAX_LIMIT_SPK_NUM CHANNEL_MUX_NUM
#else
#define MAX_LIMIT_SPK_NUM                       (6)
#endif

/*! Sytem Max Apply Number */
#define MAX_LIMIT_APPLY_NUM                     (6)

/*$ MICPHONE priority manger macro define-----------------------------------*/
#define MIC_PRIOR_MANEGER_ENABLE

/*$ network initial enable--------------------------------------------------*/
//#define NETWORK_INIT_ENABLE

#endif /* __GLOBAL_H__ */

