/*注:此算法的空队列为队列头为空，而队尾不为空,若对列为空则必须同时把
队尾指针置为空，防止其成为垂悬指针再被引用!!!!!!,如15 27行,且在获取
队列元素若队列为空，队列尾应该马上置为空 2016-1-13*/

#include "circular_link_queue.h"

queue_node* queue_get( queue *root )
{
	assert( root );
	queue_node* node_get = root->head;

	if( !is_queue_empty( root ) )
	{
		root->head = root->head->next;
	}
	else
	{
		if( root->trail != NULL )
			root->trail = NULL;
	}

	return node_get;
}

void queue_push( queue *root, queue_node *node )
{
	assert( node );
	if( is_queue_empty( root ) )
	{
		if( root->trail != NULL )
			root->trail = NULL;
	}
	
	node->next = NULL;
	if( NULL != root->trail )
	{	
		root->trail->next = node; //新节点插入到队列尾
	}

	root->trail = node; // 队列尾指针指向队列尾节点
	if( NULL == root->head ) // 队列为空
	{
		root->head = node;
	}
}

bool is_queue_empty( queue* root )
{
	assert( root );
	return (( root->head == NULL ) ? true : false );
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
		queue_node* q_node = root->head;
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
	root->head = NULL;
	root->trail = NULL;
}

int queue_size( queue* root )
{
	assert( root );
	return get_queue_length( root );
}

