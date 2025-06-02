/*!
 * Queue specialized for 32bits elements.
 *
 */
 
#ifndef _utils_queue32_h_
#define _utils_queue32_h_

#include <stddef.h>

typedef struct Queue32
{
	uint32_t* data;
	int capacity;
	int tail; // write position
	int head; // read position
} Queue32;

/*! Checks if the queue is empty */
inline bool queue32_isEmpty(Queue32* queue)
{
	return queue->tail == queue->head;
}

/*! Initializes a queue */
bool queue32_create(Queue32* queue, int initialCapacity);

/*! Destroy a queue, freeing any allocated resources */
void queue32_destroy(Queue32* queue);

/*!
 * Returns the number of elements int the queue.
 * Prefer to use queue_isEmpty if possible, since calculating the size is more
 * expensive.
 */
int queue32_size(Queue32* queue);

/*!
 * Expands the queue capacity
 */
bool queue32_reserve(Queue32* queue, int newCapacity);

/*! Pushes a new element into the queue */
bool queue32_push(Queue32* queue, uint32_t val);

/*!
 * Pops a new element from the queue
 *
 * \param val If not NULL, on exit it will contain the element. Note that by
 * passing this as NULL, the operation simply drops the element at the front of
 * the queue.
 *
 * \return true if an element was popped, false if the queue was empty.
 */
bool queue32_pop(Queue32* queue, uint32_t* val);

/*!
 * Peeks at the next element in the queue, without removing
 * \param val On exit, it will contain the element. This can't be NULL.
 * \return true if operation succeeded, false if the queue was empty.
 */
bool queue32_peek(Queue32* queue, uint32_t* val);

/*! Removes all elements from the queue */
void queue32_clear(Queue32* queue);

/*!
 * Removes from the queue all elements with the specified value.
 * \return The number of items removed
 */
int queue32_remove(Queue32* queue, uint32_t val);

/*!
 * Retrieves the element and the specified index, where index 0 is the element
 * at the front of the queue.
 * If the index is out of bounds, behaviour is undefined.
 */
uint32_t queue32_getAt(Queue32* queue, int index);

#endif

