#ifndef __TERMINAL_PRO_H__
#define __TERMINAL_PRO_H__

#include "jdksavdecc_world.h"
#include "conference.h"
#include "terminal_common.h"

#define ADDRESS_FILE "address.dat"	// �ն˵�ַ��Ϣ��ŵ��ļ�
//#define ADDRESS_FILE "profile/address.dat"	// �ն˵�ַ��Ϣ��ŵ��ļ�

#define SYSTEM_TMNL_MAX_NUM 512	// ϵͳ���ն���������
#define INIT_ADDRESS 0xffff

void init_terminal_proccess_system( void );

#endif

