#include "process.h"
#include "../kernel/kernel.h"
#include "utils/linkedlist.h"
#include "utils/bitops.h"
#include "../kernel/mmu.h"
#include "../kernel/handles.h"
#include "../kernel/handles.h"
#include <stdlib.h>

PCB* rootPCB;
TCB* rootTCB;

LINKEDLIST_VALIDATE_TYPE(PCB)
LINKEDLIST_VALIDATE_TYPE(TCB)

TCB* prc_createTCB(PCB* pcb, ThreadEntryFunc func, u32 stackTop, u32 crflags,
	u32 crirqmsk, u32 cookie)
{
	krnassert(pcb && pcb->pt && stackTop);

	TCB* tcb = calloc(sizeof(TCB));
	if (!tcb) {
		OS_ERR("%s: Out of memory", __func__);
		return NULL;
	}
		
	OS_LOG("%s: TCB %p for '%s'", __func__, tcb, pcb->info.name);
	
	tcb->pcb = pcb;
	tcb->state = TCB_STATE_READY;
	
	// If the idle process is not created yet, then
	if (rootTCB == NULL) {
		rootTCB = tcb;
		// Point to self to close the linked list
		linkedlist_addAfter(tcb, tcb);
	} else {
		linkedlist_addAfter(rootTCB->previous, tcb);
	}

	bool isMainThread = false;
	if (!pcb->mainthread) {
		pcb->mainthread = tcb;
		isMainThread = true;
	}

	tcb->ctx.gregs[CPU_REG_DS] = pcb->pt->usrDS;
	tcb->ctx.gregs[CPU_REG_SP] = stackTop;
	tcb->ctx.gregs[CPU_REG_PC] = isMainThread ? (u32)app_startup : (u32)app_threadEntry;
	tcb->ctx.crregs[CPU_CRREG_PT] = (u32)pcb->pt->data;
	
	// r0,r1,r2,r3 get passed to app_startup as parameters
	tcb->ctx.gregs[0] = (u32)func;
	tcb->ctx.gregs[1] = cookie;
	
	tcb->ctx.crregs[CPU_CRREG_FLAGS] = crflags;
	tcb->ctx.crregs[CPU_CRREG_IRQMSK] = crirqmsk;
	tcb->ctx.crregs[CPU_CRREG_TSK] = (u32)&tcb->ctx;;
	
	queue_ThreadMsg_create(&tcb->msgqueue, 0);
	tcb_enqueue(tcb, &krn.tcbReady);
	
	return tcb;
}

PCB* prc_createPCB(const char* name, PrcEntryFunc entryFunc, bool kernelMode,
	u32 stackSize, u32 heapNPages)
{
	OS_LOG("prc_create('%s', %Xh, %u, %u, %u)", name, entryFunc, kernelMode,
		stackSize, heapNPages);
		
	PCB* pcb = calloc(sizeof(PCB));
	if (!pcb)
		goto out1;
	OS_VER("PCB for '%s': %Xh", name, pcb);
	
	strncpy(pcb->info.name, name, PRC_NAME_SIZE);
	pcb->info.name[PRC_NAME_SIZE-1] = '\0';
	// NOTE: We only increment `krn.pidCounter` itself once everything succeeds
	pcb->info.pid = krn.pidCounter + 1;
	
	TCB* tcb;
	
	if (kernelMode) {
		pcb->pt = mmu_getKrnOnlyPT();
		u8* stackBottom = calloc(stackSize);
		if (!stackBottom)
			goto out2;
		u8* stackTop = stackBottom + stackSize;
		tcb = prc_createTCB(pcb, (ThreadEntryFunc)entryFunc, (u32)stackTop,
			CPU_CRREG_FLAGS_S | MMU_PTE_KEY_KRN, 0xFFFFFFFF, 0);
		if (!tcb)
		{
			free(stackBottom);
			goto out2;
		}
		tcb->state = TCB_STATE_KERNEL;
		// For a kernel process, its threads don't participate in the scheduler,
		// so remove it from the ready queue
		tcb_enqueue(tcb, NULL);
	} else {
		pcb->pt = mmu_createUsrPT(stackSize, heapNPages);
		if (!pcb->pt)
			goto out2;

		tcb = prc_createTCB(pcb, (ThreadEntryFunc)entryFunc, pcb->pt->stackEnd,
			0 | MMU_PTE_KEY_USR, 0xFFFFFFFF, 0);
		if (!tcb) {
			goto out2;
			mmu_destroyPT(pcb->pt);
		}
		
		tcb->ctx.gregs[2] = pcb->pt->heapBegin;
		tcb->ctx.gregs[3] = pcb->pt->brk - pcb->pt->heapBegin;
	}
	
	// Copy the .data_shared/.bss_shared over to to the new process
	
	
	pcb->pt->owner = pcb;
	krn.pidCounter++;
	
	if (rootPCB == NULL) {
		rootPCB = pcb;
		// Point to self to close the linked list
		linkedlist_addAfter(pcb, pcb);
	} else {
		linkedlist_addAfter(rootPCB->previous, pcb);
	}
	
	return pcb;
	
	out2:
		free(pcb);
	out1:
		return NULL;
}

static void prc_destroyPCBImpl(PCB* pcb)
{
	// #TODO : Implement this
	// Things to do:
	// - Deallocate any memory
	// - Free mmu pages
}

void prc_destroyTCB(TCB* tcb)
{
	// #TODO : Implement this
	// Things to do:
	// - Deallocate any memory
}

void prc_destroyPCB(PCB* pcb)
{
	krnassert(pcb != rootPCB);
	// NOTE: This will end up calling handles_thread_dtr, which is how
	// destruction is done.
	handles_destroy(pcb->mainthread->handle, pcb->info.pid);
}

void handles_thread_dtr(void* data)
{
	if (!data)
		return;
	TCB* tcb = (TCB*)data;
	// If it's the main thread, then destroy the process
	if (tcb == tcb->pcb->mainthread)
		prc_destroyPCBImpl(tcb->pcb);
	else
		prc_destroyTCB(tcb);
}

void timedEvent_wakeupThread(void* tcb_, void* data2, void* data3)
{
	TCB* tcb = (TCB*)tcb_;
	OS_LOG("Waking up thread %p (process %s)", tcb, tcb->pcb->info.name);

	// Mark the thread as ready again
	tcb_enqueue(tcb, &krn.tcbReady);
	tcb->state = TCB_STATE_READY;
}

void prc_putThreadToSleep(TCB* tcb, u32 ms)
{
	OS_LOG("prc_putThreadToSleep: %p, %u", tcb, ms);
	tcb->state = TCB_STATE_BLOCKED;
	tcb->wait.type = TCB_WAIT_TYPE_SLEEP;
	// Instead of saving the duration, we save the time we need to wake up.
	tcb->wait.d.sleepEnd = krn.intrCurrSecs + ((s32)ms / 1000.0f);
	
	// Remove from the ready queue.
	tcb_enqueue(tcb, NULL);
	
	// Queue up the event
	krn_addTimedEvent(
		tcb->wait.d.sleepEnd, timedEvent_wakeupThread, tcb, NULL, NULL);
}

bool prc_setBrk(PCB* pcb, u32 newbrk)
{
	// Ignore any requests outside the allowed range.
	if (newbrk < pcb->pt->heapBegin || newbrk > pcb->pt->heapEnd)
		return false;
		
	// If the request stays within the current brk, then nothing to do
	if (MMU_SAME_PAGE(newbrk, pcb->pt->brk)) {
		pcb->pt->brk = newbrk;
		return true;
	}
		
	bool res = true;
	mmu_ppBeginTransaction(pcb->pt);

	if (newbrk > pcb->pt->brk) {
		// expand heap
		res = mmu_setUsrRange(pcb->pt, pcb->pt->brk, newbrk, true, MMU_PTE_RW);
	} else if (newbrk < pcb->pt->brk) {
		// contract heap
		mmu_freePageRange(pcb->pt,
			ALIGN(newbrk, MMU_PAGE_SIZE), ALIGN(pcb->pt->brk, MMU_PAGE_SIZE));
	} else { // didn't change
		// Nothing to do
	}

	mmu_ppFinishTransaction(res);
	if (res)
		pcb->pt->brk = newbrk;
	return res;
}
 
void tcb_enqueue(TCB* tcb, Queue32* to)
{
	if (tcb->queue) {
		queue32_remove(tcb->queue, (u32)tcb);
	}
	
	if (to) {
		queue32_push(to, (u32)tcb);
		tcb->queue = to;
	} else {
		tcb->queue = NULL;
	}
}
