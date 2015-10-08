#ifndef __TERMINAL_COMMAND_H__
#define __TERMINAL_COMMAND_H__

#include "conference.h"

#define COMMAND_TMN_MASK	 	0x1F	//��������,���������ڵ�5λ
#define COMMAND_FROM_TMN	 	0x80	//�����
#define COMMAND_TMN_REPLY	 	0x40	//��Ӧ���ı�־
#define COMMAND_TMN_CHAIRMAN	0x20	//��ϯ��Ԫ��ע 

#define TMN_ADDR_MASK   0x0FFF
#define BROADCAST_FLAG  0x8000
#define BRCT_RANGE_MARK 0xFE00
#define BRDCST_ALL        0x8000
#define BRDCST_MEM      0x8200
#define BRDCST_VIP      0x8400
#define BRDCST_CHM      0x8800
#define BRDCST_EXE      0x9000

#endif

