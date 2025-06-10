#include "syscalls.h"
#include "../kernel/kernel.h"
#include "../kernel/mmu.h"
#include <stdlib.h>

/*!
 * Checks if the specified PCB can access the specified memory block.
 */
static bool checkUserPtr(struct PCB* pcb, u32 access, void* addr, u32 size)
{
	// #TODO : implements this
	// Things to do:
	// - Even if size is 0, addr needs to be checked. This avoid exploits such
	// as when the user code passes a const char* pointer to kernel space, and
	// specifies that it's size is 0, which then can end up with the kernel
	// copying private data to somewhere the user asked.
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

// By default the kernel doesn't have access to user space, to help catch
// bugs in the kernel.
// Whenever the kernel needs access to user space memory, it gives itself
// temporary acess to the user space
#define ADD_USR_KEY hwcpu_addMMUKeys(MMU_PTE_KEY_USR)
#define REMOVE_USER_KEY hwcpu_removeMMUKeys(MMU_PTE_KEY_USR)

bool syscall_sleep(void)
{
	u32 ms = krn.currTcb->ctx.gregs[0];
	prc_putThreadToSleep(krn.currTcb, ms);
	// Since the current thread was put to sleep, we need to pick another one to
	// run
	krn_pickNextTcb();
	return true;
}

bool syscall_createThread(void)
{
	int* regs = krn.currTcb->ctx.gregs;
	u32 pid = krn.currTcb->pcb->info.pid;
	
	const CreateThreadParams* inParams= (const CreateThreadParams*)regs[0];
	CHECK_USER_PTR(MMU_PTE_R, inParams, sizeof(*inParams));
	ADD_USR_KEY;
	ThreadEntryFunc entryFunc = inParams->entryFunc;
	u32 cookie = (u32)inParams->cookie;
	REMOVE_USER_KEY;
	
	u32 stackTop = regs[2];
	
	TCB* tcb = prc_createTCB(
		krn.currTcb->pcb,
		entryFunc,
		stackTop,
		0 | MMU_PTE_KEY_USR, 0xFFFFFFFF,
		cookie);
		
	if (tcb) {
		regs[0] = (u32)tcb->handle;
		return true;
	} else {
		regs[0] = INVALID_HANDLE;
		return false;
	}
}

bool syscall_outputDebugString(void)
{
	ADD_USR_KEY;
	
	// #TODO : Test if the pointer validation is working
	const char* usrStr= (const char*)krn.currTcb->ctx.gregs[0];
	
	// #TODO : Make use of size, or think hard if we can safely ignore it.
	u32 size = krn.currTcb->ctx.gregs[1];
	size = min(size, _STDC_LOG_MAXSTRINGSIZE - 1);
	CHECK_USER_PTR(MMU_PTE_R, usrStr, size); 
	
	// Copy the string to kernel pages, since hardware functions expect physical
	// addresses and kernel pages are not setup with translation.
	char str[_STDC_LOG_MAXSTRINGSIZE];
	memcpy(str, usrStr, size);
	str[size] = 0;
	
	hwnic_sendDebug((phys_addr)str);
	
	REMOVE_USER_KEY;
	return true;
}

const krn_syscallFunc krn_syscalls[kSysCall_Max] =
{
	// 
	// Process Management
	//
	
	syscall_sleep,
	syscall_createThread,
	
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
