/*
**camera_profile.h
**2015-12-08
**
*/

#ifndef __CAMERA_PROFILE_H__
#define __CAMERA_PROFILE_H__

#include "file_util.h"

#define PRESET_NUM_MAX 100 // 摄像机最大的预置点数

typedef struct preset_point_format 
{
	uint8_t  camera_num; // 所属的摄像头地址.注:这里的摄像头地址从1开始， 也是对应的摄像头地址,也是摄像头的编号
	uint8_t preset_point_num;// 预置点数
	uint16_t tmnl_addr;// 终端地址
}preset_point_format;

typedef struct camera_profile_format // 预置点保存的文件格式节点
{
	uint16_t file_chk;
	preset_point_format cmr_preset_list[PRESET_NUM_MAX];
}camera_profile_format;

bool camera_profile_check_right( camera_profile_format *set_buf, const int preset_num );
int camera_profile_fill_check( camera_profile_format *set_buf, const int preset_num );
FILE* camera_profile_open( const char *file_string, const char *open_mode );
int camera_profile_read( FILE *fd, camera_profile_format *read_buf );
int camera_profile_write( FILE *fd, camera_profile_format *write_buf );
int camera_profile_close( FILE *fd );


#endif
