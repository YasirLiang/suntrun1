/*circular_link_queue.h
*
*
*/

#ifndef __LINK_QUEUE_H__
#define __LINK_QUEUE_H__

#include <stdio.h>

typedef struct queue_node // ���нڵ�
{
	struct queue_node *next;
}queue_node;

typedef struct queue // ����
{
	queue_node *front;
	queue_node *rear;
}queue;

#endif
