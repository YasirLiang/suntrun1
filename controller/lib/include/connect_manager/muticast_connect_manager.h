/*
**File:muticast_connect_manager.h
**Author:������
**ʱ��:2016��5��19��
**����:����ϵͳ����㲥���ӱ�
*/


/********************************************/
//�½�ϵͳ�㲥���ӻ��ƵĹ���ģ�飬
//����ԭ��muticast_connector.h�ļ��е�
//�������,
//�����������ݿ�Ĺ㲥���õ�
//������
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
