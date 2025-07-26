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

static bool hwcpu_tryGrowStack(void)
{

	// - Only the main thread of a process has dynamic stack
	// - The only valid way to make the stack grow is to attempt a write
	//	 operation. Any other kind of operation means it's a bug in the process.
	if ((krn.currTcb != krn.currTcb->pcb->mainthread) ||
		(krn.intrData.reason & MMU_PTE_W) == 0)
		return false;
		
	PageTable* pt = krn.currTcb->pcb->pt;
	u32 failedAddr = krn.intrData.data2;
	if (failedAddr >= pt->stackBegin && failedAddr < pt->stackEnd) {
		OS_LOG("Growing stack for tcb %p", krn.currTcb);
		mmu_ppBeginTransaction(pt);
		bool res = mmu_setUsrRange(pt, failedAddr, failedAddr+1, true, MMU_PTE_RW);
		mmu_ppFinishTransaction(res);
		mmu_debugdumpState();
		mmu_debugdumpPT(pt);
		
		ADD_USER_KEY;
		// Initialize the new page to 0xCC, so the code that calculates used
		// stack works
		krn.currTcb->stackMappedBegin = MMU_PAGE_TO_ADDR(MMU_ADDR_TO_PAGE(failedAddr));
		memset((void*)krn.currTcb->stackMappedBegin, 0xCC, MMU_PAGE_SIZE);
		REMOVE_USER_KEY;
		
		return res;
	} else {
		return false;
	}
}

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
			// non-existent system call
		}
	} else if (type == kCpuException_DebugBreak) {
	
	} else if (type == kCpuException_Abort && hwcpu_tryGrowStack()) {
		// Successfully allocated more stack pages
		u32* regs = krn.currTcb->ctx.gregs;
		regs[CPU_REG_PC] = krn.intrData.data1;
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
