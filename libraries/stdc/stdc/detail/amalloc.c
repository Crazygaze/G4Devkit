#include "detail/stdc_internal.h"
#include "../stdc_init.h"

#include "amalloc.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////
//                            CONFIGURATION                                   //
//
// To make the firmware smaller, some of the checks are just asserts as
// opposed to returning
////////////////////////////////////////////////////////////////////////////////

#ifndef NULL
	#define NULL 0L
#endif

#define ama_printf LOG_LOG
#define ama_assert assert
#define ama_verify(expr) if (!(expr)) ama_assert(0)
#define PAGE_SIZE 4096

// This can be set to 0 or 4. If 0, then it means no guarding values are used
// If 4, guarding values are using, which is useful for debugging heap
// corruption issues
#if AMA_GUARD
	#define GUARD_SIZE 4
#else
	#define GUARD_SIZE 0
#endif

#if GUARD_SIZE
	#define GUARD_VALUE 0xFDFDFDFD
#endif

/*!
 * Alignment requirements for:
 * - Pointers passed to the API, such as the head start.
 * - The pointers given back to the user
 * - The AMANode structs
 */
#define ALIGNMENT 4

/*!
 * The minimum capacity for free blocks created when resizing a used node.
 * In practice, what this means is that when we grab a node and set it to used,
 * If the lefover capacity is big enough, it splits it into a free node.
 */
#define FREENODE_MIN_SIZE 4

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#ifndef min
	#define min(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef max
	#define max(a,b) (((a)>(b))?(a):(b))
#endif

/*!
 * Aligns `val` on `alignment`, where `alignment` is a power of two
 *
 * E.g: `ALIGN(5,4)` returns 8
 */
#define ALIGN(val, alignment) (((val) + ((alignment) - 1)) & ~((alignment) - 1))

/*!
 * Checks if `val` is aligned on `alignment` bytes, where `alignment is a power
 * of two
 */
#define ISALIGNED(val, alignment) (((val) & ((alignment) - 1)) == 0)

typedef struct AMANode {
	struct AMANode* prev;
	struct AMANode* next;

	/*!
	 * Used size in bytes, if the node is in use. 0 if free
	 */
	u32 size;

#if AMA_TRACK
	const char* file;
	int line;
#endif

#if GUARD_SIZE
	// The guard value for BEFORE the user memory
	// The one for AFTER is put right at the end of the user's data
	u32 guard1;
#endif
		
} AMANode;

#define NODE_SIZE sizeof(AMANode)


typedef struct AMAState {
	AMABrkFunc brkFunc;
	u8* heapStart;

	// Current program break
	u8* brk;

	// Initialized to currBrk
	// This is used so we never go below this limit when relinquishing pages
	// to the OS
	u8* minbrk;

	// This gets increased as we ask for more pages from the OS
	// u32 heapSize;

	AMANode* first;
	// Searches for free nodes start from this node
	AMANode* cursor_;
} AMAState;

static AMAState ama;

#if AMA_TRACK

/*!
 * Given a filename path, it returns just the file name part
 * This is used to shorten what is is displayed in ama_dump
 */
static const char* ama_getFilename(const char* file)
{
	const char* a = strrchr(file, '\\');
	const char* b = strrchr(file, '/');
	const char* c = a > b ? a : b;
	return c ? c+1 : file;
}

#endif

/*!
 * Convert a user pointer to it's node
 */
static inline AMANode* ama_ptrToNode(void* ptr)
{
	ama_assert(ISALIGNED((u32)ptr, ALIGNMENT));
	AMANode* node = (AMANode*)ptr-1;
	return node;
}

/*!
 * Converts a node into the user pointer
 */
static inline void* ama_nodeToPtr(AMANode* node)
{
	return node + 1;
}

/*!
 * Checks if the specified node is the last node in the linked list
 */
static inline bool ama_isLast(const AMANode* node)
{
	// If next wraps around (to lower address), then we are the last node
	return node >= node->next;
}

#define ama_last(void) (ama.first->prev)


/*!
 * Returns the capacity of the specified node
 */
static inline u32 ama_getCapacity(AMANode* node)
{
	// NOTE: If it's the last node in the list, then we calculate the capacity based on the heapsize
	if (ama_isLast(node))
		return (u32)(ama.brk) - (u32)(node+1);
	else
		return ((u32)node->next - (u32)(node+1)); // If it's not the last node.
}

/*!
 * Inserts `newnode` after `node`
 */
static void ama_addNodeAfter(AMANode* node, AMANode* newnode)
{
	newnode->prev = node;
	newnode->next = node->next;
	if (node->next)
		node->next->prev = newnode;
	node->next = newnode;
}

/*!
 * Removes a node from the linked list
 */
static void ama_removeNode(AMANode* node)
{
	if (node->prev)
		node->prev->next = node->next;
	if (node->next)
		node->next->prev = node->prev;
	node->next = NULL;
	node->prev = NULL;
}

/*!
* Iterates over each element of the list, the same way as a for loop
*
* \param Type Element type
* \param startnode
*	Where to start iterating. The list is circular, so it will iterate until it
*	gets back to this item
* \param itVar variable to use in the loop to acces the element 
*
* Example for a linked list of `Node` items, where `Node` has a `name` field
*
*   Node* root = <something>;
*
*	LINKEDLIST_FOREACH(root, Node*, it) {
*		printf("%s\n", it->name);
*	};
*/
#define LINKEDLIST_FOREACH(startnode, Type, itVar) \
	for(Type _begin = startnode, *itVar = (_begin && _begin->next) ? _begin : NULL, *_end = NULL; itVar != _end; _end = _begin, itVar = itVar->next)

/*!
 * Finds the first free node that fits the required size.
 * It does not look for the best fit.
 *
 * \return The node, or NULL if no free nodes found
 */
static AMANode* ama_findFree(AMANode* start, u32 minSize)
{
	LINKEDLIST_FOREACH(start, AMANode*, it) {
		if (it->size == 0 && ama_getCapacity(it) >= minSize) {
			return it;
		}
	}

	return NULL;
}

/*!
 * Tries to merge a node with the next one, if the next one is free
 */
static bool ama_tryCoalesce(AMANode* node)
{
	if (node < node->next && node->next->size == 0) {
		ama_removeNode(node->next);
		return true;
	} else {
		return false;
	}
}

#if GUARD_SIZE
static inline bool ama_checkFirstGuard(AMANode* node)
{
	if (node->guard1 == GUARD_VALUE)
		return true;
	else
		return false;
}
static inline bool ama_checkGuards(AMANode* node)
{
	if (
		(node->guard1 == GUARD_VALUE) &&
		(*((u32*)((u8*)ama_nodeToPtr(node) + node->size)) == GUARD_VALUE)
		)
		return true;
	else
		return false;
}

static inline void ama_setFirstGuard(AMANode* node)
{
	node->guard1 = GUARD_VALUE;
}
static inline void ama_setGuards(AMANode* node)
{
	node->guard1 = GUARD_VALUE;
	*((u32*)((u8*)ama_nodeToPtr(node) + node->size)) = GUARD_VALUE;
}

#else
#define ama_checkGuards(node) true
#define ama_checkFirstGuard(node) true
#define ama_setFirstGuard(node)
#define ama_setGuards(node)
#endif

/*!
 * Sets a node's size and leaves it with the specified capacity.
 * If the leftover is big enough, it creates a new free node.
 */
static void ama_resizeAsUsed(AMANode* node, u32 newSize, u32 newCapacity)
{
	ama_assert(newSize <= newCapacity && ISALIGNED(newCapacity, ALIGNMENT));
	ama_assert(newCapacity <= ama_getCapacity(node));
	node->size = newSize;
	ama_setGuards(node);
	u32 leftOver = ama_getCapacity(node) - newCapacity;

	// If there is enough leftover, create a free node for that
	if (leftOver >= (NODE_SIZE + FREENODE_MIN_SIZE + GUARD_SIZE)) {
		AMANode* freeNode = (AMANode*)((u8*)node + NODE_SIZE + newCapacity);
		freeNode->size = 0;
		ama_setFirstGuard(freeNode);
		ama_addNodeAfter(node, freeNode);
		
		// Try to coalesce the new free node with the node after
		ama_tryCoalesce(freeNode);
	}

}


////////////////////////////////////////////////////////////////////////////////
//                             PUBLIC API                                     //
////////////////////////////////////////////////////////////////////////////////

void ama_init(const AMAInitParams* params)
{
	// Check if already initialized
	ama_assert(ama.heapStart == NULL);
	ama_assert(params->heapSize >= (NODE_SIZE + GUARD_SIZE + FREENODE_MIN_SIZE));

	ama.brkFunc = params->brkFunc;

	// Adjust the heap start and size to satisfy the alignment requirements
	ama.heapStart = (u8*)ALIGN((u32)params->heapStart, ALIGNMENT);
	u32 adjustment = (u8*)ama.heapStart - (u8*)params->heapStart;
	ama.brk = ama.heapStart + params->heapSize - adjustment;
	ama.brk = (u8*)((u32)ama.brk & ~(ALIGNMENT-1)); // Align down
	ama.minbrk = ama.brk;


	//
	// Setup the first node
	//
	
	ama.first = (AMANode*)ama.heapStart;
	ama.cursor_ = ama.first;

	// Link to itself
	ama.first->prev = ama.first;
	ama.first->next = ama.first;
	// Setup the first free block, which encompasses the entire heap
	ama.first->size = 0;
#if GUARD_SIZE
	ama.first->guard1 = GUARD_VALUE;
#endif
}


void* ama_malloc(
	u32 size
	#if AMA_TRACK
	, const char* file, int line
	#endif
)
{
	ama_assert(ama.heapStart);

	if (size == 0)
		return NULL;

	u32 capacity = ALIGN(size+GUARD_SIZE, ALIGNMENT);
	AMANode* node = ama_findFree(ama.cursor_, capacity);

	// If no free chunk big enough was found then try to ask the OS for more.
	if (!node) {
		if (ama.brkFunc) {
			bool lastIsFree = ama_last()->size == 0 ? true : false;

			u32 requestSize;

			// If the last node is free, then we can request a smaller size
			if (lastIsFree)
				requestSize = (size - ama_getCapacity(ama_last())) + GUARD_SIZE;
			else 
				requestSize = NODE_SIZE + size + GUARD_SIZE;

			requestSize = ALIGN(requestSize, ALIGNMENT);

			// Calculated the desired program break so it is page aligned,
			// since the OS will work in terms of pages, and thus there is
			// no point in keep doing a bunch of system calls.
			u8* wantedBrk = (u8*)ALIGN((u32)ama.brk + requestSize, PAGE_SIZE);

			if (!ama.brkFunc(wantedBrk))
				return NULL;

			if (lastIsFree) {
				ama.brk = wantedBrk;
				// If the last node is free, then we don't need to change
				// anything. It picks up the extra capacity
				assert(ama_getCapacity(ama_last()) > capacity); 
			} else {
				// We need to insert a node to track the extra capacity
				AMANode* newNode = (AMANode*)(ama.brk);
				ama.brk = wantedBrk;
				// Insert as free node at the end
				newNode->size = 0;
				ama_addNodeAfter(ama_last(), newNode);
			}
		} else {
			return NULL;
		}

		// At this point, we know that the node to use must be the last one.
		node = ama_last();
	}

	ama_resizeAsUsed(node, size, capacity);
#if AMA_TRACK
	node->file = file;
	node->line = line;
#endif

	ama.cursor_ = node->next;
	
	void* ptr = ama_nodeToPtr(node);
#if _DEBUG
	// https://en.wikipedia.org/wiki/Magic_number_(programming)#Debug_values
	// 0xCDCDCDCD :Used to mark uninitialized heap memory
	memset(ptr, 0xCD, size);
#endif

	return ptr;
}

/*!
 * Frees the specified node from the linked list, coalescing with both
 * previous and next if either are free node.
 */
static void ama_freeImpl(AMANode* node, bool tryPageRelease)
{
	ama_assert(ama_checkGuards(node));

	// Mark the node as free
	node->size = 0;

	// Try to coalesce with next
	ama_tryCoalesce(node);

	// Try to coalesce with previous, if the previous one is free
	if (node->prev->size == 0) {
		// #TODO : Test this code path
		AMANode* prev = node->prev;
		if (ama_tryCoalesce(prev))
			node = prev;
	}
	
	ama.cursor_ = node;

	// If the last node is free, then we try to relinquish pages by 
	// contracting that node
	if (tryPageRelease && ama_last()->size == 0 && ama.brkFunc) {
		u8* newbrk = (u8*)ama_last() + NODE_SIZE + FREENODE_MIN_SIZE + GUARD_SIZE;
		newbrk = max(newbrk, ama.minbrk);

		ama_verify(ama.brkFunc(newbrk));
		ama.brk = newbrk;
	}
}

void ama_free(void* ptr)
{
	ama_assert(ama.heapStart);
	if (!ptr)
		return;
	AMANode* node = ama_ptrToNode(ptr);
	ama_assert(node->size > 0);

	// https://en.wikipedia.org/wiki/Magic_number_(programming)
	// 0xDDDDDDDD : Used to mark freed heap memory
#if _DEBUG
	memset(ptr, 0xDD, node->size);
#endif
	ama_freeImpl(node, true);
}

void* ama_calloc(
	u32 size
	#if AMA_TRACK
	, const char* file, int line
	#endif
)
{
	ama_assert(ama.heapStart);

	void* ptr = ama_malloc(
		size
		#if AMA_TRACK
		, file, line
		#endif
	);

	if (ptr)
		memset(ptr, 0, size);

	return ptr;
}

static AMANode* ama_reallocImpl(void* ptr, u32 newSize)
{
	ama_assert(ama.heapStart);

	if (ptr == NULL) {
		ptr = ama_malloc(
				newSize
				#if AMA_TRACK
				, "a.b", 0
				#endif
			);
		return ptr ? ama_ptrToNode(ptr) : NULL;
	}

	if (newSize == 0)
		return NULL;

	AMANode* node = ama_ptrToNode(ptr);
	ama_assert(node->size);
	ama_assert(ama_checkGuards(node));

	u32 currCapacity = ama_getCapacity(node);
	u32 newCapacity = ALIGN(newSize + GUARD_SIZE, ALIGNMENT);

	// CASE 1
	// If the node has enough capacity, then just reuse it
	if (newCapacity <= currCapacity) {
		ama_resizeAsUsed(node, newSize, newCapacity);
		return node;
	}

	// CASE 2
	// If the next node is free and can satisfy the request by merging, then
	// merge.
	// Note that a naive approach would be to just merge regardless, right at
	// the beginning, so that even CASE 1 would have an higher chance of
	// working, BUT that means that if realloc then fails because there is not
	// enough memory, the node could potentially be using way more capacity
	// then necessary.
	bool nextIsFree = false;
	if (node < node->next && node->next->size == 0) {
		nextIsFree = true;
		u32 nextCapacity = ama_getCapacity(node->next);
		if (currCapacity + NODE_SIZE + nextCapacity >= newCapacity) {
			ama_removeNode(node->next);
			ama_resizeAsUsed(node, newSize, newCapacity);
			return node;
		}
	}


	// CASE 3
	// If the previous node is free, lets try merging with that
	if (node > node->prev && node->prev->size == 0) {

		// Add up the capacity of the previous and current
		u32 totalCapacity = ama_getCapacity(node->prev) + NODE_SIZE + currCapacity;
		// If the next node is free (evne though it wasn't useful for CASE 2),
		// add up its capacity, since it can help us satisfy the request
		if (nextIsFree)
			totalCapacity += NODE_SIZE + ama_getCapacity(node->next);

		if (newCapacity <= totalCapacity) {
			AMANode* newNode = node->prev;
			u32 currSize = node->size;
			// This releases the node (but doesn't change the user memory).
			// It needs to be done BEFORE the memmove, because the memove
			// can end up writing over the node struct
			ama_freeImpl(node, false);

			// Do the move BEFORE the node resize, since the resize can in
			// theory end up changing memory that used to be the user's
			// memory.
			memmove(ama_nodeToPtr(newNode), ptr, currSize);

			ama_resizeAsUsed(newNode, newSize, newCapacity);
			return newNode;
		}
	}

	//
	// CASE 4
	// Just do another malloc
	void* newPtr = ama_malloc(
			newSize
			#if AMA_TRACK
			, "a.b", 0
			#endif
		);
	if (newPtr) {
		u32 currSize = node->size;
		// This releases the node (but doesn't change the user memory).
		// It needs to be done BEFORE the memmove, because the memove
		// can end up writing over the node struct
		ama_freeImpl(node, false);

		memmove(newPtr, ptr, currSize);
		return ama_ptrToNode(newPtr);
	}

	return NULL;
}

void* ama_realloc(
	void* ptr, u32 newSize
#if AMA_TRACK
	, const char* file, int line
#endif
)
{
	AMANode* node = ama_reallocImpl(ptr, newSize);
	if (!node)
		return NULL;
	
#if AMA_TRACK
	node->file = file;
	node->line = line;
#endif

	ama.cursor_ = node->next;
	return ama_nodeToPtr(node); 
}

// Used internally only
typedef struct AMAStats2
{
	u32 numNodes;
	
	// Used to check if something is wrong with the heap
	// By the end, this needs to match the heap size
	u32 totalCapacity;
} AMAStats2;

static void ama_walk(AMAStats* stats, AMAStats2* stats2, bool doLog)
{
	memset(stats, 0, sizeof(*stats));
	memset(stats2, 0, sizeof(*stats2));

	LINKEDLIST_FOREACH(ama.first, AMANode*, it) {
		u32 capacity = ama_getCapacity(it) - GUARD_SIZE;
		if (it->size == 0) {
			bool ok = ama_checkFirstGuard(it);
			if (doLog) {
				ama_printf("    %p:FREE: capacity=%10u%s\n", it, capacity, ok ? "" : " - CORRUPT");
			}
			stats->totalFree += capacity;
			if (capacity > stats->largestFreeBlock)
				stats->largestFreeBlock = capacity;
		} else {
			bool ok = ama_checkGuards(it);
			if (doLog) {
				#if AMA_TRACK
				ama_printf("    %p:USED: capacity=%10u, size=%10u, %s:%d%s\n",
					it, capacity, it->size,
					ama_getFilename(it->file), it->line, ok ? "" : " - CORRUPT");
				#else
				ama_printf("    %p:USED: capacity=%10u, size=%10u%s\n", it, capacity, it->size, ok ? "" : " - CORRUPT");
				#endif
			}
			stats->allocatedBytes += it->size;
			stats->numAllocs++;
		}

		stats2->numNodes++;
		stats2->totalCapacity += NODE_SIZE + capacity;
	}

}

void ama_dump(void)
{
	ama_printf("-- AMA DUMP BEGIN --\n");

	AMAStats stats;
	AMAStats2 stats2;
	ama_walk(&stats, &stats2, true);
	
	u32 heapSize = ama.brk - ama.heapStart;
	ama_printf("    Heap size       =%10u\n", heapSize);
	ama_printf("    Overhead p/alloc=%10u\n", NODE_SIZE + GUARD_SIZE);
	ama_printf("    Allocated bytes =%10u (%u allocs)\n", stats.allocatedBytes, stats.numAllocs);
	ama_printf("    TotalFree       =%10u\n", stats.totalFree);
	ama_printf("    LargestFreeBlock=%10u\n", stats.largestFreeBlock);
	if ((stats2.totalCapacity + GUARD_SIZE * stats2.numNodes) != heapSize) {
		ama_printf(
			"    Corruption detected. Measured capacity is %u, and should be %u",
			stats2.totalCapacity, heapSize);
	}
		
	ama_printf("-- AMA DUMP END--\n");
}

void ama_getStats(AMAStats* outStats)
{
	AMAStats2 stats2;
	ama_walk(outStats, &stats2, false);
}

