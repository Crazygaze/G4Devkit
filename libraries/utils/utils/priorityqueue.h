//////////////////////////////////////////////////////////////////////////
//
// Generic Priority queue
//
//////////////////////////////////////////////////////////////////////////

#ifndef _utils_priorityqueue_h_
#define _utils_priorityqueue_h_

#include "dynamicarray.h"

typedef int (*PQCompFunc)(const void* a, const void* b);
typedef bool (*PQFindFunc)(const void* a, uint32_t cookie);
typedef struct PQueue
{
	Array_generic a;
	PQCompFunc cmp;
} PQueue;


// These are used internally only and should not be called directly
bool _pqueue_createImpl(PQueue* q, int initialCapacity, PQCompFunc cmp, int elementSize);
void _pqueue_destroyImpl(PQueue* q);
bool _pqueue_pushImpl(PQueue* q, const void* val);
void* _pqueue_peekImpl(PQueue* q);
bool _pqueue_popImpl(PQueue* q, void* dst);
int _pqueue_removeImpl(PQueue* q, PQFindFunc, uint32_t cookie);

/*
These are the functions defined by PQUEUE_TYPEDECLARE

- bool pqueue_T_create(PQueue_T* q, int initialCapacity,
					   int (*cmp)(const T* a, const T* b));
Initializes a queue.
Returns true if successful, false otherwise.

- void pqueue_T_destroy(PQueue_T* q);
Destroys a queue, releasing any resources.

- bool pqueue_T_push(PQueue_T* q, const T* val);
Pushes the specified item to the queue. It will be inserting at a position
according to the compare function specified in `pqueue_T_create`. Returns true
if successfull, false otherwise.

- const T* peek pqueue_T_peek(PQueue_T* q);
Returns a pointer to the element at the front of the queue, without removing it,
or NULL if the queue is empty.
IMPORTANT: The caller should NOT keep hold of the returned pointer, because it
will be invalid once the queue changes.

- bool pqueue_T_pop(PQueue_T* q, T* dst);
Pops the element at the front of the queue. The element will be copied to `dst`.
Returns true if an element was popped, or false if the queue was empty.
If `dst` is NULL, the element is simply popped and dropped (not copied anywhere)

- bool pqueue_T_remove(PQueue_T* q, bool (*pred)(const T* a, uint32_t cookie),
uint32_t cookie); Removes all elements for which the predicate function returns
true. It returns how many items were removed.

*/

/*!
 * Declares a PQueue_T type, and respective functions
 */
#define PQUEUE_TYPEDECLARE(T)                                                 \
	typedef struct PQueue_##T {                                               \
		Array_generic a;                                                      \
		int (*cmp)(const T* a, const T* b);                                   \
	} PQueue_##T;                                                             \
	inline bool pqueue_##T##_create(PQueue_##T* q, int initialCapacity,       \
									int (*cmp)(const T* a, const T* b))       \
	{                                                                         \
		return _pqueue_createImpl((PQueue*)q, initialCapacity,                \
								  (PQCompFunc)cmp, sizeof(T));                \
	}                                                                         \
	inline void pqueue_##T##_destroy(PQueue_##T* q)                           \
	{                                                                         \
		_pqueue_destroyImpl((PQueue*)q);                                      \
	}                                                                         \
	inline bool pqueue_##T##_push(PQueue_##T* q, const T* val)                \
	{                                                                         \
		return _pqueue_pushImpl((PQueue*)q, val);                             \
	}                                                                         \
	inline const T* pqueue_##T##_peek(PQueue_##T* q)                          \
	{                                                                         \
		return _pqueue_peekImpl((PQueue*)q);                                  \
	}                                                                         \
	inline bool pqueue_##T##_pop(PQueue_##T* q, T* dst)                       \
	{                                                                         \
		return _pqueue_popImpl((PQueue*)q, dst);                              \
	}                                                                         \
	inline int pqueue_##T##_remove(PQueue_##T* q,                             \
								   bool (*pred)(const T* a, uint32_t cookie), \
								   uint32_t cookie)                           \
	{                                                                         \
		return _pqueue_removeImpl((PQueue*)q, (PQFindFunc)pred, cookie);      \
	}

#endif

