/*******************************************************************************
 * Dynamic queue
 ******************************************************************************/
#ifndef _utils_queue_h_
#define _utils_queue_h_

#include <stddef.h>

typedef bool (*QFindFunc)(const void* a, uint32_t cookie);

// Used internally
typedef struct Queue
{
	void* data;
	int capacity;
	int tail;
	int head;
	int elementSize;
} Queue;

//
// These are used internally
//
bool _queue_createImpl(Queue* queue, int initialCapacity, int elementSize);
void _queue_destroyImpl(Queue* queue);
#define _QUEUE_ISEMPTY(q) ((q)->tail == (q)->head)
int _queue_sizeImpl(Queue* queue);
bool _queue_reserveImpl(Queue* queue, int newCapacity);
bool _queue_pushImpl(Queue* queue, void* val);
void* _queue_pushEmptyImpl(Queue* queue);
bool _queue_popImpl(Queue* queue, void* dst);
void* _queue_peekImpl(Queue* queue);
void _queue_clearImpl(Queue* queue);
int _queue_removeImpl(Queue* queue, QFindFunc pred, uint32_t cookie);
void* _queue_getAtImpl(Queue* queue, int index);

/*! Pushes an empty element into the queue, and returns its pointer
* This is useful when you want to reserve the space, then fill the item with the
* returned pointer, since this will not involve any copying. It allows you to
* setup the item inplace.
*
* \return Pointer to the new element, or NULL if out of Memory
*/

/*
These are the functions defined by QUEUE_TYPEDECLARE:

- bool queue_T_create(Queue_T* q, int initialCapacity);
Initializes the specified queue and sets the initial capacity

- void queue_T_destroy(Queue_T* q);
Destroyed the queue, releasing all used resources.

- bool queue_T_isEmpty(const Queue_T* q);
Checks if a queue is empty.
This is more efficient than using `queue_T_size`

- int queue_T_size(const Queue_T* q);
Returns the size of the queue.
If you need it to check if the queue is empty, prefer to use queue_T_isEmpty.

- bool queue_T_reserve(Queue_T* q, int newCapacity);
Expands the queue capacity.
Returns true if successfull, false otherwise.

- bool queue_T_push(Queue_T* q, const T* val);
Pushes a new element to the queue.
Returns true if successfull, false otherwise.

- T* queue_T_pushEmpty(Queue_T* q);
Pushes an empty element into the queue and returns its pointer.
This is useful for when the caller wants to make space in the queue for a new
element and initialize the element in-place, instead of copying an element into
the queue (like `queue_T_push` does).

- bool queue_T_pop(Queue_T* q, T* dst);
Pops the element at the front of the queue and copies it into `dst`.
If `dst` is NULL, the element is popped and dropped.
It returns true if an element was popped, or false if the queue is empty.

- T* queue_T_peek(Queue_T* q);
Peeks at the next element in the queue, without removing it.
It returns a pointer to the element, or NULL if the queue is empty.
The caller should not keep hold of the pointer, since it can be invalidated when
the queue changes.

- void queue_T_clear(Queue_T* q);
Removes all elements from the queue

- bool queue_T_remove(Queue_T* q, bool (*pred)(const T* a, uint32_t cookie),
					  uint32_t cookie);
Removes all elements for which the predicate function returns true.
It returns how many items were removed.

- T* queue_T_getAt(Queue_T* q, int index);
Retrieves a pointer to the element at the specified index, where index 0 is the
element at the front of the queue.
It returns NULL if index is an invalid value (e.g, index >= queue_T_size())

*/



/*!
 * Declares a queue of type T and respective functions
 */
#define QUEUE_TYPEDECLARE(T)                                                 \
	typedef struct Queue_##T                                                 \
	{                                                                        \
		T* data;                                                             \
		int capacity;                                                        \
		int tail;                                                            \
		int head;                                                            \
		int elementSize;                                                     \
	} Queue_##T;                                                             \
	inline bool queue_##T##_create(Queue_##T* q, int initialCapacity)        \
	{                                                                        \
		return _queue_createImpl((Queue*)q, initialCapacity, sizeof(T));     \
	}                                                                        \
	inline void queue_##T##_destroy(Queue_##T* q)                            \
	{                                                                        \
		_queue_destroyImpl((Queue*)q);                                       \
	}                                                                        \
	inline bool queue_##T##_isEmpty(const Queue_##T* q)                      \
	{                                                                        \
		return _QUEUE_ISEMPTY(q);                                            \
	}                                                                        \
	inline int queue_##T##_size(const Queue_##T* q)                          \
	{                                                                        \
		return _queue_sizeImpl((Queue*)q);                                   \
	}                                                                        \
	inline bool queue_##T##_reserve(Queue_##T* q, int newCapacity)           \
	{                                                                        \
		return _queue_reserveImpl((Queue*)q, newCapacity);                   \
	}                                                                        \
	inline bool queue_##T##_push(Queue_##T* q, const T* val)                 \
	{                                                                        \
		return _queue_pushImpl((Queue*)q, val);                              \
	}                                                                        \
	inline T* queue_##T##_pushEmpty(Queue_##T* q)                            \
	{                                                                        \
		return _queue_pushEmptyImpl((Queue*)q);                              \
	}                                                                        \
	inline bool queue_##T##_pop(Queue_##T* q, T* dst)                        \
	{                                                                        \
		return _queue_popImpl((Queue*)q, dst);                               \
	}                                                                        \
	inline T* queue_##T##_peek(Queue_##T* q)                                 \
	{                                                                        \
		return _queue_peekImpl((Queue*)q);                                   \
	}                                                                        \
	inline void queue_##T##_clear(Queue_##T* q)                              \
	{                                                                        \
		_queue_clearImpl((Queue*)q);                                         \
	}                                                                        \
	inline int queue_##T##_remove(Queue_##T* q,                              \
		bool (*pred)(const T* a, uint32_t cookie), uint32_t cookie)          \
	{                                                                        \
		return _queue_removeImpl((Queue*)q, (QFindFunc)pred, cookie);        \
	}                                                                        \
	inline T* queue_##T##_getAt(Queue_##T* q, int index)                     \
	{                                                                        \
		return _queue_getAtImpl((Queue*)q, index);                           \
	}                                                                        \
	
QUEUE_TYPEDECLARE(int)


#endif

