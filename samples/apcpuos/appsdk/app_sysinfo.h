#ifndef _app_sysinfo_h_
#define _app_sysinfo_h_

#include "appsdkconfig.h"
#include "app_syscalls.h"
#include "kernel_shared/process_shared.h"
#include "kernel_shared/syncprimitives_shared.h"

/*! Gets the total number of process running
*/
int sysinfo_getProcessCount(void);


/*! Gets runtime information for the OS and for all processes
*\param osinfo
*	Where you will get the OS runtime information
*\param prcs
*	Array where you will get the the processes information
*\param prcCount
*	Size of the array for the processes.
*\param flags
*	Flags that specify what stats to recalculate.
*	Note that if a particular stat is not recalculated, it will still be
*	present.
*	Flags available are:
*	OSINFO_KERNELMEM - Recalculate kernel memory stats
*	OSINFO_CPU - Recalculate cpu usage
*\return
*	Number of processes retrieved
*/
int sysinfo_getOSInfo(OSInfo* osinfo, ProcessInfo* prcs, int prcCount,
	uint32_t flags);

#endif
