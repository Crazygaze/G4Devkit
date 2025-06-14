#include "stdc_internal.h"
#include "memdetails.h"
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "hwcrt0.h"
#include "amalloc.h"

#include "../stdc_init.h"

void _mem_init(void* start, size_t size, bool (*brkFunc)(void*))
{
	assert(start && size);

	AMAInitParams params;
	params.brkFunc = brkFunc;
	params.heapStart = start;
	params.heapSize = size;
	ama_init(&params);
}

void* _malloc_impl( size_t size
#if _STDC_MEMDEBUG
	, int line, const char* file
#endif
)
{
	uint8_t* ptr = ama_malloc(
		size
	#if _STDC_MEMDEBUG
		, file, line
	#endif
	);
	
#if _STDC_MEMDEBUG
	if (!ptr) {
		LOG_LOG("Out of memory");
	}
#endif

	return ptr;
}

void* _realloc_impl( void* oldptr, size_t size
#if _STDC_MEMDEBUG
	, int line, const char* file
#endif
)
{
	uint8_t* newptr = ama_realloc(oldptr, size
#if _STDC_MEMDEBUG
	, file, line
#endif
	);

#if _STDC_MEMDEBUG
	if (!newptr) {
		LOG_LOG("Out of memory");
	}
#endif

	return newptr;
}


void* _calloc_impl( size_t size
#if _STDC_MEMDEBUG
, int line, const char* file
#endif
)
{
	void* p = _malloc_impl(size
#if _STDC_MEMDEBUG
			, line, file
#endif
	);
	
	if (p) memset(p, 0, size);
	return p;
}


void _free_impl(void* ptr
#if _STDC_MEMDEBUG
	, int /*line*/, const char* /*file*/
#endif
)
{
	ama_free(ptr);
}

void _mem_debug(void)
{
	ama_dump();
}

void _mem_getstats(size_t* totalUsed, size_t* totalFree, size_t* maxAlloc)
{
	AMAStats stats;
	ama_getStats(&stats);
	*totalUsed = stats.allocatedBytes;
	*totalFree = stats.totalFree;
	*maxAlloc = stats.largestFreeBlock;
}
