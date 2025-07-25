#include "process.h"
#include "../kernel/kernel.h"
#include "utils/linkedlist.h"
#include "utils/bitops.h"
#include "../kernel/mmu.h"
#include "../kernel/handles.h"
#include "../kernel/handles.h"
#include "../syscalls/syscalls.h"
#include <stdlib.h>

PCB* rootPCB;
TCB* rootTCB;

LINKEDLIST_VALIDATE_TYPE(PCB)
LINKEDLIST_VALIDATE_TYPE(TCB)

TCB* prc_createTCB(PCB* pcb, ThreadEntryFunc func, u32 stackBegin, u32 stackEnd,
	u32 crflags, u32 crirqmsk, u32 cookie)
{
	krnassert(pcb && pcb->pt && stackBegin && stackEnd);

	TCB* tcb = calloc(sizeof(TCB));
	if (!tcb) {
		OS_ERR("%s: Out of memory", __func__);
		goto out1;
	}
		
	OS_LOG("%s: TCB %p for '%s'", __func__, tcb, pcb->info.name);
	
	// Allocate handle for the main thread
	tcb->handle = handles_create(pcb, kHandleType_Thread, tcb);
	if (tcb->handle == INVALID_HANDLE)
		goto out2;
	
	tcb->pcb = pcb;
	tcb->state = TCB_STATE_READY;
	tcb->stackBegin = stackBegin;
	tcb->stackEnd = stackEnd;
	
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
	tcb->ctx.gregs[CPU_REG_SP] = stackEnd;
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
	
	out2:
		free(tcb);
	out1:
		return NULL;
}

void timedEvent_wakeupThread(void* tcb_, void* data2, void* data3);
void timedEvent_threadTimer(void* data0, void* data1, void* data2);

static bool timedEvent_removeThreads(const KrnTimedEvent* evt, u32 cookie)
{
	if ((u32)evt->data0 != cookie)
		return false;

	if (evt->func == timedEvent_wakeupThread ||
		evt->func == timedEvent_threadTimer)
		return true;
	else
		return false;
}

/*!
 * Destroys a TCB
 * Don't call this directly.
 * Call `handles_destroy` with the tcb handle
 */
void prc_destroyTCBImpl(TCB* tcb)
{
	krnassert(tcb);
	
	OS_LOG("Destroying thread %p (process %s)", tcb, tcb->pcb->info.name);
	
	// Remove from all queues
	tcb_enqueue(tcb, NULL);
	
	// Remove any timed events using this TCB
	pqueue_KrnTimedEvent_remove(
		&krn.timedEvents, timedEvent_removeThreads, (u32)tcb);

	tcb->state = TCB_STATE_DONE;
	linkedlist_remove(tcb);

	queue_ThreadMsg_destroy(&tcb->msgqueue);
	
	if (krn.currTcb==tcb)
		krn_pickNextTcb();
	
	free(tcb);
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
		u8* stackBegin= calloc(stackSize);
		if (!stackBegin)
			goto out2;
		u8* stackEnd = stackBegin + stackSize;
		tcb = prc_createTCB(pcb, (ThreadEntryFunc)entryFunc, (u32)stackBegin,
			(u32)stackEnd, CPU_CRREG_FLAGS_S | MMU_PTE_KEY_KRN, 0xFFFFFFFF, 0);
		if (!tcb)
		{
			free(stackBegin);
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

		tcb = prc_createTCB(pcb, (ThreadEntryFunc)entryFunc,
			pcb->pt->stackBegin, pcb->pt->stackEnd, 0 | MMU_PTE_KEY_USR,
			0xFFFFFFFF, 0);
		if (!tcb) {
			mmu_destroyPT(pcb->pt);
			goto out2;
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

/*!
 * Destroys a PCB
 * Don't call this directly.
 * Call `handles_destroy` with the process's main thread handle
 */
static void prc_destroyPCBImpl(PCB* pcb)
{
	krnassert(pcb);
	OS_LOG("Destroying process %p (%s)", pcb, pcb->info.name);
	
	linkedlist_remove(pcb);
	mmu_destroyPT(pcb->pt);
	
	// We need to destroy the main TCB right here, because the handle
	// is already gone by now, and therefore handles_destroyPrcHandles doesn't
	// see the mainthread
	prc_destroyTCBImpl(pcb->mainthread);
	
	handles_destroyPrcHandles(pcb);
	free(pcb);
}

void prc_destroyPCB(PCB* pcb)
{
	krnassert(pcb != rootPCB);
	handles_destroy(pcb->mainthread->handle, pcb);
}

void prc_destroyTCB(TCB* tcb)
{
	krnassert(tcb);
	handles_destroy(tcb->handle, tcb->pcb);
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
		prc_destroyTCBImpl(tcb);
}

void timedEvent_wakeupThread(void* tcb_, void* data2, void* data3)
{
	TCB* tcb = (TCB*)tcb_;
	OS_LOG("Waking up thread %p (process %s)", tcb, tcb->pcb->info.name);

	// Mark the thread as ready again
	tcb_enqueue(tcb, &krn.tcbReady);
	tcb->state = TCB_STATE_READY;
	tcb->wait.type = TCB_WAIT_TYPE_NONE;
}

void timedEvent_threadTimer(void* data0, void* data1, void* data2)
{
	TCB* tcb = (TCB*)data0;
	u32 bits = (u32)data1;
	void* cookie = data2;
	
	tcb->pcb->numActiveTimers--;
	
	// If it's a repeating timer, then add it back to the queue
	if (bits & (1 << 31)) {
		u32 ms = bits & TIMER_MAX_INTERVAL_MASK;
		double execTime = krn.intrCurrSecs + ((s32)ms / 1000.0f);
		if (krn_addTimedEvent(
				execTime, timedEvent_threadTimer, tcb, (void*)bits, cookie)) {
			tcb->pcb->numActiveTimers++;
		}
	}
	
	prc_postThreadMsg(tcb, MSG_TIMER, (u32)cookie, 0);
}

static inline void prc_wakeupThread(TCB* tcb)
{
	timedEvent_wakeupThread(tcb, NULL, NULL);
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

void prc_putThreadToWait(TCB* tcb, HANDLE mtx)
{
	OS_LOG("prc_putThreadToWait: %p, %Xh", tcb, mtx);
	tcb->state = TCB_STATE_BLOCKED;
	tcb->wait.type = TCB_WAIT_TYPE_WAIT;
	tcb->wait.d.mtx = mtx;
	
	// Remove from the ready queue.
	tcb_enqueue(tcb, NULL);
}

void prc_wakeOneWaitingThread(HANDLE mtx)
{
	TCB* start = krn.idleTcb;
	LINKEDLIST_FOREACH(start, TCB*, it) {
		if (it->state == TCB_STATE_BLOCKED && it->wait.d.mtx == mtx) {
			timedEvent_wakeupThread(it, NULL, NULL);
			return;
		}
	}
}

bool prc_addThreadTimer(TCB* tcb, u32 ms, bool repeat, void* cookie)
{
	ms &= TIMER_MAX_INTERVAL_MASK;
	
	if (tcb->pcb->numActiveTimers >= TIMER_MAX_TIMERS) {
		OS_ERR("prc_addThreadTimer: %p. Thread timer limit", tcb);
		return false;
	}
	
	double execTime = krn.intrCurrSecs + ((s32)ms / 1000.0f);
	
	// The second parameter passed to the callback is just a bool but the callback
	bool res = krn_addTimedEvent(
		execTime, timedEvent_threadTimer,
		// data0
		tcb,
		// data1: The MSB tells if we should repeat the timer or not
		(void*)(repeat ? ms | (1 << 31) : ms),
		// data2
		cookie);

	if (res)
		tcb->pcb->numActiveTimers++;
		
	return res; 
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

bool prc_postThreadMsg(TCB* tcb, u32 msgId, u32 param1, u32 param2)
{
	ThreadMsg* msg = queue_ThreadMsg_pushEmpty(&tcb->msgqueue);
	if (!msg) {
		OS_ERR("Out of memory");
		return false;
	}
	
	msg->id = msgId;
	msg->param1 = param1;
	msg->param2 = param2;
	
	if (tcb->state == TCB_STATE_BLOCKED &&
		tcb->wait.type == TCB_WAIT_TYPE_WAITING_FOR_MSG) {
		// VERY IMPORTANT:
		// We wake up the thread, BUT don't copy the message to where the thread
		// is expecting it, because although we are in kernel mode, we might
		// be running this function call within the context of a different
		// process, and thus not using the right page table.
		// Therefore, we wake up the thread, and the process will have to all
		// getMsg again.
		prc_wakeupThread(tcb);
	}

	return true;
}
