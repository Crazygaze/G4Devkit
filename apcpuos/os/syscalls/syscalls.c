#include "syscalls.h"
#include "../kernel/kernel.h"
#include "../kernel/mmu.h"
#include "../kernel/handles.h"
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

	CHECK_USER_PTR(true, p.stackBegin, (u8*)p.stackEnd - (u8*)p.stackBegin);

	// Intentionally NOT checking if the stack pointers are valid
	TCB* tcb = prc_createTCB(
		krn.currTcb->pcb,
		p.entryFunc,
		(u32)p.stackBegin,
		(u32)p.stackEnd,
		0 | MMU_PTE_KEY_USR, 0xFFFFFFFF,
		(u32)p.cookie);
		
	
	ADD_USR_KEY;
	bool res;
	if (tcb) {
		regs[0] = out[0] = (u32)tcb->handle;
		res = true;
	} else {
		regs[0] = out[0] = INVALID_HANDLE;
		res = false;
	}
	REMOVE_USER_KEY;
	return res;
}

bool syscall_setBrk(void)
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

bool syscall_getCurrentThread(void)
{
	int* regs = krn.currTcb->ctx.gregs;
	regs[0] = (u32)krn.currTcb->handle;
	return true;
}

bool syscall_getThreadInfo(void)
{
	int* regs = krn.currTcb->ctx.gregs;
	ThreadInfo* inout = (ThreadInfo*)regs[0];
	
	CHECK_USER_PTR(true, inout, sizeof(ThreadInfo));

	ADD_USR_KEY;
	TCB* targetTcb =
		handles_getData(inout->thread, krn.currTcb->pcb, kHandleType_Thread);
		
	if (targetTcb) {
		inout->stackBegin = (void*)targetTcb->stackBegin;
		inout->stackEnd = (void*)targetTcb->stackEnd;
		regs[0] = true;
	} else {
		regs[0] = false;
	}
	REMOVE_USER_KEY;
	
	return true;
}

bool syscall_closeHandle(void)
{
	TCB* tmp = krn.currTcb;
	int* regs = krn.currTcb->ctx.gregs;
	HANDLE h = (HANDLE)regs[0];
	
	bool res = handles_destroy(h, krn.currTcb->pcb);
	
	// Only set the result if the thread to resume is the same one.
	// This is because if we are closing a thread handle, pointers to anything
	// in the TCB are now invalid, and some other thread was picked to run.
	if (krn.currTcb == tmp) {
		regs[0] = res;
	}
	
	return true;
}

bool syscall_createMutex(void)
{
	int* regs = krn.currTcb->ctx.gregs;
	
	// For the mutex data, we are using anything that is not a 0.
	// That's because we only need it to distinguish the handles_getData return
	// values. As-in, we use handles_getData to validate the handle in a few
	// places, and we only care if handles_getData returns NULL or not NULL.
	regs[0] = (u32)handles_create(krn.currTcb->pcb, kHandleType_Mutex, krn.currTcb);
	return true;
}

bool syscall_waitForMutex(void)
{
	int* regs = krn.currTcb->ctx.gregs;
	HANDLE h = (HANDLE)regs[0];
	
	if (handles_getData(h, krn.currTcb->pcb, kHandleType_Mutex)) {
		prc_putThreadToWait(krn.currTcb, h);
		// Since the current thread was put to sleep, we need to pick another
		// one to run
		krn_pickNextTcb();
	}
	
	return true;
}

// #TODO Implement this
bool syscall_mutexUnlocked(void)
{
	int* regs = krn.currTcb->ctx.gregs;
	HANDLE h = (HANDLE)regs[0];
	
	if (handles_getData(h, krn.currTcb->pcb, kHandleType_Mutex)) {
		prc_wakeOneWaitingThread(h);
	}
	
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
	syscall_getCurrentThread,
	syscall_getThreadInfo,
	syscall_closeHandle,
	syscall_createMutex,
	syscall_waitForMutex,
	syscall_mutexUnlocked,
	
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
