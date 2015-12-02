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

