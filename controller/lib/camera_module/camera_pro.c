/*
**camera_pro.c
**2015-12-8
**
*/

#include "camera_pro.h"
#include "host_controller_debug.h"
#include "profile_system.h"
#include "pelco_d.h"
#include "terminal_common.h"
#include "terminal_pro.h"
#include "camera_output.h"
#include "terminal_system.h"
#include "camera_common.h"
#include "control_matrix_pro.h"

preset_point_format gpresetcmr_list[PRESET_NUM_MAX];	// 预置点列表
preset_point_format gcurpresetcmr;					// 当前预置点
uint16_t gcamer_presetdndex;
FILE *gpreset_fd = NULL;		// 全局预置点信息文件描述符,系统第一次运行以wb+方式打开
uint16_t gcamerpreset_index = 0;
uint8_t gcamera_levelspeed = 25;
uint8_t gcamera_vertspeed = 25;
bool grctrl_align_enable;//对码显示页

uint8_t gfull_view_index[2]; // 全景切换索引
uint8_t gscene_out;

get_cmrpreset_pro ggetcmrpreset_pro;

uint8_t gmatrix_output[ MATRIX_OUTPUT_NUM ];

void preset_camera_list_info( void )
{
#ifdef __CAMERA_DEBUG__
		int i = 0;
		printf( "\n---------------camera preset file list init success->List Info:[(camera num)--(preset_point_num)--(addr)]------\n\t");
		for( i = 0; i < sizeof(gpresetcmr_list)/sizeof(preset_point_format); i++ )
		{
			if( gpresetcmr_list[i].tmnl_addr != 0xffff )
				printf( "[%d--%d--0x%04x] ", gpresetcmr_list[i].camera_num,\
					gpresetcmr_list[i].preset_point_num, gpresetcmr_list[i].tmnl_addr );
		}

		printf("\n");
#endif
}


/*=============================
*开始摄像头命令处理函数
*==============================*/
/**********************************
**Writer:YasirLiang
**Date:2015-12-8
**Name:camera_preset_save
**Param:
**	cmd:0
**	data:NULL
**	data_len:0
**Return Value: -1:Error
**State:在当前预置参数gcurpresetcmr中保存终
**	端的预置位
************************************/
int camera_preset_save( uint16_t cmd, void *data, uint32_t data_len )
{
	uint16_t index = 0;
	uint8_t preset = 0;

	DEBUG_INFO( "=====gcurpresetcmr.tmnl_addr = 0x%04x", gcurpresetcmr.tmnl_addr );
	if( 0xffff == gcurpresetcmr.tmnl_addr )
	{
		return -1;
	}

	if( camera_preset_list_exit_addr( gcurpresetcmr.tmnl_addr, &index)) 
	{
		gpresetcmr_list[index].preset_point_num = 0xff;
		if( !camera_prese_num(gcurpresetcmr.camera_num, &preset ))
		{
			return -1;
		}

		gcurpresetcmr.preset_point_num = preset;
	}
	else if( camera_preset_list_exit_addr(0xffff, &index) )
	{
		gpresetcmr_list[index].preset_point_num = 0xff;
		gpresetcmr_list[index].camera_num = 0xff;
		if( !camera_prese_num(gcurpresetcmr.camera_num, &preset) )
		{
			return -1;
		}

		gcurpresetcmr.preset_point_num = preset;
	}
	else
	{
		DEBUG_INFO( "not find preset addr!" );
		return -1;
	}
	
	gpresetcmr_list[index].camera_num = gcurpresetcmr.camera_num;
	gpresetcmr_list[index].preset_point_num = gcurpresetcmr.preset_point_num;
	gpresetcmr_list[index].tmnl_addr = gcurpresetcmr.tmnl_addr;
	DEBUG_INFO( "index in gpresetcmr_list = %d, [0x%04x-%d-%d]", index, gpresetcmr_list[index].tmnl_addr, gpresetcmr_list[index].camera_num, gpresetcmr_list[index].preset_point_num );

	camera_pro_control( gcurpresetcmr.camera_num, CAMERA_CTRL_PRESET_SET, 0,\
		gcurpresetcmr.preset_point_num );

	camera_profile_format cmr_file;
	int write_byte = 0;
	
	write_byte = sizeof(preset_point_format)*PRESET_NUM_MAX;
	memcpy( cmr_file.cmr_preset_list, gpresetcmr_list, write_byte );
	camera_profile_fill_check( &cmr_file, PRESET_NUM_MAX );
	if( -1 != Fseek(gpreset_fd, 0, SEEK_SET) )
	{
		camera_profile_write( gpreset_fd, &cmr_file );
		Fflush( gpreset_fd );	
	}
#ifdef __CAMERA_DEBUG__
		preset_camera_list_info();
#endif

	if((gcurpresetcmr.tmnl_addr != FULL_VIEW_ADDR) && (gcurpresetcmr.tmnl_addr != BACKUP_FULL_VIEW_ADDR))
	{//非全景定位保存预置后需要熄灭相应终端的指示灯
		tmnl_pdblist close_node = found_terminal_dblist_node_by_addr( gcurpresetcmr.tmnl_addr ); // 注: ====>>>这里也操作了终端链表<<<<<<<<=====
#ifdef ENABLE_CONNECT_TABLE
		connect_table_tarker_disconnect( close_node->tmnl_dev.entity_id, \
			close_node, true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
		trans_model_unit_disconnect( close_node->tmnl_dev.entity_id );
#endif
	}

	return 0;
}

int camera_preset_addr_select( uint16_t cmd, void *data, uint32_t data_len )
{
	uint16_t addr;

	if( (NULL==data) && (data_len!=sizeof(uint16_t)) )
	{
		return false;
	}
	
	addr = *(uint16_t *)data;
	//DEBUG_INFO( "preset select addr = %04x, gcurpresetcmr.tmnl_addr= %04x", addr, gcurpresetcmr.tmnl_addr );
	if(addr>FULL_VIEW_ADDR)
	{
		return false;
	}
	
	if( (gcurpresetcmr.tmnl_addr < BACKUP_FULL_VIEW_ADDR) \
		&& (gcurpresetcmr.tmnl_addr != addr) )
	{//改变需定位的终端，熄灭原终端的相应指示灯
		if( gcurpresetcmr.tmnl_addr != 0xFFFF )
		{
			tmnl_pdblist close_node = found_terminal_dblist_node_by_addr( gcurpresetcmr.tmnl_addr ); // 注: ====>>>这里也操作了终端链表<<<<<<<<=====
			if( close_node != NULL )
			{
#ifdef ENABLE_CONNECT_TABLE
				connect_table_tarker_disconnect( close_node->tmnl_dev.entity_id, close_node,\
					true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
				trans_model_unit_disconnect( close_node->tmnl_dev.entity_id );
#endif
			}
		}
	}
	
	if( addr < BACKUP_FULL_VIEW_ADDR )//某个终端的定位
	{
		if( addr != 0xFFFF )
		{
			tmnl_pdblist peset_node = found_terminal_dblist_node_by_addr( addr ); // 注: ====>>>这里也操作了终端链表<<<<<<<<=====
			if( peset_node != NULL )
			{
				 terminal_mic_state_set( MIC_PRESET_BIT_STATUS, peset_node->tmnl_dev.address.addr, \
					peset_node->tmnl_dev.entity_id, true, peset_node );
			}	
		}
	}
	
	gcurpresetcmr.tmnl_addr = addr;

	return true;
}

int camera_select_num( uint16_t cmd, void *data, uint32_t data_len )// 此函数未完成接口实现2015-12-09
{
	uint8_t curcmr;
	
	if( data_len != sizeof(uint8_t))
	{
		return -1;
	}
	
	curcmr = *((uint8_t*)data);
	gcurpresetcmr.camera_num = curcmr;
	DEBUG_INFO( "curcamera Num = %d", gcurpresetcmr.camera_num );

	// 切换摄像头......
	gmatrix_output[0] = CAMERA_OUT_TRACK_VIEW;
	control_matrix_input_output_switch( MATRIX_AV_SWITCH, gcurpresetcmr.camera_num, gmatrix_output, 1 );
	//camera_output_switch( gcurpresetcmr.camera_num, CAMERA_OUT_TRACK_VIEW, true );
	//CmrSwCmd(gCameraPresetCur.CameraNum-1,CMR_OUT_FOR_TRACK-1);
	
	return 0;
}

int camera_control_rightleft( uint16_t cmd, void *data, uint32_t data_len )
{
	uint8_t cond = 0;
	
	if( data_len != sizeof(uint8_t))
	{
		return -1;
	}

	cond = *((uint8_t*)data);
	camera_pro_control( gcurpresetcmr.camera_num, cond?CAMERA_CTRL_RIGHT:CAMERA_CTRL_LEFT,\
		gcamera_levelspeed, gcamera_vertspeed );
	
	return 0;
}


int camera_control_updown( uint16_t cmd, void *data, uint32_t data_len )
{
	uint8_t cond = 0;
	
	if( data_len != sizeof(uint8_t))
	{
		return -1;
	}

	cond = *((uint8_t*)data);
	camera_pro_control( gcurpresetcmr.camera_num, cond?CAMERA_CTRL_UP:CAMERA_CTRL_DOWN,\
		gcamera_levelspeed, gcamera_vertspeed );

	return 0;
	
}

int camera_control_fouce( uint16_t cmd, void *data, uint32_t data_len )
{
	uint8_t cond = 0;// 控制条件
	
	if( data_len != sizeof(uint8_t))
	{
		return -1;
	}

	cond = *((uint8_t*)data);
	camera_pro_control( gcurpresetcmr.camera_num, cond?CAMERA_CTRL_FOCUCS_NEAR:CAMERA_CTRL_FOCUCS_FAR,\
		gcamera_levelspeed, gcamera_vertspeed );

	return 0;
}

int camera_control_zoom( uint16_t cmd, void *data, uint32_t data_len )
{
	uint8_t cond = 0;// 控制条件
	
	if( data_len != sizeof(uint8_t))
	{
		return -1;
	}

	cond = *((uint8_t*)data);
	camera_pro_control( gcurpresetcmr.camera_num, cond?CAMERA_CTRL_ZOOM_TELE:CAMERA_CTRL_ZOOM_WIDE,\
		gcamera_levelspeed, gcamera_vertspeed );

	return 0;
}

int camera_control_iris( uint16_t cmd, void *data, uint32_t data_len )
{
	uint8_t cond = 0;// 控制条件
	
	if( data_len != sizeof(uint8_t))
	{
		return -1;
	}

	cond = *((uint8_t*)data);
	camera_pro_control( gcurpresetcmr.camera_num, cond?CAMERA_CTRL_IRIS_OPEN:CAMERA_CTRL_IRIS_CLOSE,\
		gcamera_levelspeed, gcamera_vertspeed );

	return 0;
}

int camera_control_align( uint16_t cmd, void *data, uint32_t data_len )
{
	uint8_t cond = 0;// 控制条件
	
	if( data_len != sizeof(uint8_t))
	{
		return -1;
	}

	cond = *((uint8_t*)data);
	if(cond)
	{
		grctrl_align_enable = true;
	}
	else
	{
		grctrl_align_enable = true;
	}
	
	return 0;
}

int camera_control_full_view( uint16_t cmd, void *data, uint32_t data_len )
{
	
	return 0;
}

/*预置点设置*/ 
int camera_preset_set( uint16_t cmd, void *data, uint32_t data_len )
{
	int i = 0;
	
	for( i = 0; i < PRESET_NUM_MAX; i++ )
	{
		gpresetcmr_list[i].camera_num = MUX_NUM_CAMERA;
		gpresetcmr_list[i].preset_point_num = MAX_PRESET_ONE_CAMERA;
		gpresetcmr_list[i].tmnl_addr = 0xffff;
	}

	return 0;
}

// 获得摄像预置点的设置，用于摄像跟踪
int camera_get_preset( uint16_t cmd, void *data, uint32_t data_len ) // 此函数未完成2015-12-09
{
	uint16_t addr;
	uint16_t i;
	uint16_t index;
	uint8_t scene_switch = 0;

	addr = *(uint16_t *)data;
	for( i = 0; i < PRESET_NUM_MAX; i++ )
	{
		if( addr == gpresetcmr_list[i].tmnl_addr )
		{
			if( (addr == FULL_VIEW_ADDR) && (gscene_out < 2) )
			{
				index = gfull_view_index[gscene_out];
			}
			else
			{
				index=i;
			}
			
			if((gfull_view_index[0]!=PRESET_NUM_MAX)&&(gfull_view_index[1]!=PRESET_NUM_MAX)
				&&( addr != FULL_VIEW_ADDR)&&\
				(gpresetcmr_list[i].camera_num == gpresetcmr_list[gfull_view_index[gscene_out]].camera_num))
			{
				gscene_out++;
				gscene_out%=2;
				camera_pro_control(gpresetcmr_list[gfull_view_index[gscene_out]].camera_num,\
					CAMERA_CTRL_PRESET_CALL,0,(gpresetcmr_list+gfull_view_index[gscene_out])->preset_point_num); 
				
				gmatrix_output[0] = CAMERA_OUT_FULL_VIEW;
				control_matrix_input_output_switch( MATRIX_AV_SWITCH, \
					gpresetcmr_list[gfull_view_index[gscene_out]].camera_num, gmatrix_output, 1 );
				//camera_output_switch(gpresetcmr_list[gfull_view_index[gscene_out]].camera_num,CAMERA_OUT_FULL_VIEW,1); 
				//CmrSwCmd((gpresetcmr_list+gfull_view_index[gscene_out])->CameraNum-1,CAMERA_OUT_FULL_VIEW-1);
				scene_switch = 1;
			}
			
			if(scene_switch)
			{
				ggetcmrpreset_pro.camera_num = gpresetcmr_list[index].camera_num;
				ggetcmrpreset_pro.preset_num = gpresetcmr_list[index].preset_point_num;
				ggetcmrpreset_pro.out = CAMERA_OUT_TRACK_VIEW;
				ggetcmrpreset_pro.busy_flag = 1;
			}
			else
			{
				camera_pro_control( gpresetcmr_list[index].camera_num,CAMERA_CTRL_PRESET_CALL,\
					0, gpresetcmr_list[index].preset_point_num );
				
				gmatrix_output[0] = CAMERA_OUT_TRACK_VIEW;
				control_matrix_input_output_switch( MATRIX_AV_SWITCH, \
					(gpresetcmr_list+index)->camera_num, gmatrix_output, 1 );
				//camera_output_switch((gpresetcmr_list+index)->camera_num,CAMERA_OUT_TRACK_VIEW,1);
				//CmrSwCmd((gpresetcmr_list+index)->CameraNum-1,CAMERA_OUT_TRACK_VIEW-1);
			}

			gcamer_presetdndex = index;
			gcurpresetcmr.camera_num = gpresetcmr_list[index].camera_num;
			break;
		}
	}

	return 0;
}
/*=============================
*结束摄像头命令处理函数
*==============================*/

/*=============================
*开始摄像头流程处理
*==============================*/

void camera_save_Cmrpreset_direct( void )
{
	if( gcamerpreset_index < PRESET_NUM_MAX )
	{
		camera_pro_control(gpresetcmr_list[gcamerpreset_index].camera_num, \
			CAMERA_CTRL_PRESET_SET, 0, gpresetcmr_list[gcamerpreset_index].camera_num );
	}
}

bool camera_prese_num( uint8_t camera_num, uint8_t *index )// 每个摄像头都有其的可用的预置点
{
	bool ret = false;
 	uint8_t preset[MAX_PRESET_ONE_CAMERA];
  	uint16_t i;
	
 	memset( preset, 0, MAX_PRESET_ONE_CAMERA );
	for(i=0; i < PRESET_NUM_MAX; i++)
	{
		if((((gpresetcmr_list+i)->tmnl_addr) <= FULL_VIEW_ADDR) &&\
			(((gpresetcmr_list+i)->camera_num) == camera_num))
		{
			if(((gpresetcmr_list+i)->preset_point_num) < MAX_PRESET_ONE_CAMERA)
			{
				preset[(gpresetcmr_list+i)->preset_point_num] = 1;
			}
		}
	}
	
	*index = MAX_PRESET_ONE_CAMERA;
	for( i = 0; i < PRESET_NUM_MAX; i++ )
	{
		if( 0 == preset[i] )
		{
			*index = i;
			ret = true;
			break;
		}
	}
	
	return ret;
}

// 在预置位列表中找到终端的偏移
bool camera_preset_list_exit_addr( uint16_t addr, uint16_t *p_index )
{
	int i = 0;
	bool ret = false;

	assert( p_index );	
	for( i = 0; i < PRESET_NUM_MAX; i++ )
	{
		if( addr == gpresetcmr_list[i].tmnl_addr )
		{
			*p_index = i;
			ret = true;
			break;
		}
	}

	return ret;
}

void camera_pro_enter_preset( void )
{
	set_terminal_system_state( CAMERA_PRESET, true );
}

void camera_pro_esc_preset( void )
{
	set_terminal_system_state( CAMERA_PRESET, false );	
}

// 设置摄像头跟踪功能
void camera_pro_lock_flags( uint8_t option )
{
	if( option == CAMARA_LOCK )
	{
		gset_sys.camara_track = CAMARA_LOCK;
	}
	else if( option == CAMARA_UNLOCK )
	{
		gset_sys.camara_track = CAMARA_UNLOCK;
	}
}

// control the camera by comand and stop the camera
int camera_pro_control( uint8_t  cmr_addr, uint16_t d_cmd, uint8_t speed_lv, uint8_t speed_vertical )
{
	camera_form_can_send( cmr_addr, d_cmd, speed_lv, speed_vertical ); // control the camera
	return (camera_form_can_send( cmr_addr, 0, 0, 0 )); // stop the camera
	return 0;
}

/*初始化预置点文件(系统第一次启动)与初始化预置点列表*/
int camera_pro_preset_file_list_init( void )
{
	FILE *fd = NULL;
	camera_profile_format cmr_file;
	int ret = 0;
	
	fd = camera_profile_open( PRESET_SYSTEM_FILE, "rb+" );
	if( NULL == fd ) // 系统第一次运行文件不存在
	{
		DEBUG_INFO( "open %s Err: not exit!Will create it for the system first runtime!", PRESET_SYSTEM_FILE );
		fd = camera_profile_open( PRESET_SYSTEM_FILE, "wb+" );
		if( NULL == fd )
		{
			DEBUG_ERR( "camera profile open" );
			return -1;
		}
		else
		{	
			int write_byte = sizeof(preset_point_format)*PRESET_NUM_MAX;
			
			camera_preset_set( 0, NULL, 0 );
			gcurpresetcmr.camera_num = 1;
			gcurpresetcmr.preset_point_num = 0;
			gcurpresetcmr.tmnl_addr = 0xffff;
			memcpy( cmr_file.cmr_preset_list, gpresetcmr_list, write_byte );
			if( camera_profile_fill_check( &cmr_file, PRESET_NUM_MAX ) == -1 )
			{
				DEBUG_INFO( "write camera preset information Err !" );
				ret = -1;
			}
			else
			{
				if( camera_profile_write( fd, &cmr_file ) != 1 )
				{
					DEBUG_ERR( "writing camera preset information for the first is wrong!" );
					ret = -1;
				}
			}
		}
	}
	else
	{
		if( camera_profile_read( fd, &cmr_file ) != 1 )
		{
			DEBUG_INFO( "read camera preset information Err !" );
			ret =-1;
		}
		else
		{
			if( !camera_profile_check_right( &cmr_file, PRESET_NUM_MAX) )
			{
				DEBUG_INFO( "read camera preset information for wrong check: need to check file %s is right", PRESET_SYSTEM_FILE );
				ret = -1;
			}
			else
			{
				memcpy( gpresetcmr_list, cmr_file.cmr_preset_list, sizeof(gpresetcmr_list));
				ret = 0;
			}	
		}
	}

	gpreset_fd = fd;

	return ret;
}

void camera_pro_init( void ) // 必须在系统配置参数读取完成才能调用
{
	if( camera_pro_preset_file_list_init() == 0 )
	{
#ifdef __CAMERA_DEBUG__
		preset_camera_list_info();
#endif
	}
	gcurpresetcmr.camera_num = gset_sys.current_cmr;
	gcurpresetcmr.preset_point_num = 0;
	gcurpresetcmr.tmnl_addr = 0xffff;
}


/*清除摄像头系统信息, 这里把内存的对应的所有数据都保存到了文件中*/ 
void camera_pro_system_close( void )
{
	/*写入配置文件*/
	camera_profile_format cmr_file;
	int write_byte = 0;
	
	write_byte = sizeof(preset_point_format)*PRESET_NUM_MAX;
	memcpy( cmr_file.cmr_preset_list, gpresetcmr_list, write_byte );
	camera_profile_fill_check( &cmr_file, PRESET_NUM_MAX );
	// 从头写入
	if( -1 != Fseek( gpreset_fd, 0, SEEK_SET))
	{
		camera_profile_write( gpreset_fd, &cmr_file );
		Fflush( gpreset_fd );
	}

	/*2016-1-26不能正常关闭*/ 
	/*
	if( gpreset_fd != NULL )
	{
		DEBUG_INFO( "============>camera system close Success!<=======" );
		camera_profile_close( gpreset_fd );
		gpreset_fd = NULL;
	}

	DEBUG_INFO( "============>camera system close Success!<=======" );*/
}

/*=============================
*结束摄像头流程处理
*==============================*/

