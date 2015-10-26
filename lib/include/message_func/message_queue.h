#ifndef __MESSAGE_QUEUE_H__
#define __MESSAGE_QUEUE_H__

#include "jdksavdecc_world.h"
#include "func_proccess.h"
#include "control_data.h"

int func_command_work_queue_messag_save( fcqueue_data_elem *p_node, fcwqueue *p_fcwq );
p_fcqueue_wnode func_command_work_queue_messag_get( fcwqueue *p_fcwq );


#endif
