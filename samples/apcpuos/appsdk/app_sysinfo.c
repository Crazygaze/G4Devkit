#include "app_sysinfo.h"

int sysinfo_getProcessCount(void)
{
	return app_syscall0(kSysCall_GetProcessCount);
}

int sysinfo_getOSInfo(OSInfo* osinfo, ProcessInfo* prcs, int prcCount,
	uint32_t flags)
{
	return app_syscall4(kSysCall_GetOSInfo, (int)osinfo, (int)prcs, prcCount,
		flags);
}
