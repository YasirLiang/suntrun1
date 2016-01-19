/*
**ring_buf.h
**2016-1-19
**/

#ifndef __RING_BUF_H__
#define __RING_BUF_H__

#include "jdksavdecc_world.h"

typedef struct _type_ring_buf_node
{
	bool empty;
	uint16_t head;
	uint16_t trail;
	uint32_t buf_size;
}tring_buf_node;

typedef struct _type_ring_buf_char
{
	bool empty;		// 环形缓冲区的空标志
	uint8_t *pring_buf;	// 环形缓冲区的指针
	uint16_t head;		// 环形缓冲区的头
	uint16_t trail;		// 环形缓冲区的尾
	uint32_t buf_size;	// 环形缓冲区的大小
}tchar_ring_buf;

bool is_ring_buf_full( tchar_ring_buf *pchar_ring_buf );
bool char_ring_buf_save( tchar_ring_buf* pchar_ring_buf, uint8_t save_ch );
bool char_ring_buf_get( tchar_ring_buf *pchar_ring_buf, uint8_t *get_ch );

#endif