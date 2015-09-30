#ifndef __CONFERENCE_DATA_PARSER_H__
#define __CONFERENCE_DATA_PARSER_H__

#include "jdksavdecc_frame.h"
#include "jdksavdecc_pdu.h"
#include "conference.h"
#include "conference_end_to_host.h"

int conference_parser_raw_data_get(struct jdksavdecc_frame *frame, uint8_t *conference_data,ssize_t *conference_data_len, struct endstation_to_host *phost, struct endstation_to_host_special *spephost);

#endif
