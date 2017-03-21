/*circular_link_queue.h
*
*
*/

#ifndef __CIRCULAR_LINK_QUEUE_H__
#define __CIRCULAR_LINK_QUEUE_H__

#include "jdksavdecc_world.h"
#include "host_controller_debug.h"

typedef struct queue_node {
	struct queue_node *next;
}queue_node;

typedef struct queue { /* circular queue */
	queue_node *head;
	queue_node *trail;
}queue;

/*$ \*/
extern queue_node* queue_get_value(queue *root);

/*$ \*/
extern queue_node* queue_get(queue *root);

/*$ \*/
extern void queue_push(queue *root, queue_node *node);

/*$ \*/
extern bool is_queue_empty(queue* root);

/*$ \*/
extern void init_queue(queue *root);

/*$ \*/
extern int get_queue_length(queue* root);

/*$ \*/
extern int queue_size(queue* root);

#endif /* __CIRCULAR_LINK_QUEUE_H__ */

