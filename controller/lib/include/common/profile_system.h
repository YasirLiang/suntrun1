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
	VAL_CHM_PRIOR_ENUM_PROFILE=0,	// 主席优先 1 
	VAL_CHM_MUSIC_PROFILE,			// 主席音乐 2
	VAL_AUTO_CLOSE_PROFILE,			// 自动关闭 3
	VAL_DSCS_MODE_PROFILE,			// 讨论模式 9
	VAL_SPKER_LIMIT_PROFILE,			// 代表发言人数限制 4
	VAL_APPLY_LIMIT_PROFILE,			// 申请人数限制 5 
	VAL_CHM_TIMED_ENUM_PROFILE,		// 主席发言限时 6
	VAL_VIP_TIMED_ENUM_PROFILE,		// VIP发言限时 7 
	VAL_SPK_LIMIT_TIME_PROFILE,		// 发言限制时长 8 

	VAL_TEMP_CLOSE_PROFILE,			// 暂时关闭,主机与终端 主席优先权限。1、静音；0、关闭
	VAL_SINGIN_TYPE_PROFILE,			// 签到类型 
	VAL_CMR_TRACK_ENUM_PROFILE,		// 摄像跟踪
	VAL_CUR_CMR_PROFILE,				// 当前摄像头
	VAL_MENU_LANG_PROFILE,			// 菜单语言
	BYTE_DATA_NUM_PROFILE	
}ENUM_SYSTEM_SET_INDEX;


typedef struct _tthost_system_set
{
	uint8_t chman_first;	// 主席优先 1
	uint8_t chman_music;	// 主席音乐 2
	uint8_t auto_close;		// 自动关闭 3
	uint8_t discuss_mode;	// 讨论模式 9
	uint8_t speak_limit;		// 代表发言人数限制 4
	uint8_t apply_limit;		// 申请人数限制 5 
	uint8_t chman_limitime;	// 主席发言限时 6 只有1和0，代表限时与不限时
	uint8_t vip_limitime;	// VIP发言限时 7 只有1和0，代表限时与不限时
	uint8_t spk_limtime;	// 发言限制时长 8 0 为限时，1-63代表限时时间
	
	uint8_t temp_close;		// 暂时关闭,主机与终端 主席优先权限。1、静音；0、关闭
	uint8_t sign_type;		// 签到类型
	uint8_t camara_track;	// 摄像跟踪
	uint8_t current_cmr;	// 当前摄像机
	uint8_t menu_language;	// 菜单语言
}thost_system_set;

typedef struct _thost_system_profile_form // 系统配置文件的格式 两个字节 + 系统的设置状态
{
	uint16_t file_crc;			// 文件的校验，所有数据的和
	thost_system_set set_sys;	// 系统配置信息
}thost_system_profile_form;

extern FILE *profile_file_fd;
extern thost_system_set gset_sys; 							// 系统配置文件的信息

int profile_system_file_write( FILE* fd,  const uint8_t save_value,  size_t index );
int profile_system_file_read( FILE* fd,  thost_system_set* system_set );
int init_profile_system_file( void );
int profile_system_file_read_byte( FILE* fd, void *out_data, size_t index, uint16_t read_len );
void profile_system_file_write_timeouts( void );
bool is_profile_system_file_write_timeouts( struct inflight_timeout *timer );
int profile_system_file_write_gb_param( FILE* fd, thost_system_set *p_set_sys );
void profile_system_close( void );


#endif

