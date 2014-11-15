#include "linkedlist.h"

void linkedlist_addAfter_impl(LinkedListNode* node, LinkedListNode* newnode)
{
	newnode->previous = node;
	newnode->next = node->next;
	
	if (node->next) {
		node->next->previous = newnode;
	}
	node->next = newnode;
}

void linkedlist_remove_impl(LinkedListNode* node)
{
	if (node->previous)
		node->previous->next = node->next;
	if (node->next)
		node->next->previous = node->previous;
	node->previous = NULL;
	node->next = NULL;
}

int linkedlist_size_impl(LinkedListNode* node)
{
	int count=0;
	LINKEDLIST_FOREACH(LinkedListNode, node, { count++; });
	return count;
}

#if TEST_LINKEDLIST

typedef struct Test
{
	struct Test* next;
	struct Test* previous;	
	int n;
} Test;

LINKEDLIST_VALIDATE_TYPE(Test)

Test t0= {NULL, NULL, 0};
Test t1= {NULL, NULL, 1};
Test t2= {NULL, NULL, 2};
Test t3= {NULL, NULL, 3};
Test t4= {NULL, NULL, 4};


void logLinkedList(Test* start, const char* tag)
{
	char indent[256];
	memset(indent,0,sizeof(indent));
	int i=0;
	
	utilshared_log(tag);
	LINKEDLIST_FOREACH(Test,start,
	{
		memset(indent, 32, i);
		i+=6;
		utilshared_log("%s%d<-%d->%d\n", indent, it->previous->n,it->n,it->next->n);	
	});
}

void test_linkedlist()
{
	linkedlist_addAfter(&t0, &t0);
	logLinkedList(&t0,"START\n");

	linkedlist_addAfter(&t0, &t1);
	logLinkedList(&t0,"START\n");

	linkedlist_addAfter(&t0, &t2);
	logLinkedList(&t0,"START\n");

	linkedlist_addAfter(&t1, &t3);
	logLinkedList(&t0,"START\n");

	linkedlist_addAfter(&t3, &t4);
	logLinkedList(&t0,"START\n");

	linkedlist_remove(&t0);
	logLinkedList(&t1,"Removed 0\n");
	
	linkedlist_remove(&t3);
	logLinkedList(&t1,"Removed 3\n");

	linkedlist_remove(&t4);
	logLinkedList(&t1,"Removed 4\n");

	linkedlist_remove(&t2);
	logLinkedList(&t1,"Removed 2\n");

}

#endif // TEST_LINKEDLIST


