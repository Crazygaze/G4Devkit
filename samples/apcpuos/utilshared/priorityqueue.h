//////////////////////////////////////////////////////////////////////////
// Generic Priority queue
//
//////////////////////////////////////////////////////////////////////////

#ifndef _utilshared_priorityqueue_h_
#define _utilshared_priorityqueue_h_

#include "dynamicarray.h"

typedef struct PriorityQueue
{
	Array_generic a;
	int(*cmp)(const void* a, const void* b);
} PriorityQueue;

/*! Initializes a priority queue
\param queue
	Queue to initialize
\param elementSize
	Queue element size, in bytes
\param initialCapacity
	How many items the queue can handle initialy
\param cmp
	Compare function responsible for sorting the queue
*/
bool priorityQueue_create(
	PriorityQueue* queue, int elementSize, int initialCapacity,
	int (*cmp)(const void* a, const void* b));

/*! Destroys a priority queue, releasing any used memory
*/
void priorityQueue_destroy(PriorityQueue* queue);

/*! Pushes a new element to the queue
*/
bool priorityQueue_push(PriorityQueue* queue, void* val);

/*! Peeks at the queue, without removing the item
* \param queue
*	Queue to peek
* \return
*	Pointer to the next item in the queue, or NULL if the queue is empty.
*	Note that the returned pointer is only valid while no further changes are
*	made to the queue.
*/
void* priorityQueue_peek(PriorityQueue* queue);

/*! Remove an item from the queue
* \param queue
*	Queue to remove from
* \param val
*	Where the popped element will be copied to.
*	If this is NULL, the element will simply be removed from the queue, without
*	copying it anywhere.
*/
bool priorityQueue_pop(PriorityQueue* queue, void* val);


/*! Removes all elements for which the suplied predicate returns true
\param queue
\param pred
	Predicate function to use.
	The "val" parameter is a pointer to the element to check.
	The "cookie" parameter is the "cookie" parameter passed to
	priorityQueue_delete.
\param cookie
	Cookie to pass to the predicate function.
\return
	How many items were removed
*/
int priorityQueue_delete(PriorityQueue* queue,
	bool (*pred)(const void* val, void* cookie), void* cookie);

#endif

