/*circular_link_queue.h
*
*
*/

#ifndef __CIRCULAR_LINK_QUEUE_H__
#define __CIRCULAR_LINK_QUEUE_H__

#include "jdksavdecc_world.h"
#include "host_controller_debug.h"

#ifdef __DEBUG__
//#define __TEST_QUEUE__// ����ѭ�����п���
#endif

typedef struct queue_node // ���нڵ�
{
	struct queue_node *next;
}queue_node;

typedef struct queue // ����
{
	queue_node *head;
	queue_node *trail;
}queue;

queue_node* queue_get( queue *root );
void queue_push( queue *root, queue_node *node );
bool is_queue_empty( queue* root );
void init_queue( queue *root );
int get_queue_length( queue* root );
int queue_size( queue* root );


#endif
