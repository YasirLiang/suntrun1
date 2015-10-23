/*circular_link_queue.h
*
*
*/

#ifndef __LINK_QUEUE_H__
#define __LINK_QUEUE_H__

#include <stdio.h>

typedef struct queue_node // 队列节点
{
	struct queue_node *next;
}queue_node;

typedef struct queue // 队列
{
	queue_node *front;
	queue_node *rear;
}queue;

#endif
