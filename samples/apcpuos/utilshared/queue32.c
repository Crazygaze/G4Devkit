#include "queue32.h"

bool queue32_create(Queue32* queue, int initialCapacity)
{
	utilshared_memset(queue, 0, sizeof(*queue));
	if (initialCapacity)
		return queue32_reserve(queue, initialCapacity);
	else
		return true;
}

void queue32_destroy(Queue32* queue)
{
	if (queue->data) {
		utilshared_free(queue->data);
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

	QueueElement* newData =
		(QueueElement*) utilshared_malloc(newCapacity * sizeof(QueueElement));
	if (!newData)
		return false;

	int size = queue32_size(queue);

	// Copy the existing data to the new buffer
	if (queue->data) {
		if (size) {
			int tmp = queue->capacity - queue->head;
			int len = min(size, tmp);
			utilshared_memcpy(newData, queue->data + queue->head,
				len*sizeof(QueueElement));
			if (len != size) {
				utilshared_memcpy(newData + len, queue->data,
					(size - len) * sizeof(QueueElement));
			}
		}
		utilshared_free(queue->data);
	}

	queue->data = newData;
	queue->capacity = newCapacity;
	queue->head = 0;
	queue->tail = size;
	return true;
}

bool queue32_push(Queue32* queue, int val)
{
	// Check if full
	if (queue->capacity==0 || queue32_size(queue)==queue->capacity-1) {
		if (!queue32_reserve(queue, queue->capacity ? queue->capacity * 2 : 16))
			return false;
	}

	queue->data[queue->tail] = val;
	queue->tail = (queue->tail + 1) % queue->capacity;
	return true;
}

bool queue32_pop(Queue32* queue, int* val)
{
	if (queue32_isEmpty(queue))
		return false;

	*val = queue->data[queue->head];
	queue->head = (queue->head + 1) % queue->capacity;
	return true;
}

bool queue32_peek(Queue32* queue, int* val)
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

int queue32_delete(Queue32* queue, int val)
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

int queue32_getAtIndex(Queue32* queue, int index)
{
	return queue->data[(queue->head + index) % queue->capacity];
}
