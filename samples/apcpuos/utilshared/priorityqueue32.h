//////////////////////////////////////////////////////////////////////////
// Priority queue for 32bits elements
//
// This is hardcoded for 32bits elements, for performance reasons
//////////////////////////////////////////////////////////////////////////

#ifndef _utilshared_priorityqueue32_h_
#define _utilshared_priorityqueue32_h_

#include "dynamicarray.h"

typedef struct PriorityQueue32
{
	array_int a;
	int(*cmp)(const void* a, const void* b);
} PriorityQueue32;

/*! Initializes a priority queue
\param queue
	Queue to initialize
\param initialCapacity
	How many items the queue can handle initialy
\param cmp
	Compare function responsible for sorting the queue
*/
bool priorityQueue32_create(
	PriorityQueue32* queue, int initialCapacity,
	int (*cmp)(const void* a, const void* b));

/*! Destroys a priority queue, releasing any used memory
*/
void priorityQueue32_destroy(PriorityQueue32* queue);

/*! Pushes a new element to the queue
*/
bool priorityQueue32_push(PriorityQueue32* queue, int val);

/*! Peeks at the queue, without removing the item
*/
bool priorityQueue32_peek(PriorityQueue32* queue, int* val);

/*! Remove an item from the queue
*/
bool priorityQueue32_pop(PriorityQueue32* queue, int* val);

/*! Drops an item from the queue
*/
void priorityQueue32_popAndDrop(PriorityQueue32* queue);

/*! Removes from the queue the specified item.
\return
	How many items were removed
*/
int priorityQueue32_delete(PriorityQueue32* queue, int val);

#endif

