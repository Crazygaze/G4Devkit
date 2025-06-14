#ifndef _appsdk_process_shared_h
#define _appsdk_process_shared_h

#include "syscalls_shared.h"


// #TODO : Implement the app_closeHandle function

/*!
 * Operating system handle.
 * An HANDLE can represent a thread, process, mutex or any other system resource.
 * Several functions return handles when creating a system resource.
 * To destroy the resource, the function app_closeHandle can be used.
 */
struct HANDLE_t { u32 unused; };
typedef struct HANDLE_t* HANDLE;
	

/*!
 * An invalid handle.
 * This can be returns by functions that generate an handle, in which case it
 * indicates a failure.
 */
#define INVALID_HANDLE 0L

/*!
 * Entry point for a new application
 */
typedef int (*PrcEntryFunc)(void* cookie);

/*!
 * Thread entry function
 */
typedef void (*ThreadEntryFunc)(void* cookie);

/*!
 * For internal use only. No need to use it directly.
 */
typedef void (*ThreadEntryFuncWrapper)(ThreadEntryFunc func, void* cookie);

/*!
 * Called by the OS as the entry point to a process.
 * You should not call this directly.
 *
 * \param func Actual app entry function
 * \param cookie Passed to the entry function
 * \param heapStart Pointer to the where the heap starts
 * \param initialHeapStart How much heap to start with. If 0, then no heap is
 * givin the process at all
 */
void app_startup(
	PrcEntryFunc func, void* cookie, void* heapStart, u32 initialHeapSize);

/*!
 * Called by the OS as the entry point for new threads.
 * You should not call this directly.
 */
void app_threadEntry(ThreadEntryFunc func, void* cookie);

/*!
 * A process's maximum name size, including the null terminator
 */
#define PRC_NAME_SIZE 9

/*!
 * Miscellaneous process information
 */
typedef struct ProcessInfo {
	// process id
	u32 pid;

	// Process name
	char name[PRC_NAME_SIZE];

	// Percentage of the cpu used by the process
	// This include the time spent in SWI calls
	u8 cpuUsr;

	// Percentage of cpu used by the process in kernel mode
	u8 cpuKrn;

	// A combination of APP_FLAG_x values
	u32 flags;
} ProcessInfo;

// #TODO : Check if this is used. If not, then remove it. 
/*!
 * Information required to create a process.
 */
typedef struct CreateProcessParms{
	char name[PRC_NAME_SIZE];

	// The process's `main` function
	PrcEntryFunc startFunc;

	// How much stack (in bytes), the process is allowed to have.
	// This is just an hint to the OS.
	// NOTE: This affects the process's page table size.
	u32 maxStack;

	// Maximum heap size (in bytes).
	// A value of 0 is valid, and it means the process is not allowed to
	// allocate dynamic memory.
	// NOTE: This affects the process's page table size.
	u32 maxHeap;
} CreateProcessParams;

/*!
 * Thread message.
 * The OS uses this to communicate with the process.
 */
typedef struct ThreadMsg
{
	u32 id;
	u32 param1;
	u32 param2;
} ThreadMsg;

/*!
 * Information required to create a thread
 */
typedef struct CreateThreadParams {

	/*!
	 * Entry function for the thread
	 */
	ThreadEntryFunc entryFunc;
	
	/*!
	 * Stack size, in bytes.
	 * Make sure this is large enough for the thread's requirements, as there
	 * is no stack overflow protection.
	 * Unlike for the process's main thread, stack overflows for new thread are
	 * not detected.
	 * This is because unlike for the the main thread, the stack for new thread
	 * is simply allocated from process's heap, and thus when the stack grows
	 * too large, it will corrupt other parts of the heap.
	 */ 
	u32 stackSize;
	
	/*!
	 * Data to pass as a parameter to the thread entry function.
	 * This provides context to the thread, if required.
	 */
	void* cookie;
	
} CreateThreadParams;

typedef struct HeapInfo {
	// Where the heap starts
	void* begin;
	// Size of the heap in pages.
	u32 numPages;
} HeapInfo;

#endif
