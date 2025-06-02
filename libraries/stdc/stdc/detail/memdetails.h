#ifndef _stdc_memdetails_h_
#define _stdc_memdetails_h_

#include <stddef.h>

/*!
* Decide if we should enable memory debugging or not.
* When memory debugging is enabled the following happens:
* - Overhead per allocation increases
* - A memory allocation records the line a file where the allocation occurs.
* - _mem_debug logs information (through the LOG macro)
*/
#ifdef DEBUG
	#define _STDC_MEMDEBUG 1
#else
	#define _STDC_MEMDEBUG 0
#endif


/*!
* Initializes the heap
*/
void _mem_init(void* start, size_t size);

/*!
* Performs memory allocator integrity checks and logs information
*/
void _mem_debug(void);

/*!
* Retrieves some memory stats
* \param totalUsed If not NULL, it will contain the total memory allocated
* \param totalFree If not NULL, it will contain the total free memory
* \param maxAlloc If not NULL, it will contain the biggest single allocation
*   that can be performed. Note that totalFree can be higher than maxAlloc, due
*   to memory fragmentation.
*/
void _mem_getstats(size_t* totalUsed, size_t* totalFree, size_t* maxAlloc);

//
// NOTE: Don't use these directly . Use the macros bellow (e.g: malloc)
//
#if _STDC_MEMDEBUG
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

#if _STDC_MEMDEBUG
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
