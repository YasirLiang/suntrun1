#ifndef __CONNECT_TIMEOUT_H__
#define __CONNECT_TIMEOUT_H__

#include "jdksavdecc_world.h"
#include "endstation_connection.h"
#include "host_time.h"

void connect_table_timer_state_avail( timetype timeout, connect_tbl_pdblist target );
void connect_table_timer_start( timetype timeout, connect_tbl_pdblist target );
void connect_table_timer_stop( connect_tbl_pdblist target );
void connect_table_timer_update( timetype timeout, connect_tbl_pdblist target );
bool connect_table_timer_timeout( connect_tbl_pdblist target);

#endif

