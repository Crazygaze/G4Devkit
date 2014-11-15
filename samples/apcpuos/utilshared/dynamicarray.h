/*******************************************************************************
 * Dynamic array
 * It uses the heap to dynamically grow
 ******************************************************************************/

#ifndef _utilshared_dynamicarray_h__
#define _utilshared_dynamicarray_h__

#ifdef __cplusplus //if we are compiling as C++, tell
extern "C" {
#endif

#include "utilsharedconfig.h"

/*
* WARNING: These fields need to match the structs declared with ARRAY_TYPEDECLARE
*/
typedef struct Array_generic {
	int size;
	int capacity;
	void* data;
	int elementsize;
} Array_generic;

/*
* These functions should NOT be used directly. Use the provided macros to
* create functions for the types you want
*/
int array_createGeneric(Array_generic* a, int initialcapacity, int elementsize);
int array_reserveGeneric(Array_generic* a, int newcapacity);
void array_destroyGeneric(Array_generic* a);
int array_pushGeneric(Array_generic* a, const void* val);
void* array_pushGenericEmpty(Array_generic* a);
int array_popGeneric(Array_generic* a, void* dest);
int array_removeAtGeneric(Array_generic* a, unsigned index);



/*
* Declares a new type of array for "elementtype" type
* For example, if you wish to use arrays of type int, put this somewhere:       
*		ARRAY_TYPEDECLARE(int);
*/
#define ARRAY_TYPEDECLARE(elementtype) \
	typedef struct array_##elementtype { \
		int size; \
		int capacity; \
		elementtype* data; \
		int elementsize; \
	} array_##elementtype; \
	int array_##elementtype##_create(array_##elementtype* a, int initialcapacity); \
	void array_##elementtype##_destroy(array_##elementtype* a); \
	int array_##elementtype##_pushPtr(array_##elementtype* a, const elementtype* val); \
	int array_##elementtype##_pushVal(array_##elementtype* a, elementtype val); \
	int array_##elementtype##_pop(array_##elementtype* a, elementtype* dest); \
	int array_##elementtype##_popAndDrop(array_##elementtype* a); \
	void array_##elementtype##_clear(array_##elementtype* a); \
	void elementtype##_create(elementtype* p); \
	void elementtype##_destroy(elementtype* p); \
	void elementtype##_copy(elementtype const* src, elementtype* dest);

#define ARRAY_TYPEDECLARE_FIND(elementtype, keytype) \
	ARRAY_TYPEDECLARE(elementtype) \
	int array_##elementtype##_find(array_##elementtype* a, keytype key);

#define ARRAY_TYPEDECLARE_SIMPLE(elementtype) ARRAY_TYPEDECLARE(elementtype)
#define ARRAY_TYPEDECLARE_NATIVE(elementtype) ARRAY_TYPEDECLARE(elementtype)

#define ARRAY_FIRST(a) (a).data[0]
#define ARRAY_LAST(a) (a).data[(a).size-1]

/*
Defines the code for the specified array type
*/
#define ARRAY_TYPEDEFINE(elementtype, createcmd, destroycmd, copycmd, movecmd) \
	int array_##elementtype##_create(array_##elementtype* a, int initialcapacity) \
	{ \
		return array_createGeneric((Array_generic*)a, initialcapacity, sizeof(elementtype)); \
	} \
	void array_##elementtype##_destroy(array_##elementtype* a) \
	{ \
		int n = a->size; \
		elementtype *p = a->data; \
		assert(a->elementsize==0 || a->elementsize==sizeof(elementtype)); \
		while(n--) \
		{ \
			{ destroycmd; } \
			++p; \
		} \
		array_destroyGeneric((Array_generic*)a); \
	} \
	int array_##elementtype##_pushPtr(array_##elementtype* a, const elementtype* src) \
	{ \
		elementtype* dst; \
		elementtype* p; \
		assert(a->elementsize==sizeof(elementtype)); \
		dst = (elementtype*) array_pushGenericEmpty((Array_generic*)a); \
		if (dst==NULL) return FALSE; \
		p = dst; \
		{ createcmd;} \
		{ copycmd; } \
		return TRUE; \
	} \
	int array_##elementtype##_pushVal(array_##elementtype* a, elementtype val) \
	{ \
		return array_##elementtype##_pushPtr(a, &val); \
	} \
	int array_##elementtype##_pop(array_##elementtype* a, elementtype* dst) \
	{ \
		elementtype *src; \
		elementtype *p; \
		assert(a->elementsize==sizeof(elementtype)); \
		if (a->size==0) return FALSE; \
		a->size--; \
		src = &a->data[a->size]; \
		{ movecmd; } \
		p = src; \
		{ destroycmd; } \
		return TRUE; \
	} \
	int array_##elementtype##_popAndDrop(array_##elementtype* a) \
	{ \
		elementtype *p; \
		assert(a->elementsize==sizeof(elementtype)); \
		if (a->size==0) return FALSE; \
		a->size--; \
		p = &a->data[a->size]; \
		{ destroycmd; } \
		return TRUE; \
	} \
	void array_##elementtype##_clear(array_##elementtype* a) \
	{ \
		int n = a->size; \
		elementtype *p = a->data; \
		assert(a->elementsize==0 || a->elementsize==sizeof(elementtype)); \
		while(n--) \
		{ \
			{ destroycmd; } \
			++p; \
		} \
		a->size = 0; \
	} \
	void elementtype##_create(elementtype* p) \
	{ \
		createcmd; \
	} \
	void elementtype##_destroy(elementtype* p) \
	{ \
		destroycmd; \
	} \
	void elementtype##_copy(elementtype const* src, elementtype* dst) \
	{ \
		copycmd; \
	}

#define ARRAY_TYPEDEFINE_SIMPLE(elementtype) \
	ARRAY_TYPEDEFINE(elementtype, {memset(p,0,sizeof(*p));}, {memset(p, 0xCC, sizeof(*p));}, {*dst=*src;}, {*dst=*src; memset(src, 0xCC, sizeof(*src));})
#define ARRAY_TYPEDEFINE_NATIVE(elementtype) \
	ARRAY_TYPEDEFINE(elementtype, {*p=0;}, {*p=0xCC;}, {*dst=*src;}, {*dst=*src; *src=0xCC;})

#define ARRAY_TYPEDEFINE_FIND(elementtype, keytype, comparator) \
	ARRAY_TYPEDEFINE(elementtype) \
	int array_##elementtype##_find(array_##elementtype* a, keytype key) \
	{ \
		int i; \
		elementtype* p; \
		i = 0; \
		p = a->data; \
		while(i<a->size) \
		{ \
			if (comparator(p, key)==0) \
				return i; \
			++p; \
			++i; \
		} \
		return -1; \
	}

/*!
* Iterates over each element of the array, and applies "command" to every item
* \param myarray
*	Array to iterate
* \param var
*	Variable used to iterate the array, as a pointer. You can use this in
*	"command", to access the item.
* \param command
*	Code to execute for every item.
*
* Example for an array of "MyItem" type:
*	MyItem* item;
* 	ARRAY_FOREACH(itemsArray, item, {free(item);} );
*/
#define ARRAY_FOREACH(myarray, var, command) \
{ \
	int arrayTodo; \
	arrayTodo = (myarray).size; \
	var = (myarray).data; \
	while(arrayTodo--) \
	{ \
		{ command; } \
		var++; \
	} \
}


ARRAY_TYPEDECLARE_NATIVE(int)

#ifdef __cplusplus //
}
#endif


#endif
