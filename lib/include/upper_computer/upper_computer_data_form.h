#ifndef __UPPER_COMPUTER_DATA_FORM_H__
#define __UPPER_COMPUTER_DATA_FORM_H__

#include "jdksavdecc_world.h"
#include "upper_computer.h"
#include "jdksavdecc_util.h"

extern void set_upper_cmpt_check( struct host_upper_cmpt *p );
extern void host_upper_cmpt_common_header_write( const struct host_upper_cmpt_common* common_header, void* base, ssize_t pos );
extern void host_upper_cmpt_data_write( const uint8_t *pdata, void* base, ssize_t pos, const uint16_t data_len );
extern void host_upper_cmpt_end_crc_write( const uint8_t crc, void* base, ssize_t pos );
extern size_t  conference_host_to_upper_computer_frame_write( void *base, struct host_upper_cmpt *p, const uint16_t data_len, size_t pos, size_t len );
extern int conference_host_to_upper_computer_form_msg( struct host_upper_cmpt_frame *frame,  struct host_upper_cmpt *phost );


#endif

