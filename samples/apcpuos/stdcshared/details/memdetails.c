#include "memdetails.h"
#include "assert_shared.h"
#include "string_shared.h"
#include "extern/tlsf/tlsf.h"


#if MEMDEBUG
	typedef struct allocdbginfo_st {
		int line;
		const char* file;
	} allocdbginfo_st;
	#define BOUNDSIZE 0
	#define USER_TO_INTERNAL(ptr) ((ptr) ? ((uint8_t*)(ptr) - (sizeof(allocdbginfo_st) + BOUNDSIZE)) : NULL)
	#define INTERNAL_TO_USER(ptr) ((ptr) ? ((uint8_t*)ptr + (sizeof(allocdbginfo_st) + BOUNDSIZE)) : NULL)
	#define SIZE_USER_TO_INTERNAL(size) ((size) ? (size + sizeof(allocdbginfo_st) + BOUNDSIZE*2) : 0)
	#define SIZE_INTERNAL_TO_USER(size) ((size) ? (size - sizeof(allocdbginfo_st) - BOUNDSIZE*2) : 0)
	#define LOGMEM LOG
#else
	#define USER_TO_INTERNAL(ptr) (ptr)
	#define INTERNAL_TO_USER(ptr) (ptr)
	#define SIZE_USER_TO_INTERNAL(size) (size)
	#define SIZE_INTERNAL_TO_USER(size) (size)
	#define LOGMEM(...) (void)(0)
#endif


tlsf_t _mem_tdata;

void _mem_init(void* start, size_t size)
{
	assert( size>=tlsf_size() );

	_mem_tdata = tlsf_create_with_pool(start, size);

	LOGMEM("  tlsf_size() = %u", tlsf_size() );
	LOGMEM("  tlsf_align_size() = %u", tlsf_align_size() );
	LOGMEM("  tlsf_block_size_min() = %u", tlsf_block_size_min() );
	LOGMEM("  tlsf_block_size_max() = %u", tlsf_block_size_max() );
	LOGMEM("  tlsf_pool_overhead() = %u", tlsf_pool_overhead());
	LOGMEM("  tlsf_alloc_overhead() = %u", tlsf_alloc_overhead());
	LOGMEM("  OS debug alloc overhead = %u", SIZE_USER_TO_INTERNAL(1)-1 );
	LOGMEM("  Total alloc overhead = %u", tlsf_alloc_overhead() + SIZE_USER_TO_INTERNAL(1)-1 );
	if (tlsf_block_size_max()<=size) {
		LOGMEM("  WARNING: maximum allowed block size not big enough for available RAM.");
	}

}


void* _malloc_impl( size_t size
#if MEMDEBUG
	, int line, const char* file
#endif
)
{

	uint8_t* ptr = tlsf_malloc(_mem_tdata, SIZE_USER_TO_INTERNAL(size) );

#if MEMDEBUG
	if (ptr) {
		((allocdbginfo_st*)ptr)->line = line;
		((allocdbginfo_st*)ptr)->file = file;
	} else {
		LOGMEM("Out of memory");
	}
#endif

	return INTERNAL_TO_USER(ptr);
}

void* _realloc_impl( void* oldptr, size_t size
#if MEMDEBUG
	, int line, const char* file
#endif
)
{
	uint8_t* newptr = tlsf_realloc(
				_mem_tdata,
				USER_TO_INTERNAL(oldptr),
				SIZE_USER_TO_INTERNAL(size));

#if MEMDEBUG
	if (newptr)
	{
		((allocdbginfo_st*)newptr)->line = line;
		((allocdbginfo_st*)newptr)->file = file;
	} else {
		LOGMEM("Out of memory");
	}
#endif

	return INTERNAL_TO_USER(newptr);
}


void* _calloc_impl( size_t size
#if MEMDEBUG
, int line, const char* file
#endif
)
{
	void* p = _malloc_impl(size
#if MEMDEBUG	
			, line, file
#endif
	);
	
	if (p) memset(p, 0, size);
	return p;
}


void _free_impl(void* ptr
#if MEMDEBUG
	, int line, const char* file
#endif
)
{
	tlsf_free(_mem_tdata, USER_TO_INTERNAL(ptr));
}

static void _mem_walker(void* ptr, size_t size, int used, void* user)
{
	if (used)
	{
#if MEMDEBUG
		allocdbginfo_st* dbg = (allocdbginfo_st*)ptr;
		size_t usersize = SIZE_INTERNAL_TO_USER(size);
		uint8_t* userptr = INTERNAL_TO_USER(ptr);
		LOGMEM(
			"\t0x%X:%d bytes (user:0x%X:%d bytes), %s:%d\n", ptr,
			(unsigned int)size, userptr, (unsigned int)usersize,
			dbg->file, dbg->line);		
#else
		LOGMEM(
			"\t0x%X:%d bytes", ptr, (unsigned int)size);		
#endif
	}
	else
	{
		LOGMEM("\t0x%X size=%d, FREE \n", ptr, (unsigned int)size);
	}
}

void _mem_debug(void)
{
	assert( tlsf_check(_mem_tdata)==0 );
	LOGMEM("MEMDEBUG START");
	tlsf_walk_pool( tlsf_get_pool(_mem_tdata), _mem_walker, NULL);
	LOGMEM("MEMDEBUG END");
}

typedef struct MemStats
{
	size_t used;
	size_t free;
	size_t maxalloc;
} MemStats;

static void _mem_stats_walker(void* ptr, size_t size, int used, void* cookie )
{
	MemStats* stats = cookie;
	
	if (used) {
		stats->used += size;
	} else {
		stats->free += size;
		if (size > stats->maxalloc)
			stats->maxalloc = size;		
	}
}

void _getmemstats(size_t* totalUsed, size_t* totalFree, size_t* maxAlloc)
{
	MemStats stats;
	memset(&stats, 0, sizeof(stats));
	
	tlsf_walk_pool( tlsf_get_pool(_mem_tdata), _mem_stats_walker, &stats);
	if (totalUsed)
		*totalUsed = stats.used;
	if (totalFree)
		*totalFree = stats.free;
	if (maxAlloc)
		*maxAlloc = stats.maxalloc - (SIZE_USER_TO_INTERNAL(1)-1);
}
