#include "priorityqueue.h"

bool priorityQueue_create(
	PriorityQueue* queue, int elementSize, int initialCapacity,
	int(*cmp)(const void* a, const void* b))
{
	int res = array_createGeneric(&queue->a, initialCapacity, elementSize);
	if (!res)
		return FALSE;

	queue->cmp = cmp;
	return TRUE;
}

void priorityQueue_destroy(PriorityQueue* queue)
{
	array_destroyGeneric(&queue->a);
}

// TODO : remove this
int debug_queue_pushpos=0;
int debug_queue_pushpos_zerocount=0;
void* custommemcpy(void* dst_, const void* src_, int count)
{
	u8* dst = (u8*)dst_;
	u8* src = (u8*)src_;
	while(count--)
		*dst++ = *src++;
		
	return dst_;
}

bool priorityQueue_push(PriorityQueue* queue, void* val)
{
	// Based on http://rosettacode.org/wiki/Binary_search#C
	int low = 0;
	int high = queue->a.size-1;
	char* data = (char*)queue->a.data;
	int eleSize = queue->a.elementsize;
	int mid=0;
	while (low <= high) {
		// invariants: value > A[i] for all i < low
		//             value <= A[i] for all i > high
		mid = (low + high) / 2;
		int cmp = queue->cmp(data + mid*eleSize, val);
		if (cmp >= 0)
			high = mid - 1;
		else
			low = mid + 1;
	}

	int pos = low;
	debug_queue_pushpos = pos;
	if (pos==0 && queue->a.size>=2)
	{
		debug_queue_pushpos_zerocount++;
	}

	if (queue->a.size == queue->a.capacity) {
		if (!array_reserveGeneric(
			&queue->a, queue->a.capacity ? queue->a.capacity*2 : 16)) {			
				return FALSE;
			}
	}

	int eleToMove = queue->a.size - pos;
	if (eleToMove) {
		memcpy(
			(char*)queue->a.data + (pos + 1)*eleSize,
			(char*)queue->a.data + pos*eleSize,
			eleToMove*eleSize);
	}
		
	memcpy((char*)queue->a.data + pos*eleSize, val, eleSize);
	queue->a.size++;
	return TRUE;
}

void* priorityQueue_peek(PriorityQueue* queue)
{
	if (queue->a.size)
		return (char*)queue->a.data + (queue->a.size-1)*queue->a.elementsize;
	else
		return NULL;
}

bool priorityQueue_pop(PriorityQueue* queue, void* val)
{
	return array_popGeneric(&queue->a, val);
}

int priorityQueue_delete(PriorityQueue* queue,
	bool (*pred)(const void* val, void* cookie), void* cookie)
{
	int count = 0;
	int todo = queue->a.size;
	char* dst = queue->a.data;
	char* src = queue->a.data;
	int eleSize = queue->a.elementsize;

	while (todo--)
	{	
		if (pred(src, cookie)) {
			count++;
		} else {
			if (dst!=src)
				memcpy(dst, src, eleSize);
			dst += eleSize;
		}		
		src += eleSize;
	}

	queue->a.size = queue->a.size - count;
	return count;
}
