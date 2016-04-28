/**
*file:en485_send.c
*build date:27-4-2016
*writer:YasirLiang
*/

//******************
//使能底板485发送数据
//
//******************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "enum.h"
#include "host_controller_debug.h"
#include "en485_send.h"
#include "log_machine.h"

#define ENPIN485_DRV_FILE "/dev/enpin485"// 使能458发送驱动字符设备文件
#define FAIL_INIT_NUM 5// 错误初始化失败的次数

#define MAGIC		'E'
#define PIN485_DISEN	_IO(MAGIC,0)
#define PIN485_EN		_IO(MAGIC,1)

#define delay(x) {int loop; for( loop = 0; loop < x;loop++ ); }

static int gen485_fd = -1; // 使能458发送驱动字符设备文件描述符

/*********************************
*Auther:YasirLiang
*date:27-4-2016
*Name:en458_send_init
*Function:底板458端口发送使能
*Param:None
*Return Value:使能成功返回真，否则返回假
*注:gp_log_imp必须是实例化了的
**********************************/
bool en485_send_init( void )
{
	int i = 0;
	
	gen485_fd = open( ENPIN485_DRV_FILE, O_RDWR );
	if( gen485_fd < 0 )
	{
		if( gp_log_imp != NULL )
		gp_log_imp->log.post_log_msg( &gp_log_imp->log, 
									LOGGING_LEVEL_ERROR, 
									"open enpin485 file( %s )failed:No such file", 
									ENPIN485_DRV_FILE );
		return false;
	}

	for( i = 0; i < FAIL_INIT_NUM; i++ )
	{
		delay(10000000/2);
		if( ioctl( gen485_fd, PIN485_EN, 1 ) < 0 ) // 只能为1
		{
			if( gp_log_imp != NULL )
			gp_log_imp->log.post_log_msg( &gp_log_imp->log, 
									LOGGING_LEVEL_ERROR, 
									"ioctl(%d) enable send 485 pin(%s) failed!", 
									i,
									ENPIN485_DRV_FILE );
		}
		else
		{
			DEBUG_INFO("gen485fd ioctl  enable 0x%x-%d success\n", PIN485_EN, 1);
			return true;
		}
	}

	return false;
}

void en485_send_mod_cleanup( void )
{
	close( gen485_fd );
}

