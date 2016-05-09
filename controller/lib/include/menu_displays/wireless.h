#ifndef _WIRELESS_H_
#define _WIRELESS_H_

#define WIRELESS_MSG_LEN	6

#define STUDY_KEY_VALUE        0x01
#define QUERY_KEY_VALUE        0x11
#define CHANGE_KEY_VALUE       0x21
#define UP_KEY_VALUE           0x22
#define DOWN_KEY_VALUE         0x23
#define LEFT_KEY_VALUE         0x24
#define RIGHT_KEY_VALUE        0x25
#define UP_LEFT_KEY_VALUE      0x26
#define UP_RIGHT_KEY_VALUE     0x27
#define DOWN_RIGHT_KEY_VALUE   0x28
#define DOWN_LEFT_KEY_VALUE    0x29
#define ZOOM_IN_KEY_VALUE      0x2b
#define ZOOM_OUT_KEY_VALUE     0x2c
#define AUTO_FOCUS_KEY_VALUE   0x2d
#define FOCUS_FAR_KEY_VALUE    0x2e
#define FOCUS_NEAR_KEY_VALUE   0x2f
#define LOCK_KEY_VALUE         0x30
#define UNLOCK_KEY_VALUE       0x31
#define CHANGE_CAM1_KEY_VALUE  0x41
#define CHANGE_CAM2_KEY_VALUE  0x42
#define CHANGE_CAM3_KEY_VALUE  0x43
#define CHANGE_CAM4_KEY_VALUE  0x44
#define STOP_KEY_VALUE         0x55


void wireless_pross(unsigned char *pWirData);
extern void RCtrlAlignEn(int flag); // moltanisk add in 2016-5-09 1:Ê¹ÄÜ£¬0:½ûÖ¹

#endif
