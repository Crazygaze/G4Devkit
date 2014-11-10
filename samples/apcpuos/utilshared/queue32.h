//////////////////////////////////////////////////////////////////////////
// Queue for 32bits elements
//
// This is hardcoded for 32 bits elements, for performance reasons, since
// this kind of queue is used very frequently
//////////////////////////////////////////////////////////////////////////

#ifndef _utilshared_queue32_h_
#define _utilshared_queue32_h_

#include "utilsharedconfig.h"

typedef int QueueElement;
typedef struct Queue32
{
	QueueElement* data;
	int capacity;
	int tail; // write position
	int head; // read position
} Queue32;

/*! Checks if the queue is empty */
#define queue32_isEmpty(queue) ((queue)->tail==(queue)->head)

/*! Initializes a queue */
bool queue32_create(Queue32* queue, int initialCapacity);

/*! Destroy a queue, freeing any allocated resources */
void queue32_destroy(Queue32* queue);

/*! Returns the number of elements int the queue
Prefer to use queue_isEmpty if possible, since calculating the size is more expensive
*/
int queue32_size(Queue32* queue);

/*!
Expands the queue capacity
*/
bool queue32_reserve(Queue32* queue, int newCapacity);

/*! Pushes a new element into the queue */
bool queue32_push(Queue32* queue, int val);

/*! Pops a new element from the queue */
bool queue32_pop(Queue32* queue, int* val);

/*! Peeks at the next element in the queue, without removing */
bool queue32_peek(Queue32* queue, int* val);

/*! Removes all elements from the queue */
void queue32_clear(Queue32* queue);

/*! Deletes from the queue all elements with the specified value
\return
	The number of items removed
*/
int queue32_delete(Queue32* queue, int val);

/*! Use this only for debugging */
int queue32_getAtIndex(Queue32* queue, int index);

#endif


