/*
**ring_buf.c
**2016-1-19
**/

#include "ring_buf.h"
#include "host_controller_debug.h"

bool is_ring_buf_full( tchar_ring_buf *pchar_ring_buf )
{
	assert( pchar_ring_buf );
	if( pchar_ring_buf == NULL )
		return false;
	
	if( (!pchar_ring_buf->empty) && (pchar_ring_buf->head == pchar_ring_buf->trail) )
	{
		return true;
	}
	
	return false;
}

bool char_ring_buf_save( tchar_ring_buf *pchar_ring_buf, uint8_t save_ch )
{
	assert( pchar_ring_buf );
	if( pchar_ring_buf == NULL )
		return false;
	
	if( (pchar_ring_buf->empty) || (pchar_ring_buf->head != pchar_ring_buf->trail) )
	{
		pchar_ring_buf->pring_buf[pchar_ring_buf->trail++] = save_ch;
		pchar_ring_buf->trail %= pchar_ring_buf->buf_size;

		if( pchar_ring_buf->empty ) // save char ,buffer is not empty
			pchar_ring_buf->empty = false;

		return true;
	}

	return false;
}

bool char_ring_buf_get( tchar_ring_buf *pchar_ring_buf, uint8_t *get_ch )
{
	assert( pchar_ring_buf && get_ch );
	if( (pchar_ring_buf == NULL) || (get_ch == NULL) )
		return false;
	
	if( !pchar_ring_buf->empty )
	{
		*get_ch = pchar_ring_buf->pring_buf[pchar_ring_buf->head++];
		pchar_ring_buf->head %= pchar_ring_buf->buf_size;

		if( pchar_ring_buf->head == pchar_ring_buf->trail )
			pchar_ring_buf->empty = true;

		return true;
	}

	return false;
}

