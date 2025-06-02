#include "detail/utils_internal.h"
#include "dynamicarray.h"
#include <stdlib.h>

ARRAY_TYPEDEFINE_NATIVE(int)

//////////////////////////////////////////////////////////////////////////
//
//		Private code
//
//////////////////////////////////////////////////////////////////////////
bool array_createGeneric(Array_generic* a, int initialcapacity, int elementsize)
{
	memset(a, 0, sizeof(*a));
	a->capacity = initialcapacity;
	if (initialcapacity) {
		a->data = malloc(initialcapacity * elementsize);
		if (!a->data) {
			return false;
		}
	}
	a->elementsize = elementsize;
	return true;
}

bool array_reserveGeneric(Array_generic* a, int newcapacity)
{
	if (newcapacity > a->capacity) {
		void* newdata = realloc(a->data, newcapacity * a->elementsize);
		if (!newdata) {
			return false;
		}

		a->data = newdata;
		a->capacity = newcapacity;
	}

	return true;
}

void array_destroyGeneric(Array_generic* a)
{
	if (a->data) {
		free(a->data);
		memset(a, 0, sizeof(*a));
	}
}

bool array_pushGeneric(Array_generic* a, const void* val)
{
	// Grow the array if necessary
	if (a->size == a->capacity) {
		int newCapacity = a->capacity ? a->capacity * 2 : 16;
		if (!array_reserveGeneric(a, newCapacity)) {
			return false;
		}
	}

	// insert the new element
	void* ptr = (char*)a->data + (a->size * a->elementsize);
	memcpy(ptr, val, a->elementsize);
	a->size++;
	return true;
}

void* array_pushGenericEmpty(Array_generic* a)
{
	void* ptr;
	// Grow the array if necessary
	if (a->size == a->capacity) {
		int newCapacity = a->capacity ? a->capacity * 2 : 16;
		if (!array_reserveGeneric(a, newCapacity)) {
			return NULL;
		}
	}

	ptr = (char*)a->data + (a->size * a->elementsize);
	// memset the space of the new element
	memset(ptr, 0, a->elementsize);
	a->size++;
	return ptr;
}

bool array_popGeneric(Array_generic* a, void* dest)
{
	if (a->size == 0) {
		return false;
	}

	if (dest) {
		memcpy(dest, (char*)a->data + ((a->size - 1) * a->elementsize),
			   a->elementsize);
	}

	a->size--;
	return true;
}

void apcpuas_array_generic_clear(Array_generic* a)
{
	a->size = 0;
}

bool array_removeAtGeneric(Array_generic* a, unsigned index)
{
	if (index >= (unsigned)a->size) {
		return false;
	}

	// If it's not the last index, we need to move items back
	if (index < (unsigned)a->size - 1) {
		memcpy((uint8_t*)a->data + (index * a->elementsize),
			   (uint8_t*)a->data + ((index + 1) * a->elementsize),
			   (a->size - index - 1) * a->elementsize);
	}

	a->size--;
	return true;
}
