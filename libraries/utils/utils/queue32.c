#include "detail/utils_internal.h"
#include "queue32.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

bool queue32_create(Queue32* queue, int initialCapacity)
{
	memset(queue, 0, sizeof(*queue));
	if (initialCapacity)
		return queue32_reserve(queue, initialCapacity);
	else
		return true;
}

void queue32_destroy(Queue32* queue)
{
	if (queue->data) {
		free(queue->data);
		queue->data = NULL;
	}
}

int queue32_size(Queue32* queue)
{
	if (queue->capacity == 0)
		return 0;
	else
		return (queue->tail + queue->capacity - queue->head) % queue->capacity;
}

bool queue32_reserve(Queue32* queue, int newCapacity)
{
	// Due to the fact that we don't track the actual fillcount, there is always
	// one slot that is empty (so we can tell the identify empty or full queue).
	// Therefore, we need to allocate 1 more than what the caller asked for
	newCapacity++;

	if (newCapacity <= queue->capacity)
		return true;

	uint32_t* newData = (uint32_t*)malloc(newCapacity * sizeof(uint32_t));
	if (!newData)
		return false;

	int size = queue32_size(queue);

	// Copy the existing data to the new buffer
	if (queue->data) {
	
		if (size) {
			int tmp = queue->capacity - queue->head;
			int len = min(size, tmp);
			memcpy(newData, queue->data + queue->head, len * sizeof(uint32_t));
			if (len != size) {
				memcpy(newData + len, queue->data,
					   (size - len) * sizeof(uint32_t));
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

bool queue32_push(Queue32* queue, uint32_t val)
{
	// Check if full
	if (queue->capacity==0 || queue32_size(queue)==queue->capacity-1) {
		int newCapacity = queue->capacity ? (queue->capacity-1) * 2 : 16;
		if (!queue32_reserve(queue, newCapacity))
			return false;
	}

	queue->data[queue->tail] = val;
	queue->tail = (queue->tail + 1) % queue->capacity;
	return true;
}

bool queue32_pop(Queue32* queue, uint32_t * val)
{
	if (queue32_isEmpty(queue))
		return false;

	*val = queue->data[queue->head];
	queue->head = (queue->head + 1) % queue->capacity;
	return true;
}

bool queue32_peek(Queue32* queue, uint32_t* val)
{
	if (queue32_isEmpty(queue))
		return false;
	*val = queue->data[queue->head];
	return true;
}

void queue32_clear(Queue32* queue)
{
	queue->head = queue->tail = 0;
}

int queue32_remove(Queue32* queue, uint32_t val)
{
	int count = 0;
	int todo = queue32_size(queue);
	int dstIndex = queue->head;
	int srcIndex = queue->head;

	while (todo--)
	{
		if (queue->data[srcIndex] == val) {
			count++;
		} else {
			queue->data[dstIndex] = queue->data[srcIndex];
			dstIndex = (dstIndex + 1) % queue->capacity;
		}
		srcIndex = (srcIndex + 1) % queue->capacity;
	}

	queue->tail = dstIndex;
	return count;
}

uint32_t queue32_getAt(Queue32* queue, int index)
{
	assert((unsigned int)index < queue32_size(queue));
	return queue->data[(queue->head + index) % queue->capacity];
}
