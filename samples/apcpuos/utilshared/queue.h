/*******************************************************************************
 * Dynamic queue
 ******************************************************************************/
#ifndef _utilshared_queue_h_
#define _utilshared_queue_h_

#include "utilsharedconfig.h"

/*!
*/
typedef struct Queue
{
	void* data;
	int capacity;
	int tail;
	int head;
	int elementSize;
} Queue;

/*! Checks if the queue is empty */
#define queue_isEmpty(queue) ((queue)->tail==(queue)->head)

/*! Initializes a queue */
bool queue_create(Queue* queue, int elementSize, int initialCapacity);

/*! Destroy a queue, freeing any allocated resources */
void queue_destroy(Queue* queue);

/*! Returns the number of elements int the queue
* Prefer to use queue_isEmpty if possible, since calculating the size is more
* expensive.
*/
int queue_size(Queue* queue);

/*!
* Expands the queue capacity
*/
bool queue_reserve(Queue* queue, int newCapacity);

/*! Pushes a new element into the queue */
bool queue_push(Queue* queue, void* val);

/*! Pushes an empty element into the queue, and returns its pointer
* This is useful when you want to reserve the space, then fill the item with the
* returned pointer, since this will not involve any copying. It allows you to
* setup the item inplace.
*
* \return Pointer to the new element, or NULL if out of Memory
*/
void* queue_pushEmpty(Queue* queue);

/*! Pops a new element from the queue
* \param queue
*	Queue to pop from
* \param val
*	Where the popped element will be copied to.
*	If this is NULL, the element will simply be removed from the queue, without
*	copying it anywhere.
* \return
*	True if sucessfull, false if the queue is empty
*/
bool queue_pop(Queue* queue, void* val);

/*! Peeks at the next element in the queue, without removing
* \param queue
* 	Queue to peek
* \return
*	Pointer to the next item in the queue, or NULL if the queue is empty.
*	Note that that the returned pointer will only be valid while not changes are
*	made to the queue.
*/
void* queue_peek(Queue* queue);

/*! Removes all elements from the queue */
void queue_clear(Queue* queue);

/*! Deletes all elements for which the suplied predicate returns true
* \param queue
*
* \param cmp
*	If this function returns true, the element is removed.
*	The "val" parameter is a pointer to the element to check.
*	The "cookie" parameter is the "cookie" parameter passed to queue_delete
*
* \param cookie
*	This is passed to the predicate to provide any required context
*
* \return
*	The number of items removed
*/
int queue_delete(Queue* queue, bool (*pred)(const void* val, void* cookie),
	void* cookie);

/*! Returns a pointer to the item at the specified index
* \warning
*	Use this only for debugging.
* \return
*	Pointer to the item, or NULL if the queue is empty, or the index is not a
*	valid index
*/
void* queue_getAtIndex(Queue* queue, int index);

#endif



