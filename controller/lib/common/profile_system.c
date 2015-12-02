/*profile_system.c
**2015/10/28
**
*/

#include "profile_system.h"
#include "terminal_common.h"

int init_profile_system_file( void )
{
	FILE* fd = NULL;
	
	fd = Fopen( STSTEM_SET_STUTUS_PROFILE, "rb+" );
	if( NULL == fd )
	{
		DEBUG_INFO( "open %s Err: not exit!Will create for the system first time!", STSTEM_SET_STUTUS_PROFILE );
		fd = Fopen( STSTEM_SET_STUTUS_PROFILE, "wb+"); // 首次创建文件, 并初始化文件内容
		if( fd == NULL )
		{
			DEBUG_INFO( " init profile system file failed! " );
			return -1;
		}
		else
		{
			
			thost_system_profile_form system_set_form;
			system_set_form.file_crc = 0;
			system_set_form.set_sys.chman_first = 0;
			system_set_form.set_sys.apply_limit = 3;
			system_set_form.set_sys.auto_close = 0;
			system_set_form.set_sys.camara_track = 0;
			system_set_form.set_sys.current_cmr = 1;
			system_set_form.set_sys.speak_limit = 3;
			system_set_form.set_sys.temp_close = 0;
			system_set_form.set_sys.discuss_mode = FREE_MODE;
			system_set_form.set_sys.spk_limtime = 0;
			system_set_form.set_sys.menu_language = 0; // 中文

			// 计算校验
			int i = 0;
			for( i = 0; i < sizeof(thost_system_set); i++ )
			{
				uint8_t* p = (uint8_t*)&system_set_form.set_sys + i;
				system_set_form.file_crc += *p;
			}
			
			if(Fwrite( fd, &system_set_form, sizeof(thost_system_profile_form), 1) != 1)
			{
				DEBUG_INFO( "init profile system file failed: write Err!" );
				Fclose( fd );
				return -1;
			}
		}
	}

	Fclose( fd );
	return 0;
}

/*读系统配置文件，并检查检验。若检验正确则正常输出.适用于配置文件已存在,且以ab+打开文件*/
int profile_system_file_read( FILE* fd,  thost_system_set* system_set )
{
	uint16_t read_crc = 0, count_crc = 0;
	thost_system_profile_form tmp_profile;
	int read_count = 0;
	int i = 0;
	
	if( fd == NULL || system_set == NULL )
	{
#ifdef __DEBUG__
	assert( fd != NULL && system_set != NULL );
#else
	return -1;
#endif
	}
	
	// 设置文件指针在文件的开头处
	if( Fseek( fd, 0, SEEK_SET) == -1 )
		return -1;
	
	if((read_count = Fread( fd, &tmp_profile, sizeof(thost_system_profile_form), 1 )) != 1)
	{
		DEBUG_INFO( "profile system read error!" );
		return -1;
	}

	/*检查校验,这里检验是所有数据之和,除了检验位*/ 
	read_crc = tmp_profile.file_crc;
	for( i = 0; i < sizeof(thost_system_set); i++)
	{
		uint8_t *p = ((uint8_t *)&tmp_profile.set_sys) + i;
		count_crc += *p;
	}

	if( read_crc != count_crc )
	{
		DEBUG_INFO( "profile read err: crc wrong!" );
		return -1;
	}

	//DEBUG_INFO( "sizeof(thost_system_set) = %d", sizeof(thost_system_set) );
	memcpy( system_set, &tmp_profile.set_sys, sizeof(thost_system_set));

	return 0;
}

/*保存系统配置文件, 专门的写函数。适用于配置文件已存在,且以ab+打开文件, index 是save_value在文件结构体成员 thost_system_set 中的偏移*/
int profile_system_file_write( FILE* fd,  const uint8_t save_value,  size_t index )
{
	thost_system_profile_form read_profile;
	uint16_t wr_crc = 0;
	int i = 0;
	
	if( fd == NULL )
	{
#ifdef __DEBUG__
	assert( fd != NULL );
#else
	return -1;
#endif
	}

	if( index > BYTE_DATA_NUM )
	{
		DEBUG_INFO( " index Err : index (%d) > BYTE_DATA_NUM( %d ) ", index, BYTE_DATA_NUM );
		return -1;
	}

	// 设置文件指针在文件的开头处
	if( Fseek( fd, 0, SEEK_SET) == -1 )
		return -1;
	
	// 读出所有数据
	if( profile_system_file_read( fd, &read_profile.set_sys ) == -1 )
	{
		DEBUG_INFO( "err read file!" );
		return -1;
	}

	// 设置新的值save_value
	uint8_t *p1 = (uint8_t *)&read_profile.set_sys;
	if( *(p1 + index) == save_value )
	{
		//DEBUG_INFO( "same system set value! not save" );
		return 0;
	}
	else
	{
		*(p1 + index) = save_value;
	}

	// 重新计算校验,并设置新的检验
	for( i = 0; i <  sizeof(thost_system_set); i++ )
	{
		uint8_t *p2 = ((uint8_t *)&read_profile.set_sys) + i;
		wr_crc += *p2;
	}
	read_profile.file_crc = wr_crc;
	
	// 设置文件指针在文件的开头处
	if( Fseek( fd, 0, SEEK_SET) == -1 )
		return -1;
	
	if (Fwrite( fd, &read_profile, sizeof(thost_system_profile_form), 1 ) != 1 )
	{
		DEBUG_INFO( "write profile system file Err!" );
		return -1;
	}

	return 0;
}

/***************************************************
**Writer:YasirLiang
**Date: 2015/10/29
**Name:terminal_mic_auto_close
**Garam:
**		fd: 文件输出流
**		out_data: 读好的数据的输出缓冲区
**		index: 数据在文件数据区(不包含校验区)中的偏移
**		read_len: 读数据的长度
**Func: procces mic_auto_close system set command
******************************************************/
int profile_system_file_read_byte( FILE* fd, void *out_data, size_t index, uint16_t read_len )
{
	assert( fd && out_data );
	uint16_t read_crc = 0, count_crc = 0;
	uint8_t read_buf[FILE_MAX_READ_BUF] = {0};
	bool is_crc_right = false;

	// 设置文件指针在文件的开头处
	if( Fseek( fd, 0, SEEK_SET) == -1 )
		return -1;

	// 根据文件格式读取校验
	if( Fread( fd, &read_crc, sizeof(uint8_t), 1) != 1)
		return -1;

	// check the crc(校验)
	if( (read_len <= FILE_MAX_READ_BUF ) && (index <= FILE_MAX_READ_BUF ))
	{
		// count the crc of the data and read the file
		int i = 0;
		while( !feof( fd ) )
		{
			uint8_t byte = 0;
			int ret = 0;
			ret = Fread( fd, &byte, sizeof(byte), 1 );
			if( ferror( fd ) ) // 读取错误
			{	
				return -1;
			}

			if( ret != 0 ) // 不是文件末尾
			{
				count_crc += byte;
				read_buf[i++] = byte;
			}
		}

		if( count_crc == read_crc )
			is_crc_right = true;
	}

	if( is_crc_right )
	{
		memcpy((uint8_t*)out_data, &read_buf[index] , read_len);
	}

	return 0;
}

