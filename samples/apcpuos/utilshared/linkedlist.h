/*!
* Code to manage linked lists
* To use, implement a struct has the first members as LinkedListNode.
* You can use the LINKEDLIST_VALIDATE_TYPE macro to validate a type.
*
* Also, all the functions assume the linked lists are circular. As in, there is
* no first or last node.
*/
#ifndef _utilshared_linkedlist_h_
#define _utilshared_linkedlist_h_


#include "utilsharedconfig.h"
#include "staticassert.h"

/*!
*/
typedef struct LinkedListNode
{
	struct LinkedListNode* next;
	struct LinkedListNode* previous;	
} LinkedListNode;

/*
 * Dont use these directly. Use the macros provided below
 */
void linkedlist_addAfter_impl(LinkedListNode* node, LinkedListNode* newnode);
void linkedlist_remove_impl(LinkedListNode* node);
int linkedlist_size_impl(LinkedListNode* node);

/*!
 * Macro to validate if a given struct matches the requirements to be used as a
 * linked list node
 * Note:
 * Don't put a ; after the macro, otherwise it won't compile in release
 * build. Use as:
 * LINKEDLIST_VALIDATE_TYPE(SomeType)
 * not
 * LINKEDLIST_VALIDATE_TYPE(SomeType);
 */
#define LINKEDLIST_VALIDATE_TYPE(Type) \
	STATIC_ASSERT( offsetof(Type,next)==offsetof(LinkedListNode,next) && offsetof(Type,previous)==offsetof(LinkedListNode,previous) )

#define LINKEDLIST_VALIDATE_NODE(node) \
	{ void* a = (node)->next; void* b = (node)->previous; }

#ifdef DEBUG
	// A cheap way to perform a simple check (to see if it exists a "next" field
	#define LINKEDLIST_VALIDATE_INPLACE(node) ((node)->next->previous)
#else
	#define LINKEDLIST_VALIDATE_INPLACE(node) (node)
#endif

/*!
* It adds some extra stuff to try to catch some invalid parameter types at
* compile time.
* With any optimization level, the compiler will compile out that generated code
*/
#define linkedlist_addAfter(node,newnode)\
	{ \
		LINKEDLIST_VALIDATE_NODE(node); \
		LINKEDLIST_VALIDATE_NODE(newnode); \
		linkedlist_addAfter_impl((LinkedListNode*)(node),(LinkedListNode*)(newnode)); \
	}
#define linkedlist_remove(node) \
	{ \
		LINKEDLIST_VALIDATE_NODE(node); \
		linkedlist_remove_impl((LinkedListNode*)(node)); \
	}

/*!
 * Counts how many items there are in the list
 */
#define linkedlist_size(startnode) \
	linkedlist_size_impl( LINKEDLIST_VALIDATE_INPLACE(startnode) )


/*!
* Iterates through the list
* \param Type
*	What type the items will be casted to
* \param startnode
*	Where to start iterating. The list is circular, so it will iterate until it
*	gets back to this item
* \param command
*	Command to apply to every item.
*	The item can be referenced to with "it" (as-in Iterator)
* Example
*
*	LINKEDLIST_FOREACH(MyItem,firstItem,
*	{
*		printf("%s\n", it->name);
*	});
*/
#define LINKEDLIST_FOREACH(Type, startnode, command) \
	{ \
		Type* it = (startnode); \
		do { \
			{ command; } \
			it = it->next; \
		} while(it!=(startnode)); \
	}

#endif

