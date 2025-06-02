#include "hwcpu.h"
#include "boot/boot.h"
#include "../kernel/kernel.h"
#include "../syscalls/syscalls.h"

typedef struct {
	hw_Drv base;
} hwcpu_Drv;

static hwcpu_Drv cpuDrv;

// "target-warning: Impossible to figure out if calling a variadic
// function. Assuming it is not variadic. You should check for possible
// compiler bugs"
#pragma dontwarn 323
void hwcpu_handler(void)
{
	CpuException type = hwcpu_getExceptionType(krn.intrData.reason);
	
	if (type == kCpuException_SWI) {
		u32 callId = krn.intrData.reason >> 14;
		if (callId < kSysCall_Max) {
			OS_VER("SWI %u BEGIN", callId);
			krn_syscalls[callId]();
			OS_VER("SWI %u END", callId);
		} else {
			// Do nothing for now
			
			// #TODO : The application should be killed if it tries to call a
			// non-existant system call
		}
	} else if (type == kCpuException_DebugBreak) {
	
	} else {
		OS_ERR("App crashed with: reason=%u", krn.intrData.reason);
	}
	
	// Nothing to do for the CPU handling, since it's the kernel's job
}
#pragma popwarn


hw_Drv* hwcpu_ctor(uint8_t bus)
{
	u32 totalRam = hwcpu_getTotalRam();
	u32 totalPages = MMU_SIZE_TO_PAGES(totalRam);
	boot_ui_printf("  Total ram installed: %u bytes (%uKB, %u pages)\n", totalRam,
		   totalRam / 1024, totalPages);
		   
	cpuDrv.base.handler = hwcpu_handler;

	return &cpuDrv.base;
}

void hwcpu_dtor(hw_Drv* drv)
{
	memset(drv, 0, sizeof(cpuDrv));
}
