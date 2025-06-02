#include "syscalls.h"
#include "../kernel/kernel.h"
#include "../kernel/mmu.h"

/*!
 * Checks if the specified PCB can access the specified memory block.
 */
static bool checkUserPtr(struct PCB* pcb, u32 access, void* addr, u32 size)
{
	// #TODO : implements this
	return true;
}

static bool checkUserPtrAndLog(struct PCB* pcb, u32 access, void* addr, u32 size)
{
	if (checkUserPtr(pcb, access, addr, size))
		return true;

	OS_ERR("checkUserPtr failed: PCB=%p, access=%u, addr=%p, size=%u", pcb,
		access, addr, size);
	return false;
}

#define CHECK_USER_PTR(access, addr, size) \
	if (!checkUserPtrAndLog(krn.currTcb->pcb, access, addr, size)) return false;

bool syscall_sleep(void)
{
	u32 ms = krn.currTcb->ctx.gregs[0];
	prc_putThreadToSleep(krn.currTcb, ms);
	// Since the current thread was put to sleep, we need to pick another one to
	// run
	krn_pickNextTcb();
	return true;
}

// By default the kernel doesn't have access to user space, to help catch
// bugs in the kernel.
// Whenever the kernel needs access to user space memory, it gives itself
// temporary acess to the user space
#define ADD_USR_KEY hwcpu_addMMUKeys(MMU_PTE_KEY_USR)
#define REMOVE_USER_KEY hwcpu_removeMMUKeys(MMU_PTE_KEY_USR)

bool syscall_outputDebugString(void)
{
	ADD_USR_KEY;
	
	// #TODO : Test if the pointer validation is working
	const char* str = (const char*)krn.currTcb->ctx.gregs[0];
	
	// #TODO : Make use of size, or think hard if we can safely ignore it.
	u32 size = krn.currTcb->ctx.gregs[1];
	CHECK_USER_PTR(MMU_PTE_R, str, size); 

	hwnic_sendDebug(str);
	
	REMOVE_USER_KEY;
	return true;
}

const krn_syscallFunc krn_syscalls[kSysCall_Max] =
{
	// 
	// Process Management
	//
	
	//
	// Process control
	//
	syscall_sleep,
	
	//
	// System information
	//
	
	//
	// Hardware
	//
	
	// 
	// Disk Drive
	//

	//
	// Rendering
	//
	
	//
	// Debug
	//
	syscall_outputDebugString
};
