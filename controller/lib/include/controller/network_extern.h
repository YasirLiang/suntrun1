/*File:network_extern.h
  *Author:梁永富
  *Build data:2016-07-07
  *descriptor:外部网络总的抽象文件
  */

#ifndef __NETWORK_EXTERN_H__
#define __NETWORK_EXTERN_H__

struct network_extern
{
	int (*network_init)(void**);
	int (*network_send)(void*, int, const void *);
	int (*network_recv)(void*, int, const void*);
	int (*network_cleanup)(void**);
};

#endif
