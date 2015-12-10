#ifndef __HOST_CONTROLLER_DEBUG_H__
#define __HOST_CONTROLLER_DEBUG_H__

#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include "jdksavdecc_world.h"

#ifndef __DEBUG__
#define __DEBUG__		// ���Ժ꿪��
#ifdef __DEBUG__		// ������Ϣ��ӡ 
#include <assert.h>		// ������Բ��Գ���
#define DEBUG_LINE() fprintf( stdout,"\033[32m[%s:%s] line = %d\033[0m\r\n", __FILE__, __func__, __LINE__ )
#define DEBUG_ERR(fmt, args...) fprintf(  stdout,"\033[46;31m[%s:%s:%d]\033[0m "fmt" errno=%d\r\n", __FILE__, __func__, __LINE__, ##args, errno ); \
		perror(fmt);
#define DEBUG_INFO(fmt, args...) fprintf(  stdout,"\033[32m[%s:%s:%d]\033[0m "fmt"\r\n", __FILE__, __func__, __LINE__, ##args)
#define DEBUG_ONINFO(fmt, args...) fprintf(  stdout,"\033[34m"fmt"\033[0m\r\n", ##args)
#define DEBUG_MSGINFO(fmt, args...) fprintf(  stdout,"%s:%s:%d: "fmt"\r\n", __FILE__, __func__, __LINE__, ##args)
#define DEBUG_RECV( buf, n, arg )\
	int i = 0;\
	uint8_t *p = (uint8_t*)buf;\
	fprintf( stdout,"\033[32m[%s:%d]\033[0m %s:\t",  __func__, __LINE__, arg);\
	for(;i < n; i++)\
		fprintf(stdout, "%02x ", p[i] );\
	fprintf(stdout,"\n");
#define DEBUG_SEND( buf, n, arg )\
	int i = 0;\
	uint8_t *p = (uint8_t*)buf;\
	fprintf( stdout,"\033[32m[%s:%d]\033[0m %s:\t",  __func__, __LINE__, arg);\
	for(;i < n; i++)\
		fprintf(stdout, "%02x ", p[i] );\
	fprintf(stdout,"\n");

#define MSGINFO(fmt, args...) \
	fprintf(  stdout,""fmt"\n", ##args)
	
#define DABORT( cond ) if( !cond ){ fprintf( stdout, "Aborting at line %d in source file %s\n",__LINE__,__FILE__); abort(); }

#else				// ���а汾,����ӡ�κ���Ϣ
#define NDEBUG 		// ���Կ��أ�������������еĶ��Բ��ᱻִ��
#include <assert.h>		
#define DEBUG_LINE()
#define DEBUG_ERR(fmt, ...)
#define DEBUG_INFO(fmt, ...)
#define DEBUG_ONINFO(fmt, args...) 
#define DEBUG_RECV(buf, n, arg)
#define DEBUG_SEND( buf, n, arg )
#define DEBUG_MSGINFO(fmt, args...) 
#define MSGINFO(fmt, args...) fprintf(  stdout,""fmt"\n", ##args)
#define DABORT( cond )
#endif

#endif

#endif
