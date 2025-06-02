#include "detail/utils_internal.h"
#include "priorityqueue.h"

bool _pqueue_createImpl(PQueue* q, int initialCapacity, PQCompFunc cmp,
						int elementSize)
{
	int res = array_createGeneric(&q->a, initialCapacity, elementSize);
	if (!res)
		return false;

	q->cmp = cmp;
	return true;
}

void _pqueue_destroyImpl(PQueue* q)
{
	array_destroyGeneric(&q->a);
}

bool _pqueue_pushImpl(PQueue* q, const void* val)
{
	// NOTE: Putting this in a variable, because at the time of writting there
	// is a bit of a problem with vbcc trying to figure out if `queue->cmp` is
	// a variadic function call or not
	// Putting it in a variable first, gets rid of the warning.
	PQCompFunc cmpFunc = q->cmp;

	// Based on http://rosettacode.org/wiki/Binary_search#C
	int low = 0;
	int high = q->a.size - 1;
	char* data = (char*)q->a.data;
	int eleSize = q->a.elementsize;
	int mid = 0;
	while (low <= high) {
		// invariants: value > A[i] for all i < low
		//             value <= A[i] for all i > high
		mid = (low + high) / 2;
		int cmp = cmpFunc(data + mid * eleSize, val);
		if (cmp >= 0)
			high = mid - 1;
		else
			low = mid + 1;
	}

	int pos = low;

	if (q->a.size == q->a.capacity) {
		int newCapacity = q->a.capacity ? q->a.capacity * 2 : 16;
		if (!array_reserveGeneric(&q->a, newCapacity)) {
			return false;
		}
	}

	int eleToMove = q->a.size - pos;
	if (eleToMove) {
		memcpy((char*)q->a.data + (pos + 1) * eleSize,
			   (char*)q->a.data + pos * eleSize, eleToMove * eleSize);
	}

	memcpy((char*)q->a.data + pos * eleSize, val, eleSize);
	q->a.size++;
	return true;
}

void* _pqueue_peekImpl(PQueue* q)
{
	if (q->a.size)
		return (char*)q->a.data + (q->a.size - 1) * q->a.elementsize;
	else
		return NULL;
}

bool _pqueue_popImpl(PQueue* q, void* val)
{
	return array_popGeneric(&q->a, val);
}

int _pqueue_removeImpl(PQueue* q,
					   bool (*pred)(const void* val, uint32_t cookie),
					   uint32_t cookie)
{
	int count = 0;
	int todo = q->a.size;
	char* dst = q->a.data;
	char* src = q->a.data;
	int eleSize = q->a.elementsize;

	while (todo--) {
		if (pred(src, cookie)) {
			count++;
		} else {
			if (dst != src)
				memcpy(dst, src, eleSize);
			dst += eleSize;
		}
		src += eleSize;
	}

	q->a.size = q->a.size - count;
	return count;
}
