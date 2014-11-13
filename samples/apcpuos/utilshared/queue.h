#ifndef _utilshared_queue_h_
#define _utilshared_queue_h_

#include "utilsharedconfig.h"

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
Prefer to use queue_isEmpty if possible, since calculating the size is more expensive
*/
int queue_size(Queue* queue);

/*!
Expands the queue capacity
*/
bool queue_reserve(Queue* queue, int newCapacity);

/*! Pushes a new element into the queue */
bool queue_push(Queue* queue, void* val);

/*! Pushes an empty element into the queue, and returns its pointer
* \return Pointer to the new element, or NULL if out of Memory
*/
void* queue_pushEmpty(Queue* queue);

/*! Pops a new element from the queue
\param queue
	Queue to pop from
\param val
	Where the popped element will be copied to
\return
	True if sucessfull, false if the queue is empty
*/
bool queue_pop(Queue* queue, void* val);

/*! Peeks at the next element in the queue, without removing */
bool queue_peek(Queue* queue, void** val);

/*! Removes all elements from the queue */
void queue_clear(Queue* queue);

/*! Deletes from the queue all elements for which the suplied predicate return
true
\param queue
\param cmp
	If this function returns true, the element is removed.
	The "val" parameter is a pointer to the element to check.
	The "cookie" parameter is the "cookie" parameter passed to queue_delete
\param cookie
	P
\return
	The number of items removed
*/
int queue_delete(Queue* queue, bool (*pred)(const void* val, void* cookie),
	void* cookie);

/*! Use this only for debugging */
void* queue_getAtIndex(Queue* queue, int index);

#endif



