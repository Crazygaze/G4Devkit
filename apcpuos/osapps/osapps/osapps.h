#ifndef _os_kernel_apps_h
#define _os_kernel_apps_h

#include "appsdk/os_shared/process_shared.h"

typedef enum KernelAppID
{
	kKernelAppID_Idle,
	kKernelAppID_HelloWorld,
	kKernelAppID_MAX
} KernelAppID;

typedef struct KernelAppInfo {
	const char* name;
	PrcEntryFunc startFunc;
	
	// If true, the app will run in kernel mode
	bool privileged;
	
	// How much stack space the app requires
	u32 stacksize;
	
	// Maximum heap size required (in pages).
	// A value of 0 means that no heap is necessary for the application, and
	// thus the allocator is not even initialized, since the page table will not
	// have any address range for the heap.
	//
	// A value of >0 will setup the page table's address range to support that
	// heap size, BUT only 1 page is actually preallocated from the start. It
	// does NOT guarantee the process will have that ammount of heap available.
	// It only guarantees that if the application is launched, it will have 1
	// page to start with, and the allocator can then ask for more as necessary.
	u32 heapNPages;
	
	// Flags to tweak the app behavior.
	// See all the APPFLAG_x macros 
	u32 flags;
} KernelAppInfo;

extern const KernelAppInfo krnApps[kKernelAppID_MAX];

#endif

