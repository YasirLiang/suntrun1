/*
* @file pelco_d.h
* @brief protocal pelco_d
* @ingroup protocal pelco_d
* @cond
******************************************************************************
* Build Date
* Last updated for version 1.0.0
* Last updated on  2017-2-24
*
*                    Moltanisk Liang
*                    ---------------------------
*                    Avdecc Reception Control System
*
* Copyright (C) Moltanisk Liang, GuangZhou Suntron. All rights reserved.
******************************************************************************
* @endcond
*/
#ifndef __PELCO_D_H__
#define __PELCO_D_H__

/*Including-----------------------------------------------------------------*/
#include "jdksavdecc_world.h"
#include "host_controller_debug.h"

/*$ pelco_d param define----------------------------------------------------*/
#define CAMERA_SYNC_BYTE_HEAD    (0xFF)
/*$ \*/
#define CAMERA_PELCO_D_DEAL_LEN  (7)
/*$ \*/
#define CAMERA_CTRL_STOP         (0x0000)
/*$ \*/
#define CAMERA_CTRL_AUTO_SCAN    (0x0010)
/*$\*/
#define CAMERA_CTRL_IRIS_CLOSE   (0x0004)
/*$ \*/
#define CAMERA_CTRL_IRIS_OPEN    (0x0002)
/*$ \*/
#define CAMERA_CTRL_FOCUCS_NEAR  (0x0001)
/*$\*/
#define CAMERA_CTRL_FOCUCS_FAR   (0x8000)
/*$ \*/
#define CAMERA_CTRL_ZOOM_WIDE    (0x4000)
/*$ \*/
#define CAMERA_CTRL_ZOOM_TELE    (0x2000)
/*$ \*/
#define CAMERA_CTRL_DOWN         (0x1000)
/*$ \*/
#define CAMERA_CTRL_UP           (0x0800)
/*$\*/
#define CAMERA_CTRL_LEFT         (0x0400)
/*$ \*/
#define CAMERA_CTRL_RIGHT        (0x0200)
/*$ \*/
#define CAMERA_CTRL_PRESET_SET   (0x0300)
/*$ \*/
#define CAMERA_CTRL_PRESET_CALL  (0x0700)

/*$ \*/
#define DIVISOR_CHECK_COUNT 0x000000FF
/*$ count crc */
#define CHECK_DIGIT_RESULT(addr, cmd1, cmd2, data1, data2)\
    (((addr) + (cmd1) + (cmd2) + (data1) + (data2)) & (DIVISOR_CHECK_COUNT))

/*! level speed-------------------------------------------------------------*/
typedef struct _type_control_level_speed {
    uint8_t level_speed; /*! range((00-3FH)) */
}control_lv_speed;
/*! vertical speed----------------------------------------------------------*/
typedef struct _type_control_vertical_speed {
    uint8_t vertical_speed; /*! range((00-3FH)) */
}control_vtcl_speed;
/*! D protocal command format-----------------------------------------------*/
typedef struct _pelco_d_command_form {
    uint8_t sync;                   /*! sync byte */
    uint8_t bit_id;                 /*! address id */
    uint16_t order;                 /*! opt one */
    control_lv_speed data_code_1;   /*! data code 1*/
    control_vtcl_speed data_code_2; /*! data code 2*/
    uint8_t check_digit;            /*! check crc */
}pelco_d_format;
/*$ \*/
int pelco_d_cammand_set(uint8_t camera_address, uint16_t d_cmd,
    uint8_t speed_lv, uint8_t speed_vertical, pelco_d_format* askbuf);
/*$ \*/
uint16_t pelco_d_cammand_get(const uint8_t* frame, uint16_t pos);

#endif /* __PELCO_D_H__ */

