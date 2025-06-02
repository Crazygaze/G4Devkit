#include "detail/utils_internal.h"
#include "priorityqueue32.h"

bool priorityQueue32_create(
	PriorityQueue32* queue,
	int initialCapacity, int(*cmp)(const void* a, const void* b))
{
	int res = array_int_create(&queue->a, initialCapacity);
	if (!res)
		return FALSE;

	queue->cmp = cmp;
	return TRUE;
}

void priorityQueue32_destroy(PriorityQueue32* queue)
{
	array_int_destroy(&queue->a);
}

bool priorityQueue32_push(PriorityQueue32* queue, int val)
{
	// Based on http://rosettacode.org/wiki/Binary_search#C
	int low = 0;
	int high = queue->a.size-1;
	int* data = queue->a.data;
	int mid=0;
	while (low <= high) {
		// invariants: value > A[i] for all i < low
		//             value <= A[i] for all i > high
		mid = (low + high) / 2;
		int cmp = queue->cmp(&data[mid], &val);
		if (cmp >= 0)
			high = mid - 1;
		else
			low = mid + 1;
	}

	int pos = low;

	if (queue->a.size == queue->a.capacity)
		if (!array_reserveGeneric(
			(Array_generic*)&queue->a,
			queue->a.capacity ? queue->a.capacity*2 : 16)) {			
				return FALSE;
			}

	utilshared_memcpy(
		&queue->a.data[pos + 1],
		&queue->a.data[pos],
		(queue->a.size - pos)*queue->a.elementsize);

	queue->a.data[pos] = val;
	queue->a.size++;
	return TRUE;
}

bool priorityQueue32_peek(PriorityQueue32* queue, int* val)
{
	if (queue->a.size) {
		*val = ARRAY_LAST(queue->a);
		return TRUE;
	}
	else {
		return FALSE;
	}
}

bool priorityQueue32_pop(PriorityQueue32* queue, int* val)
{
	return array_int_pop(&queue->a, val);
}

void priorityQueue32_popAndDrop(PriorityQueue32* queue)
{
	array_int_popAndDrop(&queue->a);
}

int priorityQueue32_delete(PriorityQueue32* queue, int val)
{
	int count = 0;
	int todo = queue->a.size;
	int dstIndex = 0;
	int srcIndex = 0;
	int* data = queue->a.data;

	while (todo--)
	{
		if (data[srcIndex] == val) {
			count++;
		} else {
			data[dstIndex] = data[srcIndex];
			dstIndex++;
		}
		srcIndex++;
	}

	queue->a.size = dstIndex;
	return count;

}
