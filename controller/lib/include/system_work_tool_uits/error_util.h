/*
**Author:YasirLiang
**File:error_util.h
**Date Create:18-2-2016
**
*/

#ifndef __ERROR_UTIL_H__
#define __ERROR_UTIL_H__

#include "jdksavdecc_world.h"

#define SYS_ERR(err)  do{\
	perror(err);\
	exit(-1);\
}while(0)

#endif