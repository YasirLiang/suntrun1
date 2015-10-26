#include "system.h"
#include "host_controller_debug.h"
#include "acmp_controller_machine.h"
#include "adp_controller_machine.h"
#include "aecp_controller_machine.h"
#include "terminal_pro.h"
#include "udp_client_controller_machine.h"

void init_system( void )
{
	endpoint_list = init_endpoint_dblist( &endpoint_list );
	assert( endpoint_list );
	command_send_guard = init_inflight_dblist( &command_send_guard );
	assert( command_send_guard );
	init_descptor_dblist( &descptor_guard );
	assert( descptor_guard );

	acmp_endstation_init( command_send_guard, endpoint_list, descptor_guard );
	aecp_controller_init(endpoint_list, descptor_guard,command_send_guard );
	DEBUG_ONINFO("[ SUCCESS: ENDPOINT AND INFLIGHT list init. ]");

	init_terminal_proccess_system();
	init_func_command_work_queue();
}

void set_system_information( struct fds net_fd, struct udp_context* p_udp_net )
{
	assert( p_udp_net );
	struct jdksavdecc_eui64 zero;
	bzero( &zero, sizeof(struct jdksavdecc_eui64));

	init_udp_client_controller_endstation( net_fd.udp_server_fd,  &p_udp_net->udp_srv.srvaddr );
	// found all endpoints
	adp_entity_avail( zero, JDKSAVDECC_ADP_MESSAGE_TYPE_ENTITY_DISCOVER);
}


