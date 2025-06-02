#include "process.h"
#include "../kernel/kernel.h"
#include "utils/linkedlist.h"
#include "../kernel/mmu.h"
#include <string.h>
#include <stdlib.h>

PCB* rootPCB;
TCB* rootTCB;

LINKEDLIST_VALIDATE_TYPE(PCB)
LINKEDLIST_VALIDATE_TYPE(TCB)

static void prc_setupTCB(
	TCB* tcb, PCB* pcb, u32 stackTop, ThreadEntryFunc func, u32 crflags, u32 crirqmsk)
{
	krnassert(tcb && pcb && pcb->pt && stackTop);
	
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
	tcb->ctx.gregs[1] = (pcb->pt == mmu_getKrnOnlyPT()) ? true : false;
	
	tcb->ctx.crregs[CPU_CRREG_FLAGS] = crflags;
	tcb->ctx.crregs[CPU_CRREG_IRQMSK] = crirqmsk;
	tcb->ctx.crregs[CPU_CRREG_TSK] = (u32)&tcb->ctx;;
	
	queue_ThreadMsg_create(&tcb->msgqueue, 0);
	tcb_enqueue(tcb, &krn.tcbReady);
}

PCB* prc_create(const char* name, PrcEntryFunc entryFunc, bool kernelMode,
	u32 stackSize, u32 heapSize)
{
	OS_LOG("prc_create('%s', %Xh, %u, %u, %u)", name, entryFunc, kernelMode,
		stackSize, heapSize);
		
	PCB* pcb = calloc(sizeof(PCB));
	if (!pcb)
		goto out1;
	OS_VER("PCB for '%s': %Xh", name, pcb);
	
	strncpy(pcb->info.name, name, PRC_NAME_SIZE);
	pcb->info.name[PRC_NAME_SIZE-1] = '\0';
	// NOTE: We only increment `krn.pidCounter` itself once everything succeeds
	pcb->info.pid = krn.pidCounter + 1;
	
	TCB* tcb = calloc(sizeof(TCB));
	if (!tcb)
		goto out2;
	OS_VER("TCB for '%s': %Xh", name, tcb);
	
	if (kernelMode) {
		pcb->pt = mmu_getKrnOnlyPT();
		u8* stackBottom = calloc(stackSize);
		if (!stackBottom)
			goto out3;
		u8* stackTop = stackBottom + stackSize;
		prc_setupTCB(tcb, pcb, (u32)stackTop, (ThreadEntryFunc)entryFunc,
			CPU_CRREG_FLAGS_S | MMU_PTE_KEY_KRN, 0xFFFFFFFF);
		tcb->state = TCB_STATE_KERNEL;
		// For a kernel process, its threads don't participate in the scheduler,
		// so remove it from the ready queue
		tcb_enqueue(tcb, NULL);
	} else {
		pcb->pt = mmu_createUsrPT(stackSize, heapSize);
		if (!pcb->pt)
			goto out3;

		prc_setupTCB(tcb, pcb, pcb->pt->usrSP, (ThreadEntryFunc)entryFunc,
			0 | MMU_PTE_KEY_USR, 0xFFFFFFFF);
	}
	
	krn.pidCounter++;
	
	if (rootPCB == NULL) {
		rootPCB = pcb;
		// Point to self to close the linked list
		linkedlist_addAfter(pcb, pcb);
	} else {
		linkedlist_addAfter(rootPCB->previous, pcb);
	}
	
	return pcb;
	
	out3:
		free(tcb);
	out2:
		free(pcb);
	out1:
		return NULL;
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
	tcb->wait.d.sleepEnd = krn.intrCurrSecs + (ms / 1000.0f);
	
	// Remove from the ready queue.
	tcb_enqueue(tcb, NULL);
	
	// Queue up the event
	krn_addTimedEvent(
		tcb->wait.d.sleepEnd, timedEvent_wakeupThread, tcb, NULL, NULL);
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
