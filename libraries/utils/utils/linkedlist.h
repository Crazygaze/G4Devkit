/*!
 * Manages linked lists
 * To use, implement a struct that has the first members as LinkedListNode.
 * You can use the LINKEDLIST_VALIDATE_TYPE macro to validate a type.
 *
 * Also, all the functions assume the linked lists are circular. As in, there is no first or last
 * node.
 */
#ifndef _utils_linkedlist_h_
#define _utils_linkedlist_h_

#include <stddef.h>
#include "staticassert.h"

/*!
 * This defines the requirements for element type.
 * The type should have two pointers (named `next` and `previous`) as the first two fields.
 *
 * E.g:
 *
 * ```
 *	typedef struct Item
 *	{
 *		struct Item* next;
 *		struct Item* previous;
 *		// Item's fields
 *		int a;
 *		int b;
 *	} Item;
 *	// Validate that it matches requirements
 *	LINKEDLIST_VALIDATE_TYPE(Item)
 * ```
 */

typedef struct LinkedListNode
{
	struct LinkedListNode* next;
	struct LinkedListNode* previous;	
} LinkedListNode;

/*!
 * Macro to validate if a given struct matches the requirements to be used as a
 * linked list node
 *
 * Note:
 * Don't put a ; after the macro, otherwise it won't compile in release
 * build. Use as:
 * LINKEDLIST_VALIDATE_TYPE(SomeType)
 * not
 * LINKEDLIST_VALIDATE_TYPE(SomeType);
 */
#define LINKEDLIST_VALIDATE_TYPE(Type) \
	STATIC_ASSERT( offsetof(Type,next)==offsetof(LinkedListNode,next) && offsetof(Type,previous)==offsetof(LinkedListNode,previous) )

// Set this to 1 if you wish to have type checking (only for Debug builds)
// Type checking lets you catch any possible places where you might be passing
// wrong types to the macros. The macro accepts wrongs types, because it ends
// up casting everything to `LinkedListNode*`.
// The checking is disabled by default, because it generates a bunch of instructions
#define LINKEDLIST_ENABLE_TYPE_CHECKING 0

// This macro is only used internally, as kinda of a static assert that checks
// if the pointers passed to the functions have a `next` and `previous` fields.
// This is handy, because we are using casts to `LinkedListNode*`, and so it
// would be too easy for the developer to pass something completely wrong.
// By checking for the existence of `next` and `previous`, we eliminate most
// of those problems.
#if defined(DEBUG) && LINKEDLIST_ENABLE_TYPE_CHECKING
	#define _LINKEDLIST_VALIDATE_INPLACE(node) \
		(LinkedListNode*)(&(node)->previous - (&(node)->previous - &(node)->next))
#else
	#define _LINKEDLIST_VALIDATE_INPLACE(node) (node)
#endif

/*!
 * Adds `newnode` after `node`
 */
#define linkedlist_addAfter(node,newnode)                        \
	_linkedlist_addAfter_impl(                                   \
		(LinkedListNode*)_LINKEDLIST_VALIDATE_INPLACE(node),     \
		(LinkedListNode*)_LINKEDLIST_VALIDATE_INPLACE(newnode))
	
/*!
 * Removes the specified node from the list
 */
#define linkedlist_remove(node) \
	_linkedlist_remove_impl((LinkedListNode*)_LINKEDLIST_VALIDATE_INPLACE(node))

/*!
 * Counts how many items there are in the list.
 * Note that this needs to transverse the entire list.
 *
 * \param startnode
 *	Node to start counting from. Since the list is circular, it doesn't really
 *	matter which node this is. Any node in the list will do.
 */
#define linkedlist_size(startnode) \
	_linkedlist_size_impl((LinkedListNode*)_LINKEDLIST_VALIDATE_INPLACE(startnode))

/*!
* Iterates over each element of the list, the same way as a for loop
*
* \param Type Element type
* \param startnode
*	Where to start iterating. The list is circular, so it will iterate until it
*	gets back to this item
* \param itVar variable to use in the loop to acces the element 
*
* Example for a linked list of `Node` items, where `Node` has a `name` field
*
*   Node* root = <something>;
*
*	LINKEDLIST_FOREACH(root, Node*, it) {
*		printf("%s\n", it->name);
*	};
*/
#define LINKEDLIST_FOREACH(startnode, Type, itVar) \
	for(Type _begin = startnode, *itVar = (_begin && _begin->next) ? _begin : NULL, *_end = NULL; itVar != _end; _end = _begin, itVar = itVar->next)

/*
 * Dont use these directly. Use the macros provided below
 */
void _linkedlist_addAfter_impl(LinkedListNode* node, LinkedListNode* newnode);
void _linkedlist_remove_impl(LinkedListNode* node);
int _linkedlist_size_impl(LinkedListNode* node);

#endif

