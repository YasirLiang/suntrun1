#ifndef __TIME_EVENT_H__
#define __TIME_EVENT_H__

#include "host_time.h"
#include "inflight.h"
#include "entity.h"
#include "adp_controller_machine.h"
#include "udp_server_controller_machine.h"
#include "acmp_controller_machine.h"
#include "acmp_controller_machine.h"
#include "udp_client_controller_machine.h"

extern void time_tick_event( solid_pdblist guard, inflight_plist inflight_guard );
extern volatile bool is_inflight_timeout;

#endif

