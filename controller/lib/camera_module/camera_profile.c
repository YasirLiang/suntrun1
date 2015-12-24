/*
**camera_profile.c
**2015-12-08
**
*/

#include "camera_profile.h"
#include "host_controller_debug.h"

/*检查读出的数据是否有效 preset_num 必须小于 PRESET_NUM_MAX*/
bool camera_profile_check_right( camera_profile_format *set_buf, const int preset_num )
{
	uint16_t read_chk = 0, count_chk = 0;
	uint8_t *p_buf = NULL;
	bool check_right = false;
	int i = 0;
	
	assert( set_buf );
	read_chk = set_buf->file_chk;
	p_buf = (uint8_t*)set_buf->cmr_preset_list;
	
	if( preset_num > PRESET_NUM_MAX )
	{
		DEBUG_INFO( "out of presetting max num!" );
		assert( preset_num <= PRESET_NUM_MAX );
	}
	else if( preset_num > 0)
	{
		for( i = 0; i < sizeof(preset_point_format) * preset_num; i++ )
		{
			count_chk += *p_buf++;
		}

		if( read_chk == count_chk )
		{
			check_right = true;
		}
	}
	else
	{
		DEBUG_INFO( "invalue preset num: %d", preset_num );
	}

	return check_right;
}

/*填充摄像头预置位配置文件的检验位 preset_num 必须小于 PRESET_NUM_MAX*/
int camera_profile_fill_check( camera_profile_format *set_buf, const int preset_num )
{
	uint8_t *p_buf = NULL;
	int i = 0, ret = -1;
	
	assert( set_buf );
	set_buf->file_chk = 0;
	p_buf = (uint8_t*)set_buf->cmr_preset_list;
	
	if( preset_num > PRESET_NUM_MAX )
	{
		DEBUG_INFO( "out of presetting max num!" );
		assert( preset_num <= PRESET_NUM_MAX );	
	}
	else if( preset_num > 0 )
	{
		for( i = 0; i < (sizeof(preset_point_format) * preset_num); i++ )
		{
			set_buf->file_chk += *p_buf++; // 先解引用指针再自加
		}

		ret = i;
	}
	else
	{
		DEBUG_INFO( "invalue preset num: %d", preset_num );
	}

	return ret;
}

FILE* camera_profile_open( const char *file_string, const char *open_mode )
{
	assert( file_string && open_mode );

	return Fopen( file_string, open_mode );
}

int camera_profile_read( FILE *fd, camera_profile_format *read_buf )
{
	assert( fd && read_buf );
	int ret = Fread( fd, read_buf, sizeof(camera_profile_format), 1 );
	if( ret != 1 )
	{
		DEBUG_ERR( "camera_profile_read ERROR!" );
		ret = -1;
	}

	return ret;
}

int camera_profile_write( FILE *fd, camera_profile_format *write_buf )
{
	assert( fd && write_buf );
	int ret = Fwrite( fd, write_buf, sizeof(camera_profile_format), 1);
	if( ret != 1 )
	{
		DEBUG_ERR( "camera profile write ERROR!" );
		ret = -1;
	}

	return ret;
}

int camera_profile_close( FILE *fd )
{
	assert( fd );

	Fclose( fd );

	return 0;
}

