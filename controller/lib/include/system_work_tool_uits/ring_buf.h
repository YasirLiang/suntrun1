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
	bool empty;		// ���λ������Ŀձ�־
	uint8_t *pring_buf;	// ���λ�������ָ��
	uint16_t head;		// ���λ�������ͷ
	uint16_t trail;		// ���λ�������β
	uint32_t buf_size;	// ���λ������Ĵ�С
}tchar_ring_buf;

bool is_ring_buf_full( tchar_ring_buf *pchar_ring_buf );
bool char_ring_buf_save( tchar_ring_buf* pchar_ring_buf, uint8_t save_ch );
bool char_ring_buf_get( tchar_ring_buf *pchar_ring_buf, uint8_t *get_ch );

#endif