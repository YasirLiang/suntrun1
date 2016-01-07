#include "circular_link_queue.h"

queue_node* queue_get( queue *root )
{
	assert( root );
	queue_node* node_get = root->front;

	if( !is_queue_empty( root ) )
	{
		root->front = root->front->next;
	}

	return node_get;
}

void queue_push( queue *root, queue_node *node )
{	
	assert( node );
	if( NULL != root->rear )
	{	
		root->rear->next = node; //新节点插入到队列尾
	}
	
	root->rear = node; // 队列尾指针指向队列尾节点
	if( NULL == root->front ) // 队列为空
	{
		root->front = node;
	}

	node->next = NULL;
}

bool is_queue_empty( queue* root )
{
	assert( root );
	return (( root->front == NULL ) ? true : false );
}

int get_queue_length( queue* root )
{
	assert( root );
	int length = 0;

	if ( is_queue_empty(root) )
	{
		length = 0;
	}
	else
	{
		queue_node* q_node = root->front;
		do
		{
			length++;
			q_node = q_node->next;
		}while ( q_node != NULL );	
	}

	return length;
}

void init_queue( queue *root )
{
	root->front = NULL;
	root->rear = NULL;
}

int queue_size( queue* root )
{
	assert( root );
	return get_queue_length( root );
}

