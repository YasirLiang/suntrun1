/*
**camera_pro.h
**2015-12-8
**
**����ͷ��������ͷ�ļ�
*/

#ifndef __CAMERA_PRO_H__
#define __CAMERA_PRO_H__

#include "camera_profile.h"
#include "host_controller_debug.h"

#ifdef __DEBUG__
#define __CAMERA_DEBUG__ 
#endif

#define PRESET_SYSTEM_FILE "preset_plist.dat"
#define MUX_NUM_CAMERA 4
#define MAX_PRESET_ONE_CAMERA 128
#define FULL_VIEW_ADDR  0x1000
#define BACKUP_FULL_VIEW_ADDR	0x0FFF

typedef struct get_cmrpreset_pro
{
	uint8_t busy_flag;
	uint8_t camera_num;
	uint8_t preset_num;
	uint8_t out;
}get_cmrpreset_pro;

extern preset_point_format gcurpresetcmr;					// ��ǰԤ�õ�

/*=============================
*��ʼ����ͷ�������
*==============================*/
int camera_preset_save( uint16_t cmd, void *data, uint32_t data_len );
int camera_preset_addr_select( uint16_t cmd, void *data, uint32_t data_len );
int camera_select_num( uint16_t cmd, void *data, uint32_t data_len );// �˺���δ��ɽӿ�ʵ��2015-12-09
int camera_control_rightleft( uint16_t cmd, void *data, uint32_t data_len );
int camera_control_updown( uint16_t cmd, void *data, uint32_t data_len );
int camera_control_fouce( uint16_t cmd, void *data, uint32_t data_len );
int camera_control_zoom( uint16_t cmd, void *data, uint32_t data_len );
int camera_control_iris( uint16_t cmd, void *data, uint32_t data_len );
int camera_control_align( uint16_t cmd, void *data, uint32_t data_len );
int camera_control_full_view( uint16_t cmd, void *data, uint32_t data_len );
int camera_preset_set( uint16_t cmd, void *data, uint32_t data_len );
int camera_get_preset( uint16_t cmd, void *data, uint32_t data_len ); // �˺���δ���2015-12-09

/*=============================
*��������ͷ�������
*==============================*/

/*=============================
*��ʼ����ͷ���̴���
*==============================*/
void camera_save_Cmrpreset_direct( void );
bool camera_prese_num( uint8_t camera_num, uint8_t *index );// ÿ������ͷ������Ŀ��õ�Ԥ�õ�
bool camera_preset_list_exit_addr( uint16_t addr, uint16_t *p_index );
void camera_pro_enter_preset( void );
void camera_pro_esc_preset( void );
void camera_pro_lock_flags( uint8_t option );
int camera_pro_control( uint8_t  cmr_addr, uint16_t d_cmd, uint8_t speed_lv, uint8_t speed_vertical );
int camera_pro_preset_file_list_init( void );
void camera_pro_init( void ); // ������ϵͳ���ò�����ȡ��ɲ��ܵ���
void camera_pro_system_close( void );
int camera_pro_timetick( void );
void camera_pro( void );
/*=============================
*��������ͷ���̴���
*==============================*/
#endif
