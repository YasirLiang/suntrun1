/*
* @file pelco_d.c
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
/*Including-----------------------------------------------------------------*/

#include "pelco_d.h"

/*$ pelco_d_cammand_set()...................................................*/
int pelco_d_cammand_set(uint8_t camera_address, uint16_t d_cmd,
    uint8_t speed_lv, uint8_t speed_vertical, pelco_d_format* askbuf)
{
    assert(askbuf != (pelco_d_format*)0);

    /* format buffer */
    askbuf->sync = CAMERA_SYNC_BYTE_HEAD;
    askbuf->bit_id = camera_address;
    askbuf->order = d_cmd;
    askbuf->data_code_1.level_speed = speed_lv;
    askbuf->data_code_2.vertical_speed = speed_vertical;
    askbuf->check_digit = (uint8_t)CHECK_DIGIT_RESULT(camera_address,
        (uint8_t)(d_cmd&0x00ff), (d_cmd >> 8)&0x00ff,
        speed_lv, speed_vertical);
    /* return value */
    return 0;
}
/*$ pelco_d_cammand_get()...................................................*/
uint16_t pelco_d_cammand_get(const uint8_t * frame, uint16_t pos) {
    assert(frame != (uint8_t*)0);
    return (((uint16_t)frame[2]) |((uint16_t)frame[3] << 8));
}

