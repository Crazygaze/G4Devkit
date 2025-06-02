#include "detail/utils_internal.h"
#include "linkedlist.h"

void _linkedlist_addAfter_impl(LinkedListNode* node, LinkedListNode* newnode)
{
	newnode->previous = node;
	newnode->next = node->next;
	
	if (node->next) {
		node->next->previous = newnode;
	}
	node->next = newnode;
}

void _linkedlist_remove_impl(LinkedListNode* node)
{
	if (node->previous)
		node->previous->next = node->next;
	if (node->next)
		node->next->previous = node->previous;
	node->previous = NULL;
	node->next = NULL;
}

int _linkedlist_size_impl(LinkedListNode* node)
{
	int count=0;
	LINKEDLIST_FOREACH(node, LinkedListNode*, dummy) {
		count++;
	}
	return count;
}
