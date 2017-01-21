/*
**File:muticast_connect_manager.h
**Author:梁永富
**时间:2016年5月19日
**功能:管理系统中央广播连接表
*/


/********************************************/
//新建系统广播连接机制的管理模块，
//代替原来muticast_connector.h文件中的
//管理机制,
//并新增了数据库的广播配置的
//管理功能
/********************************************/

#ifndef __MUTICAST_CONNECT_MANAGER_H__
#define __MUTICAST_CONNECT_MANAGER_H__

#include "jdksavdecc_world.h"
#include "global.h"

struct list_head;

extern int muticast_connect_manger_timeout_event_image( void );
extern int muticast_connect_manger_chdefault_outmuticastor( struct list_head *p_muti, struct list_head *p_muit_out );
extern int muticast_connect_manger_database_update( void* p_muti_param );
extern bool muticast_muticast_connect_manger_get_discut_self_flag( void );
extern void muticast_muticast_connect_manger_pro_stop(void);
extern void muticast_muticast_connect_manger_init( void );

#endif
