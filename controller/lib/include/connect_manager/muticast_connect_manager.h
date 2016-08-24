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

#include "list.h"
#include "system_database.h"
#include "host_timer.h"

enum _enum_muticast_pro_state
{
	MUTICAST_PRO_PRIMED,
	MUTICAST_PRO_HANDING,
	MUTICAST_PRO_IDLE,
};

enum _enum_change_muticast_state
{
	MUTICAST_CHANGE_BEGIN,
	MUTICAST_CHANGE_HANDING,
	MUTICAST_CHANGE_FINISH
};

typedef struct _type_muticast_manager
{
	enum _enum_change_muticast_state mm_cha_state;
	enum _enum_muticast_pro_state mm_pro_state;
	bool running;
	bool muticast_exist;
	Tstr_sysmuti_param mm_sys_flags;
	host_timer mm_errlog_timer;
	struct list_head * ptr_curcfc_recv_model;// tconference_recieve_model
	struct list_head * ptr_muticastor;// T_pccuTModel
	struct list_head *ptr_muticastor_output;// ����㲥�ߵ����ͨ�� ����ΪTOutChannel
}Tstr_MMPro;

extern int muticast_connect_manger_timeout_event_image( void );
extern int muticast_connect_manger_chdefault_outmuticastor( struct list_head *p_muti, struct list_head *p_muit_out );
extern int muticast_connect_manger_database_update( void* p_muti_param );
extern bool muticast_muticast_connect_manger_get_discut_self_flag( void );
extern void muticast_muticast_connect_manger_pro_stop(void);
extern void muticast_muticast_connect_manger_init( void );

#endif
