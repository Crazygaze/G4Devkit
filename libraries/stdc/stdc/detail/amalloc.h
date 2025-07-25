#ifndef _amalloc_h
#define _amalloc_h

#include "detail/config.h"
#include <stddef.h>

#if 0
	typedef int bool;
	#define true 1
	#define false 0
	typedef unsigned int u32;
	typedef unsigned char u8;
#else
	#include "hwcrt0_stdc.h"
#endif

// If not defined, then assume default depending on build type
#ifndef AMA_TRACK
	#ifdef _DEBUG
		#define AMA_TRACK 1
	#else
		#define AMA_TRACK 0
	#endif
#endif

// If not defined, then assume default depending on build type
#ifndef AMA_GUARD
	#ifdef _DEBUG
		#define AMA_GUARD 1
	#else
		#define AMA_GUARD 0
	#endif
#endif


/*
 -- Generic allocator for using with the APCPU-OS --

NOTE: The `ama_` prefix stands for "(A)pcu (M)emory (A)lloc".

It's not the fastest or the smartest. The main objective is to have something
that can be used by both the kernel and user space by using the shared-data
sections.

It's implemented as a simple circular linked list of nodes, and uses the
first-fit strategy.
It does some optimizations such as coalescing free nodes.

Overhead per allocation is acceptable (imo) and there is no other bookeeping,
other than a small struct with some state such as heapsize and and callbacks.
*/

/*!
 * Signature for our equivalent of the `brk` function.
 * See  https://en.wikipedia.org/wiki/Sbrk
 *
 * This is only used if the library is to be used in user space mod.
 *
 * \param addr New value for the upper limit of the heap.
 * \return
 *	true on success, false on error.
 *	If `true` is returned, then the upper heap limit for the process was
 *	adjusted.
 */
typedef bool (*AMABrkFunc)(void* addr);

/*!
 * Library initialization parameters
 */
typedef struct AMAInitParams {

	/*!
	 * The function to use to set the upper limit of the heap.
	 * This is used to ask the OS for more pages when necessary, and to
	 * relinquish pages (at the top) when not needed.
	 *
	 * This can be NULL, in which case the library will not ask for new pages
	 * and will use only the space specified by `heapStart` and `heapSize`.
	 * Specifying NULL makes the library usable for kernel mode.
	 */
	AMABrkFunc brkFunc;

	/*!
	 * Where the heap starts.
	 * Internally, this might get aligned to match the library requirements.
	 * This can't be nulled.
	 */
	void* heapStart;

	/*!
	 * Initial size of the heap.
	 *
	 * This will be adjusted downwards internally, to meet any alignment
	 * requirements.
	 * The library assumes that this is already valid memory, and will not
	 * ask the OS for these pages.
	 *
	 * When an allocation request can't be satisfied, the library will call the
	 * `brkFunc` (if specified) to increment the program break as required.
	 *
	 * This can't be `0`, because when initializing some heap is necessary to
	 * setup the first node.
	 */
	u32 heapSize;

} AMAInitParams;


/*!
 * Initializes the library
 */
void ama_init(const AMAInitParams* params);

#if AMA_TRACK
void* ama_malloc(u32 size, const char* file, int line);
void* ama_calloc(u32 size, const char* file, int line);
void* ama_realloc(void* ptr, u32 size, const char* file, int line);
void ama_free(void *ptr);
#else
void* ama_malloc(u32 size);
void* ama_calloc(u32 size);
void* ama_realloc(void* ptr, u32 size);
void ama_free(void *ptr);
#endif

/*!
 * Memory stats that can be retrieved with `ama_getStats`
 */
typedef struct AMAStats {
	u32 numAllocs;
	u32 totalFree;
	u32 largestFreeBlock;
	u32 allocatedBytes;

} AMAStats;

/*!
 * Logs the heap nodes
 */
void ama_dump(void);

/*!
 * Gets mem stats, such as total allocated memory, etc
 */
void ama_getStats(AMAStats* outStats);


#endif

