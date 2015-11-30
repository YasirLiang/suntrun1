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
		root->rear->next = node; //�½ڵ���뵽����β
	}
	
	root->rear = node; // ����βָ��ָ�����β�ڵ�
	if( NULL == root->front ) // ����Ϊ��
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

void init_queue( queue *root )
{
	root->front = NULL;
	root->rear = NULL;
}

