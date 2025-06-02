/*******************************************************************************
 * Dynamic array
 * It uses the heap to dynamically grow
 ******************************************************************************/

#ifndef _utils_dynamicarray_h_
#define _utils_dynamicarray_h_

#include "detail/utils_common.h"
#include <assert.h>
#include <string.h>

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
bool array_createGeneric(Array_generic* a, int initialcapacity, int elementsize);
bool array_reserveGeneric(Array_generic* a, int newcapacity);
void array_destroyGeneric(Array_generic* a);
bool array_pushGeneric(Array_generic* a, const void* val);
void* array_pushGenericEmpty(Array_generic* a);
bool array_popGeneric(Array_generic* a, void* dest);

// #RVF: Write a test for this
bool array_removeAtGeneric(Array_generic* a, unsigned index);

/*!
 * Declares the type "array of type T"
 * For example, if you wish to use arrays of type Foo, put this in a header:
 *		ARRAY_TYPEDECLARE(Foo)
 *
 * This declares all the necessary functions to manipulate an array of type T.
 */
#define ARRAY_TYPEDECLARE(T)                                       \
	typedef struct Array_##T {                                     \
		int size;                                                  \
		int capacity;                                              \
		T* data;                                                   \
		int elementsize;                                           \
	} Array_##T;                                                   \
	bool array_##T##_create(Array_##T* a, int initialcapacity);    \
	void array_##T##_destroy(Array_##T* a);                        \
	bool array_##T##_pushPtr(Array_##T* a, const T* val);          \
	bool array_##T##_pushVal(Array_##T* a, T val);                 \
	bool array_##T##_pop(Array_##T* a, T* dst);                    \
	bool array_##T##_popAndDrop(Array_##T* a);                     \
	void array_##T##_clear(Array_##T* a);                          \
	inline bool array_##T##_removeAt(Array_##T* a, unsigned index) \
	{                                                              \
		return array_removeAtGeneric((Array_generic*)a, index);    \
	}                                                              \
	void T##_create(T* p);                                         \
	void T##_destroy(T* p);                                        \
	void T##_copy(T const* src, T* dest);

/*!
 * Can be used after ARRAY_TYPEDECLARE, to declare a find function.
 */
#define ARRAY_DECLARE_FIND(T, KeyType)                     \
	int array_##T##_find(const Array_##T* a, KeyType key);
	
/*
The list of functions created are:

- bool array_T_create(Array_T* a, int initialcapacity;
Initializes an array with the specified capacity

- void array_T_destroy(Array_T* a);
Destroys the specified array, releasing any resources in the process.

- bool array_T_pushPtr(Array_T* a, const T* val);
Pushes a new element to the end of the array

- bool array_T_pushVal(Array_T* a, T val);
Pushes a new element to the end of the array, by value

- int array_T_find(const Array_T* a, KeyType key);
Searches the array for an element with the given key. Returns the element's
index, or -1 if not found

- bool array_T_pop(Array_T* a, T* dst);
Pops the last element of the array and moves it into `dst`. It returns true
if an element was popped, or false if the array was empty.

- bool array_T_popAndDrop(Array_T* a);
Pops the last element of the array and drops it

- void array_T_clear(Array_T* a);
Clears the array, calling the destroyExpr on every element. It leaves the 
array's capacity intact.
*/

#define ARRAY_TYPEDECLARE_SIMPLE(elementtype) ARRAY_TYPEDECLARE(elementtype)
#define ARRAY_TYPEDECLARE_NATIVE(elementtype) ARRAY_TYPEDECLARE(elementtype)

/*!
 * Access the first element on array
 */
#define ARRAY_FIRST(a) (a).data[0]

/*!
 * Access the last element of an array
 */
#define ARRAY_LAST(a) (a).data[(a).size-1]

/*!
 * Defines the code for the specified array type (array of type T)
 *
 * \param T
 * Element type
 *
 * \param createExpr
 * Expression to run to create a new element.
 * The expression should initialize `T* p`.
 *
 * \param destroyExpr
 * Expression to run to destroy an element.
 * The expression should destroy `T* p`.
 *
 * \param copyExpr
 * Expression to run to copy one element to another.
 * The expression should copy `const T* src` to `T* dst`.
 *
 * \parm moveExpr
 * Expression to run to move one element to another.
 * The exression should  move `T* src` into `T* dst)`.
 */
#define ARRAY_TYPEDEFINE(T, createExpr, destroyExpr, copyExpr, moveExpr)           \
	bool array_##T##_create(Array_##T* a, int initialcapacity)                     \
	{                                                                              \
		return array_createGeneric((Array_generic*)a, initialcapacity, sizeof(T)); \
	}                                                                              \
	void array_##T##_destroy(Array_##T* a)                                         \
	{                                                                              \
		int n = a->size;                                                           \
		T *p = a->data;                                                            \
		assert(a->elementsize==0 || a->elementsize==sizeof(T));                    \
		while(n--)                                                                 \
		{                                                                          \
			{ destroyExpr; }                                                       \
			++p;                                                                   \
		}                                                                          \
		array_destroyGeneric((Array_generic*)a);                                   \
	}                                                                              \
	bool array_##T##_pushPtr(Array_##T* a, const T* src)                           \
	{                                                                              \
		T* dst;                                                                    \
		T* p;                                                                      \
		assert(a->elementsize==sizeof(T));                                         \
		dst = (T*) array_pushGenericEmpty((Array_generic*)a);                      \
		if (dst==NULL) return false;                                               \
		p = dst;                                                                   \
		{ createExpr;}                                                             \
		{ copyExpr; }                                                              \
		return true;                                                               \
	}                                                                              \
	bool array_##T##_pushVal(Array_##T* a, T val)                                  \
	{                                                                              \
		return array_##T##_pushPtr(a, &val);                                       \
	}                                                                              \
	bool array_##T##_pop(Array_##T* a, T* dst)                                     \
	{                                                                              \
		T *src;                                                                    \
		T *p;                                                                      \
		assert(a->elementsize==sizeof(T));                                         \
		if (a->size==0) return false;                                              \
		a->size--;                                                                 \
		src = &a->data[a->size];                                                   \
		{ moveExpr; }                                                              \
		p = src;                                                                   \
		{ destroyExpr; }                                                           \
		return true;                                                               \
	}                                                                              \
	bool array_##T##_popAndDrop(Array_##T* a)                                      \
	{                                                                              \
		T *p;                                                                      \
		assert(a->elementsize==sizeof(T));                                         \
		if (a->size==0) return false;                                              \
		a->size--;                                                                 \
		p = &a->data[a->size];                                                     \
		{ destroyExpr; }                                                           \
		return true;                                                               \
	}                                                                              \
	void array_##T##_clear(Array_##T* a)                                           \
	{                                                                              \
		int n = a->size;                                                           \
		T *p = a->data;                                                            \
		assert(a->elementsize==0 || a->elementsize==sizeof(T));                    \
		while(n--)                                                                 \
		{                                                                          \
			{ destroyExpr; }                                                       \
			++p;                                                                   \
		}                                                                          \
		a->size = 0;                                                               \
	}                                                                              \
	void T##_create(T* p)                                                          \
	{                                                                              \
		createExpr;                                                                \
	}                                                                              \
	void T##_destroy(T* p)                                                         \
	{                                                                              \
		destroyExpr;                                                               \
	}                                                                              \
	void T##_copy(T const* src, T* dst)                                            \
	{                                                                              \
		copyExpr;                                                                  \
	}


#define ARRAY_TYPEDEFINE_SIMPLE(elementtype) \
	ARRAY_TYPEDEFINE(elementtype, {memset(p,0,sizeof(*p));}, {memset(p, 0xCC, sizeof(*p));}, {*dst=*src;}, {*dst=*src; memset(src, 0xCC, sizeof(*src));})
#define ARRAY_TYPEDEFINE_NATIVE(elementtype) \
	ARRAY_TYPEDEFINE(elementtype, {*p=0;}, {*p=0xCC;}, {*dst=*src;}, {*dst=*src; *src=0xCC;})

/*!
 * Defines the array_T_find function.
 *
 * \param T
 * The element type
 *
 * \param KeyType
 * Type of the key
 *
 * \param compexpr
 * Comparator expression.
 * The expression should use the knowledge that there is a `const T* p`
 * and a `KeyType key` variable in the current scope, and evaluate to true or
 * false.
 */
#define ARRAY_DEFINE_FIND(T, KeyType, compExpr)           \
	int array_##T##_find(const Array_##T* a, KeyType key) \
	{                                                     \
		int i;                                            \
		const T* p;                                       \
		i = 0;                                            \
		p = a->data;                                      \
		while(i<a->size)                                  \
		{                                                 \
			if (compExpr(p, key))                         \
				return i;                                 \
			++p;                                          \
			++i;                                          \
		}                                                 \
		return -1;                                        \
	}

/*!
 * Iterates over each element of the array, the same way as a for loop.
 *
 * \param array Array to iterate
 * \param Type Element type
 * \param itVar variable name to use in the loop to access the element
 *
 * Example for an array `items` of type `Foo`:
 *
 * ```
 * 	ARRAY_FOREACH(items, Foo*, ele)	{
 *		useEle(ele);
 *	}
 * ```
 */
#define ARRAY_FOREACH(array, Type, itVar)   \
	for(Type itVar = (array).data, *_end = itVar + a.size; itVar != _end; ++itVar)
	

ARRAY_TYPEDECLARE_NATIVE(int)

#endif
