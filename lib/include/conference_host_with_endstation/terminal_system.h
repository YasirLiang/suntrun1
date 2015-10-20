#ifndef __TERMINAL_SYSTEM_H__
#define __TERMINAL_SYSTEM_H__

#define TMN_RGST_STATE	0x00000001 	 //Զ�˱���
#define CAMERA_PRESET        (0x00000001<<1) //����ͷԤ��
#define SIGN_STATE          	(0x00000001<<2) //ǩ��״̬
#define DISCUSS_STATE         (0x00000001<<3) //����״̬
#define VOTE_STATE          	(0x00000001<<4) //���״̬
#define GRADE_STATE       	(0x00000001<<5) //����״̬
#define ELECT_STATE         	(0x00000001<<6) //ѡ��״̬
#define INTERPOSE_STATE     (0x00000001<<7) //��ϯ�廰
#define HOST_STA_NUM  8

typedef struct _thost_system_state
{
	uint8_t host_state;
}thost_sys_state;

#endif

