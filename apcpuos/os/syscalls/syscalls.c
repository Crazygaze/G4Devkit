#include "syscalls.h"
#include "../kernel/kernel.h"
#include "../kernel/mmu.h"
#include <stdlib.h>

#define CHECK_USER_PTR(needsWrite, addr, size) \
	if (!mmu_checkUserPtr(krn.currTcb->pcb, needsWrite, addr, size)) return false;

// By default the kernel doesn't have access to user space, to help catch
// bugs in the kernel.
// Whenever the kernel needs access to user space memory, it gives itself
// temporary acess to the user space
#define ADD_USR_KEY hwcpu_addMMUKeys(MMU_PTE_KEY_USR)
#define REMOVE_USER_KEY hwcpu_removeMMUKeys(MMU_PTE_KEY_USR)

bool syscall_setupDS(void)
{
	const PTKrnLayout* layout = mmu_getKrnPTLayout();
	u32 size = layout->oriSharedDataEnd - layout->oriSharedDataBegin;
	
	ADD_USR_KEY;
	hwcpu_addMMUKeys(MMU_PTE_KEY_ORIGINAL_SHARED);
	memcpy((u8*)krn.currTcb->pcb->pt->usrDS, (u8*)layout->oriSharedDataBegin, size);
	hwcpu_removeMMUKeys(MMU_PTE_KEY_ORIGINAL_SHARED);
	REMOVE_USER_KEY;
	
	return true;
}

bool syscall_setTlsPtr(void)
{
	// There is no need for any checks, since the kernel itself doesn't try to
	// use the pointer. If the process passed an invalid pointer, it doesn't
	// affect the kernel whatsoever, since apart from setting the pointer,
	// TLS management is done entirely in user space.
	krn.currTcb->tlsSlotsPtr = (u32*)krn.currTcb->ctx.gregs[0];
	krn_setCurrTCBTlsSlots();
	return true;
}

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
	
	const CreateThreadParams_* in = (const CreateThreadParams_*)regs[0];
	u32* out= (u32*)regs[1];
	
	CHECK_USER_PTR(true, out, sizeof(u32)*4);
	
	CHECK_USER_PTR(false, in, sizeof(*in));
	ADD_USR_KEY;
	CreateThreadParams_ p = *in;
	REMOVE_USER_KEY;
	
	TCB* tcb = prc_createTCB(
		krn.currTcb->pcb,
		p.entryFunc,
		(u32)p.stackEnd,
		0 | MMU_PTE_KEY_USR, 0xFFFFFFFF,
		(u32)p.cookie);
		
	
	ADD_USR_KEY;
	bool res;
	if (tcb) {
		regs[0] = out[0] = (u32)tcb->handle;
		tcb->stackBegin = (void*)p.stackBegin;
		res = true;
	} else {
		regs[0] = out[0] = INVALID_HANDLE;
		res = false;
	}
	REMOVE_USER_KEY;
	return res;
}

bool syscall_setBrk()
{
	int* regs = krn.currTcb->ctx.gregs;
	u32 newbrk = regs[0];
	
	//OS_ERR("********** BEFORE syscall_setBrk(%p) ************", (void*)newbrk);
	// mmu_debugdumpPT(krn.currTcb->pcb->pt);
	
	regs[0] = prc_setBrk(krn.currTcb->pcb, newbrk);
	
	//OS_ERR("********** AFTER syscall_setBrk ************");
	// mmu_debugdumpPT(krn.currTcb->pcb->pt);

	return true;
}

bool syscall_outputDebugString(void)
{
	ADD_USR_KEY;
	
	// #TODO : Test if the pointer validation is working
	const char* usrStr= (const char*)krn.currTcb->ctx.gregs[0];
	
	// #TODO : Make use of size, or think hard if we can safely ignore it.
	u32 size = krn.currTcb->ctx.gregs[1];
	size = min(size, _STDC_LOG_MAXSTRINGSIZE - 1);
	CHECK_USER_PTR(false, usrStr, size); 
	
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
	syscall_setupDS,
	syscall_setTlsPtr,
	syscall_sleep,
	syscall_createThread,
	syscall_setBrk,
	
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
