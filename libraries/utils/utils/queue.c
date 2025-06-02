#include "detail/utils_internal.h"
#include "queue.h"
#include <string.h>
#include <stdlib.h>

#define ELE_PTR(index) ((char*)queue->data + ((index)*queue->elementSize))

bool _queue_createImpl(Queue* queue, int initialCapacity, int elementSize)
{
	memset(queue, 0, sizeof(*queue));
	queue->elementSize = elementSize;
	if (initialCapacity)
		return _queue_reserveImpl(queue, initialCapacity);
	else
		return true;
}

void _queue_destroyImpl(Queue* queue)
{
	if (queue->data) {
		free(queue->data);
		queue->data = NULL;
	}
}

int _queue_sizeImpl(Queue* queue)
{
	if (queue->capacity == 0)
		return 0;
	else
		return (queue->tail + queue->capacity - queue->head) % queue->capacity;
}

bool _queue_reserveImpl(Queue* queue, int newCapacity)
{
	// Due to the fact that we don't track the actual fillcount, there is always
	// one slot that is empty (so we can detect empty or full queue).
	// Therefore, we need to allocate 1 more than what the caller asked for
	newCapacity++;

	if (newCapacity <= queue->capacity)
		return true;

	void* newData = malloc(newCapacity * queue->elementSize);
	if (!newData)
		return false;

	int size = _queue_sizeImpl(queue);

	// Copy the existing data to the new buffer
	if (queue->data) {
		if (size) {
			int tmp = queue->capacity - queue->head;
			int len = min(size, tmp);
			memcpy(newData, ELE_PTR(queue->head), len*queue->elementSize);
			if (len != size) {
				memcpy(
					(char*)newData + (len*queue->elementSize), queue->data,
					(size - len) * queue->elementSize);
			}
		}
		free(queue->data);
	}

	queue->data = newData;
	queue->capacity = newCapacity;
	queue->head = 0;
	queue->tail = size;
	return true;
}

void* _queue_pushEmptyImpl(Queue* queue)
{
	// Check if full
	if (queue->capacity==0 || _queue_sizeImpl(queue)==queue->capacity-1) {
		int newCapacity = queue->capacity ? (queue->capacity-1) * 2 : 16;
		if (!_queue_reserveImpl(queue, newCapacity)) 
			return NULL;
	}

	void* newEle = ELE_PTR(queue->tail);
	queue->tail = (queue->tail + 1) % queue->capacity;
	return newEle;
}

bool _queue_pushImpl(Queue* queue, void* val)
{
	void* newEle = _queue_pushEmptyImpl(queue);
	if (!newEle)
		return false;
	memcpy(newEle, val, queue->elementSize);
	return true;
}

bool _queue_popImpl(Queue* queue, void* val)
{
	if (_QUEUE_ISEMPTY(queue))
		return false;

	if (val) {
		memcpy(val, ELE_PTR(queue->head), queue->elementSize);
	}
	queue->head = (queue->head + 1) % queue->capacity;
	return true;
}

void* _queue_peekImpl(Queue* queue)
{
	if (_QUEUE_ISEMPTY(queue))
		return NULL;
		
	return ELE_PTR(queue->head);
}

void _queue_clearImpl(Queue* queue)
{
	queue->head = queue->tail = 0;
}

int _queue_removeImpl(Queue* queue, bool (*pred)(const void* val, uint32_t cookie),
	uint32_t cookie)
{
	int count = 0;
	int todo = _queue_sizeImpl(queue);
	int dstIndex = queue->head;
	int srcIndex = queue->head;

	while (todo--)
	{
		if (pred(ELE_PTR(srcIndex), cookie)) {
			count++;
		} else {
			memcpy(ELE_PTR(dstIndex), ELE_PTR(srcIndex), queue->elementSize);
			dstIndex = (dstIndex + 1) % queue->capacity;
		}
		srcIndex = (srcIndex + 1) % queue->capacity;
	}

	queue->tail = dstIndex;
	return count;
}

void* _queue_getAtImpl(Queue* queue, int index)
{
	// NOTE: Using (unsigned) in case index is negative
	if ((unsigned)index>=(unsigned)_queue_sizeImpl(queue))
		return NULL;
	return ELE_PTR((queue->head + index) % queue->capacity);
}
