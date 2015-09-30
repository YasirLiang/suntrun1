#include "inflight.h"

bool is_inflight_cmds_retried( inflight_plist station )
{
	return (station->host_tx.flags.retried >= 2); // 只重发送两次
}

void inflight_timer_state_avail( timetype timeout, inflight_plist target )
{
	inflight_timer_update( timeout, target);
}

void inflight_timer_start( timetype timeout, inflight_plist target )
{
	target->host_tx.timeout.running = true;
	target->host_tx.timeout.elapsed = false;	// 超时标志
	target->host_tx.timeout.count_time = timeout;
	target->host_tx.timeout.start_time = get_current_time();
}

void inflight_timer_stop( inflight_plist target )
{
	target->host_tx.timeout.elapsed = false;
	target->host_tx.timeout.running = false;
}

void inflight_timer_update( timetype timeout, inflight_plist target )
{
	target->host_tx.timeout.running = true;
	target->host_tx.timeout.elapsed = false;	// 超时标志
	target->host_tx.timeout.count_time = timeout;
	target->host_tx.timeout.start_time = get_current_time();
}

bool inflight_timer_timeout( inflight_plist target)
{
	if( target->host_tx.timeout.running && !target->host_tx.timeout.elapsed )
        {
        	uint32_t elapsed_ms;
            	timetype current_time = get_current_time();
            	elapsed_ms = (uint32_t)(current_time - target->host_tx.timeout.start_time);

           	if(elapsed_ms > target->host_tx.timeout.count_time)
            	{
                	target->host_tx.timeout.elapsed = true;
			return (target->host_tx.timeout.elapsed);
            	}
		else
			return false;		
	}

	return false;
}

void background_inflight_timer_start( timetype timeout, struct background_inflight_cmd *binflight_cmd )
{
	assert(binflight_cmd);
	
	binflight_cmd->timeouts.elapsed = false;
	binflight_cmd->timeouts.running = true;
	binflight_cmd->timeouts.count_time = timeout;
	binflight_cmd->timeouts.start_time = get_current_time();
}

bool background_inflight_timeout( struct background_inflight_cmd *binflight_cmd )
{
	if( binflight_cmd->timeouts.running && !binflight_cmd->timeouts.elapsed )
        {
        	uint32_t elapsed_ms;
            	timetype current_time = get_current_time();
            	elapsed_ms = (uint32_t)(current_time - binflight_cmd->timeouts.start_time );

           	if(elapsed_ms >  binflight_cmd->timeouts.count_time )
            	{
                	binflight_cmd->timeouts.elapsed = true;
			return ( binflight_cmd->timeouts.elapsed );
            	}
		else
			return false;		
	}
	
	return false;
}


