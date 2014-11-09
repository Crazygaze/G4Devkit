#ifndef _memdetails_shared_h_
#define _memdetails_shared_h_

#include "stddef_shared.h"

// Called by the OS to initialize a process's head
void _mem_init(void* start, size_t size);
void _mem_debug(void);

void _getmemstats(size_t* totalUsed, size_t* totalFree, size_t* maxAlloc);

#ifdef DEBUG
	#define MEMDEBUG 1
#endif


//
// NOTE: Don't use these directly . Use the macros bellow (e.g: malloc)
//
#if MEMDEBUG
	void* _malloc_impl(size_t size, int line, const char* file);
	void* _realloc_impl(void* ptr, size_t size, int line, const char* file);
	void* _calloc_impl(size_t size, int line, const char* file);
	void  _free_impl(void* ptr, int line, const char* file);
#else
	void* _malloc_impl(size_t);
	void* _realloc_impl(void* ptr, size_t size);
	void* _calloc_impl(size_t);
	void  _free_impl(void* ptr);
#endif

#if MEMDEBUG
	#define malloc(size) _malloc_impl(size, __LINE__, __FILE__)
	#define realloc(ptr,size) _realloc_impl(ptr, size, __LINE__, __FILE__)
	#define calloc(size) _calloc_impl(size, __LINE__, __FILE__)
	#define free(ptr) _free_impl(ptr, __LINE__, __FILE__)
#else
	#define malloc(size) _malloc_impl(size)
	#define realloc(ptr,size) _realloc_impl(ptr, size)
	#define calloc(size) _calloc_impl(size)
	#define free(ptr) _free_impl(ptr)
#endif


#endif
