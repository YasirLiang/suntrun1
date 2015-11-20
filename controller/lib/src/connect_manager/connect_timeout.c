#include "connect_timeout.h"

void connect_table_timer_state_avail( timetype timeout, connect_tbl_pdblist target )
{
	connect_table_timer_update( timeout, target );
}

void connect_table_timer_start( timetype timeout, connect_tbl_pdblist target )
{
	target->connect_elem.spk_timeout.running = true;
	target->connect_elem.spk_timeout.elapsed = false;	// 超时标志
	target->connect_elem.spk_timeout.is_limit = true;        // 限时标志
	target->connect_elem.spk_timeout.count_time = timeout;
	target->connect_elem.spk_timeout.start_time = get_current_time();
}

void connect_table_timer_stop( connect_tbl_pdblist target )
{
	target->connect_elem.spk_timeout.running = false;
	target->connect_elem.spk_timeout.elapsed = false;	// 超时标志
	target->connect_elem.spk_timeout.is_limit = false;	  // 限时标志
}

void connect_table_timer_update( timetype timeout, connect_tbl_pdblist target )
{
	target->connect_elem.spk_timeout.running = true;
	target->connect_elem.spk_timeout.elapsed = false;	// 超时标志
	target->connect_elem.spk_timeout.is_limit = true;	  // 限时标志
	target->connect_elem.spk_timeout.count_time = timeout;
	target->connect_elem.spk_timeout.start_time = get_current_time();
}

bool connect_table_timer_timeout( connect_tbl_pdblist target)
{
	if( target->connect_elem.spk_timeout.running && target->connect_elem.spk_timeout.is_limit \
		&& !target->connect_elem.spk_timeout.elapsed )
        {
        	uint32_t elapsed_ms;
            	timetype current_time = get_current_time();
            	elapsed_ms = (uint32_t)( current_time - target->connect_elem.spk_timeout.start_time );

           	if(elapsed_ms > target->connect_elem.spk_timeout.count_time)
            	{
                	target->connect_elem.spk_timeout.elapsed = true;
            	}		
	}

	return target->connect_elem.spk_timeout.elapsed;
}



