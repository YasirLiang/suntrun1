/*circular_link_queue.h
*
*
*/

#ifndef __CIRCULAR_LINK_QUEUE_H__
#define __CIRCULAR_LINK_QUEUE_H__

#include "jdksavdecc_world.h"

typedef struct queue_node // 队列节点
{
	struct queue_node *next;
}queue_node;

typedef struct queue // 队列
{
	queue_node *front;
	queue_node *rear;
}queue;

queue_node* queue_get( queue *root );
void queue_push( queue *root, queue_node *node );
bool is_queue_empty( queue* root );
void init_queue( queue *root );


#endif
