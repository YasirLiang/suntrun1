/*ע:���㷨�Ŀն���Ϊ����ͷΪ�գ�����β��Ϊ��,������Ϊ�������ͬʱ��
��βָ����Ϊ�գ���ֹ���Ϊ����ָ���ٱ�����!!!!!!,��17/29��,���ڻ�ȡ
����Ԫ��������Ϊ�գ�����β����������Ϊ�� 2016-1-13*/

#include "circular_link_queue.h"

queue_node* queue_get( queue *root )
{
	assert( root );
	queue_node* node_get = root->head;

	if( !is_queue_empty( root ) )
	{
		root->head = root->head->next;
	}
	else// �˴���ֹ����Ϊ��ʱ��β��Ϊ��!��ִ�жԶ��в�����ȷ��������Ϊ��ʱ������ִ�ж���Ϊ�յĲ�����ǰ����,�˴���ʡ��
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
	{// �˴���ֹ����Ϊ��ʱ��β��Ϊ��!��ִ�жԶ��в�����ȷ��������Ϊ��ʱ������ִ�ж���Ϊ�յĲ�����ǰ����,�˴���ʡ��
		if( root->trail != NULL )
			root->trail = NULL;
	}
	
	node->next = NULL;
	if( NULL != root->trail )
	{	
		root->trail->next = node; //�½ڵ���뵽����β
	}

	root->trail = node; // ����βָ��ָ�����β�ڵ�
	if( NULL == root->head ) // ����Ϊ��
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

