#ifndef _appsdk_process_shared_h
#define _appsdk_process_shared_h

#include "syscalls_shared.h"

// #TODO : Document this
typedef void* HANDLE;
#define INVALID_HANDLE 0L

/*!
 * Entry point for a new application
 */
typedef int (*PrcEntryFunc)(void* userdata);

/*!
 * Thread entry function
 */
typedef void (*ThreadEntryFunc)(void* userdata);

/*!
 * Called by the OS as the entry point to a process.
 * You should not call this directly.
 */
void app_startup(PrcEntryFunc func, bool isKernelApp);

/*!
 * Called by the OS as the entry point for new threads.
 * You should not call this directly.
 */
void app_threadEntry(ThreadEntryFunc func, void* userdata);

/*!
 * A process's maximum name size, including the null terminator
 */
#define PRC_NAME_SIZE 9

/*!
 * Miscellaneous process information
 */
typedef struct ProcessInfo
{
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

/*!
 * Information required to create a process.
 */
typedef struct ProcessCreateInfo
{
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
	
} ProcessCreateInfo;

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



#endif
