#include "dynamicarray.h"

ARRAY_TYPEDEFINE_NATIVE(int)

//////////////////////////////////////////////////////////////////////////
//
//		Private code
//
//////////////////////////////////////////////////////////////////////////
int array_createGeneric(Array_generic* a, int initialcapacity, int elementsize)
{
	memset(a, 0, sizeof(*a));
	a->capacity = initialcapacity;
	if (initialcapacity) {
		a->data = utilshared_malloc(initialcapacity*elementsize);
		if (!a->data) {
			return FALSE;
		}
	}
	a->elementsize = elementsize;
	return TRUE;
}

int array_reserveGeneric(Array_generic* a, int newcapacity)
{
	if (newcapacity > a->capacity)
	{
		void* newdata = utilshared_realloc(a->data, newcapacity*a->elementsize);
		if (!newdata){
			return FALSE;
		}

		a->data = newdata;
		a->capacity = newcapacity;		
	}

	return TRUE;
}

void array_destroyGeneric(Array_generic* a)
{
	if (a->data)
	{
		utilshared_free(a->data);
		memset(a, 0, sizeof(*a));
	}
}

int array_pushGeneric(Array_generic* a, const void* val)
{
	// Grow the array if necessary
	if (a->size==a->capacity)
	{
		if (!array_reserveGeneric(a, (a->capacity==0) ? 16 : a->capacity*2)) {
			return FALSE;
		}
	}

	// insert the new element
	void* ptr = (char*)a->data + (a->size*a->elementsize);
	memcpy(ptr, val, a->elementsize);
	a->size++;
	return TRUE;
}

void* array_pushGenericEmpty(Array_generic* a)
{
	void* ptr;
	// Grow the array if necessary
	if (a->size==a->capacity)
	{
		if (!array_reserveGeneric(a, (a->capacity==0) ? 16 : a->capacity*2)) {
			return FALSE;
		}
	}

	ptr = (char*)a->data + (a->size*a->elementsize);
	// memset the space of the new element
	memset( ptr, 0, a->elementsize);
	a->size++;
	return ptr;
}

int array_popGeneric(Array_generic* a, void* dest)
{
	if (a->size==0) {
		return FALSE;
	}

	if (dest) {
		memcpy(dest, (char*)a->data + ((a->size-1)*a->elementsize), a->elementsize);
	}

	a->size--;
	return TRUE;
}

void apcpuas_array_generic_clear(Array_generic* a)
{
	a->size=0;
}

int array_removeAtGeneric(Array_generic* a, unsigned index)
{
	if (index >= (unsigned)a->size) {
		return TRUE;
	}
	
	// If it's not the last index, we need to move items back
	if (index < (unsigned)a->size-1) {
		memcpy(
			(uint8_t*)a->data + (index*a->elementsize), 
			(uint8_t*)a->data + ((index+1)*a->elementsize),
			(a->size-index-1) * a->elementsize);
	}
	
	a->size--;
	return TRUE;
}
