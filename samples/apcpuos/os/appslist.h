#ifndef _APCPUOS_APPSLIST_H_
#define _APCPUOS_APPSLIST_H_

#include <stddef_shared.h>
#include "appsdk/kernel_shared/syscalls_shared.h"
#include "appsdk/kernel_shared/process_shared.h"


typedef struct KernelAppInfo
{
	const char* name;
	
	PrcEntryFunc startFunc;

	// If TRUE, task is created to run as supervisor mode
	bool privileged;

	// How much stack space to give to this app.
	uint32_t stacksize;
	
	// How much memory to reserve for dynamic memory allocations
	uint32_t memsize;
	
	// Flags to tweak some behaviour
	uint32_t flags;
	
	// User defined cookie. Can be anything 
	uint32_t cookie;
} KernelAppInfo;

int os_getNumApps(void);
KernelAppInfo* os_getAppInfo(uint32_t appnumber);

#endif
