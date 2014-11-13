#include "queue.h"

#define ELE_PTR(index) ((char*)queue->data + ((index)*queue->elementSize))

bool queue_create(Queue* queue, int elementSize, int initialCapacity)
{
	utilshared_memset(queue, 0, sizeof(*queue));
	queue->elementSize = elementSize;
	if (initialCapacity)
		return queue_reserve(queue, initialCapacity);
	else
		return true;
}

void queue_destroy(Queue* queue)
{
	if (queue->data) {
		utilshared_free(queue->data);
		queue->data = NULL;
	}
}

int queue_size(Queue* queue)
{
	if (queue->capacity == 0)
		return 0;
	else
		return (queue->tail + queue->capacity - queue->head) % queue->capacity;
}

bool queue_reserve(Queue* queue, int newCapacity)
{
	// Due to the fact that we don't track the actual fillcount, there is always
	// one slot that is empty (so we can tell the identify empty or full queue).
	// Therefore, we need to allocate 1 more than what the caller asked for
	newCapacity++;

	if (newCapacity <= queue->capacity)
		return true;

	void* newData = utilshared_malloc(newCapacity * queue->elementSize);
	if (!newData)
		return false;

	int size = queue_size(queue);

	// Copy the existing data to the new buffer
	if (queue->data) {
		if (size) {
			int tmp = queue->capacity - queue->head;
			int len = min(size, tmp);
			utilshared_memcpy( newData, ELE_PTR(queue->head),
				len*queue->elementSize);
			if (len != size) {
				utilshared_memcpy(
					(char*)newData + (len*queue->elementSize), queue->data,
					(size - len) * queue->elementSize);
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

void* queue_pushEmpty(Queue* queue)
{
	// Check if full
	if (queue->capacity==0 || queue_size(queue)==queue->capacity-1) {
		if (!queue_reserve(queue, queue->capacity ? queue->capacity * 2 : 16))
			return NULL;
	}

	void* newEle = ELE_PTR(queue->tail);
	queue->tail = (queue->tail + 1) % queue->capacity;
	return newEle;
}

bool queue_push(Queue* queue, void* val)
{
	void* newEle = queue_pushEmpty(queue);
	if (!newEle)
		return false;
	memcpy(newEle, val, queue->elementSize);
	return true;
}

bool queue_pop(Queue* queue, void* val)
{
	if (queue_isEmpty(queue))
		return false;

	memcpy(val, ELE_PTR(queue->head), queue->elementSize);
	queue->head = (queue->head + 1) % queue->capacity;
	return true;
}

bool queue_peek(Queue* queue, void** val)
{
	if (queue_isEmpty(queue))
		return false;
	//*((uint32_t**)val) = ELE_PTR(queue->head);
	*val = ELE_PTR(queue->head);
	return true;
}

void queue_clear(Queue* queue)
{
	queue->head = queue->tail = 0;
}

int queue_delete(Queue* queue, bool (*pred)(const void* val, void* cookie),
	void* cookie)
{
	int count = 0;
	int todo = queue_size(queue);
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

void* queue_getAtIndex(Queue* queue, int index)
{
	return ELE_PTR((queue->head + index) % queue->capacity);
}
