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
	
	// How much heap space the app requires
	u32 heapsize;
	
	// Flags to tweak the app behavior.
	// See all the APPFLAG_x macros 
	u32 flags;
} KernelAppInfo;

extern const KernelAppInfo krnApps[kKernelAppID_MAX];

#endif

