/*profile_system.h
**2015/10/28
**
*/

#ifndef __PROFILE_SYSTEM_H__
#define __PROFILE_SYSTEM_H__

#include "jdksavdecc_world.h"
#include "file_util.h"
#include "host_controller_debug.h"
#include "inflight.h"

#define STSTEM_SET_STUTUS_PROFILE "system.dat"

typedef enum
{
	VAL_CHM_PRIOR_ENUM_PROFILE=0,	// ��ϯ���� 1 
	VAL_CHM_MUSIC_PROFILE,			// ��ϯ���� 2
	VAL_AUTO_CLOSE_PROFILE,			// �Զ��ر� 3
	VAL_DSCS_MODE_PROFILE,			// ����ģʽ 9
	VAL_SPKER_LIMIT_PROFILE,			// �������������� 4
	VAL_APPLY_LIMIT_PROFILE,			// ������������ 5 
	VAL_CHM_TIMED_ENUM_PROFILE,		// ��ϯ������ʱ 6
	VAL_VIP_TIMED_ENUM_PROFILE,		// VIP������ʱ 7 
	VAL_SPK_LIMIT_TIME_PROFILE,		// ��������ʱ�� 8 

	VAL_TEMP_CLOSE_PROFILE,			// ��ʱ�ر�,�������ն� ��ϯ����Ȩ�ޡ�1��������0���ر�
	VAL_SINGIN_TYPE_PROFILE,			// ǩ������ 
	VAL_CMR_TRACK_ENUM_PROFILE,		// �������
	VAL_CUR_CMR_PROFILE,				// ��ǰ����ͷ
	VAL_MENU_LANG_PROFILE,			// �˵�����
	BYTE_DATA_NUM_PROFILE	
}ENUM_SYSTEM_SET_INDEX;


typedef struct _tthost_system_set
{
	uint8_t chman_first;	// ��ϯ���� 1
	uint8_t chman_music;	// ��ϯ���� 2
	uint8_t auto_close;		// �Զ��ر� 3
	uint8_t discuss_mode;	// ����ģʽ 9
	uint8_t speak_limit;		// �������������� 4
	uint8_t apply_limit;		// ������������ 5 
	uint8_t chman_limitime;	// ��ϯ������ʱ 6 ֻ��1��0��������ʱ�벻��ʱ
	uint8_t vip_limitime;	// VIP������ʱ 7 ֻ��1��0��������ʱ�벻��ʱ
	uint8_t spk_limtime;	// ��������ʱ�� 8 0 Ϊ��ʱ��1-63������ʱʱ��
	
	uint8_t temp_close;		// ��ʱ�ر�,�������ն� ��ϯ����Ȩ�ޡ�1��������0���ر�
	uint8_t sign_type;		// ǩ������
	uint8_t camara_track;	// �������
	uint8_t current_cmr;	// ��ǰ�����
	uint8_t menu_language;	// �˵�����
}thost_system_set;

typedef struct _thost_system_profile_form // ϵͳ�����ļ��ĸ�ʽ �����ֽ� + ϵͳ������״̬
{
	uint16_t file_crc;			// �ļ���У�飬�������ݵĺ�
	thost_system_set set_sys;	// ϵͳ������Ϣ
}thost_system_profile_form;

extern FILE *profile_file_fd;
extern thost_system_set gset_sys; 							// ϵͳ�����ļ�����Ϣ

int profile_system_file_write( FILE* fd,  const uint8_t save_value,  size_t index );
int profile_system_file_read( FILE* fd,  thost_system_set* system_set );
int init_profile_system_file( void );
int profile_system_file_read_byte( FILE* fd, void *out_data, size_t index, uint16_t read_len );
void profile_system_file_write_timeouts( void );
bool is_profile_system_file_write_timeouts( struct inflight_timeout *timer );
int profile_system_file_write_gb_param( FILE* fd, thost_system_set *p_set_sys );
void profile_system_close( void );


#endif

