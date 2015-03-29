/*******************************************************************************
 *  Code for managing programs
 ******************************************************************************/

#include "process.h"
#include "boot/boot.h"
#include "hw/hwcpu.h"
#include "hw/hwclock.h"
#include "hw/hwscreen.h"
#include "kernel/kernel.h"
#include "kernel/kerneldebug.h"
#include "kernel/mmu.h"
#include "kernel/handles.h"
#include "kernel/kernelTimedEvents.h"
#include "utilshared/misc.h"

#include <string_shared.h>
#include <stdlib_shared.h>

PCB rootpcb;
TCB rootthread;

static uint8_t pidCounter;

LINKEDLIST_VALIDATE_TYPE(PCB)
LINKEDLIST_VALIDATE_TYPE(TCB)


/*
Memory layout:
 TOP
    .....
    .....
	Dynamic memory
	Shared data copy
	Extra (mmu table if kernel process)
	Stack
 BOTTOM
*/
typedef struct HeapInfo
{
	void* start;
	size_t size;
} HeapInfo;

/*! Setups the process page permissions
*
*/
void prc_setPagesAccess(PCB* pcb, uint8_t pid)
{
	mmu_setPages(pcb->firstPage, pcb->numPages, pid, true, true, false);
	
	// Search for threads that belong to this process, and set any pages that
	// were allocated for that thread
	TCB *it = pcb->mainthread->next;
	while(
		// Normally, "it" will always be valid, since the threads are presented
		// as a circular linked list, but if this function is called during
		// a process setup, then it can be NULL.
		it &&
		it!=pcb->mainthread)
	{
		if (it->pcb==pcb) {
			mmu_setPages(
				ADDR_TO_PAGE(it->stackBottom),
				SIZE_TO_PAGES(it->stackTop - it->stackBottom),
				pid, true, true, false);
		}
		it = it->next;
	};
}

void prc_giveAccessToKernel(PCB* pcb, bool status)
{
//
// Depending if we want a faster or a safer approach or a faster one, we either
// give the kernel access to everything by overriding the mmu settings (faster),
// or we change the target process pages settings (slower)
//
#if (defined(DEBUG) && DEBUGBUILD_FAST_USERSPACE_CHECK) || (!defined(DEBUG) && RELEASEBUILD_FAST_USERSPACE_CHECK)
	if (status)
		hw_cpu_setProcessKeys(MMU_KEY(0,0,PID_KERNEL));
	else
		hw_cpu_setProcessKeys(MMU_KEY(PID_KERNEL, PID_KERNEL, PID_KERNEL));
#else
	prc_setPagesAccess(pcb, status ? PID_KERNEL : pcb->info.pid);
#endif
}


static bool prc_setupMemory(
	PCB* pcb,
	size_t stackSize, size_t heapSize, size_t extraSize,
	HeapInfo* heapInfo)
{
	bool isKrn = pcb==&rootpcb;
	
	int sharedSize = align(processInfo.sharedReadWriteSize, 4);	
	stackSize = align( max(stackSize,PRC_MIN_STACKSIZE) ,4);
	heapSize = align(heapSize,4);
	size_t neededSize = stackSize + extraSize + sharedSize + heapSize;
	// If no dynamic memory needed, then don't waste space with tlsf
	if (heapSize)
		neededSize += tlsf_size();
	neededSize = align(neededSize, MMU_PAGE_SIZE);
	int numPages = SIZE_TO_PAGES(neededSize);
	
	if (!isKrn) {
		KERNEL_DEBUG("PCB %s: stack %u, heap %u, total %u, pages %u",
			pcb->info.name, stackSize, heapSize, neededSize, numPages);
	}

	//
	// Calculate first page
	//
	int firstPage;
	if (isKrn) {
		firstPage = ADDR_TO_PAGE( align(
			processInfo.sharedReadWriteAddr+sharedSize,MMU_PAGE_SIZE));
	} else {
		firstPage = mmu_findFreePages( numPages );
		if (firstPage==-1) {
			KERNEL_DEBUG("Not enough contiguous pages to create process heap");
			return FALSE;
		}
	}
	
	uint8_t* memStart = PAGE_TO_ADDR(firstPage);
	if (!isKrn) {
		KERNEL_DEBUG("PCB %s: Reserved pages %u..%d, address %u..%u",
			pcb->info.name, firstPage, firstPage+numPages-1,
			(uint32_t)memStart, (uint32_t)memStart+neededSize-1);
	}

	pcb->firstPage = firstPage;
	pcb->numPages = numPages;
	pcb->ds = memStart + stackSize + extraSize;	
	
	// WARNING: This needs to be setup before initializing the MMU, otherwise
	// if this is the kernel process initialization, it will crash since this
	// thread context is in protected memory
	TCB* tcb = pcb->mainthread;
	tcb->cpuctx->gregs[CPU_REG_DS] = (uint32_t)pcb->ds;	
	
	//
	// Stack
	// The stack is at the start, to help catch stack overflow,
	// since it will crash when trying to write to the lower page
	// Note: Not actually setting the stack register here, because this code
	// is also used to initialize the kernel process. The caller is responsible
	// by setting the registers
	uint8_t* ptr = memStart;
	tcb->stackBottom = ptr;
	tcb->stackTop = ptr + stackSize;

	// If this is the kernel process, we initialize the mmu as soon as
	// possible to help catch bugs during kernel boot.		
	if (isKrn)
	{
		assert(extraSize>=MMU_TABLE_SIZE);
		mmu_init(firstPage, numPages, memStart+stackSize);
	}
	
	//
	// Shared data
	//
	// Temporarly set write permissions for the kernel so we can write stuff into
	// the process heap area
	prc_setPagesAccess(pcb, PID_KERNEL);
	memcpy(pcb->ds , (void*)processInfo.sharedReadWriteAddr, sharedSize);
	ptr = (uint8_t*)pcb->ds + sharedSize;
	prc_setPagesAccess(pcb, pcb->info.pid);

	memset(heapInfo, 0, sizeof(*heapInfo));
	if (heapSize) {
		heapInfo->start = ptr;
		heapInfo->size = neededSize - (ptr-memStart);
	}

	pcb->info.memory = mmu_countPages(pcb->info.pid) * MMU_PAGE_SIZE;
	krn.info.mem_available = mmu_countPages(PID_NONE)*MMU_PAGE_SIZE;
	return TRUE;
}

void* prc_getPtrToShared(PCB* pcb, void* var)
{
	u32 offset = (u32)var - rootthread.cpuctx->gregs[CPU_REG_DS];
	return (void*)(pcb->mainthread->cpuctx->gregs[CPU_REG_DS] + offset);
}

PCB* prc_initKernelPrc(void)
{	
	//
	// Initialize PCB
	strncpy(rootpcb.info.name, "kernel", PRC_NAME_SIZE);
	rootpcb.mainthread = &rootthread;
	rootpcb.info.pid = ++pidCounter;
	assert(rootpcb.info.pid==PID_KERNEL);
	// Point to self to close circle
	linkedlist_addAfter(&rootpcb, &rootpcb);
	
	//
	// Setup main thread
	rootthread.cpuctx = (CpuCtx*)(&intrCtxStart);
	rootthread.pcb = &rootpcb;
	// marking as kernel, so we don't try to run explicitly
	rootthread.state = TCB_STATE_KERNEL;
	// Point to self, to close the circle
	linkedlist_addAfter(&rootthread, &rootthread);
	rootthread.cpuctx->flags =
		(rootthread.cpuctx->flags & 0xFF000000) |
		MMU_KEY(rootpcb.info.pid, rootpcb.info.pid,rootpcb.info.pid);
	
	// TODO : Fix this. The kernel shouldn't have automatic access to the 
	// entire memory.
	//rootthread.cpuctx->flags = (rootthread.cpuctx->flags & 0xFF000000);
		
	HeapInfo heapInfo;
	bool ret = prc_setupMemory(
		&rootpcb, KERNEL_STACKSIZE, KERNEL_HEAPSIZE, MMU_TABLE_SIZE,
		&heapInfo);
	assert(ret);
	stdcshared_init(krn_debugOutput, heapInfo.start, heapInfo.size);

	return &rootpcb;
}

TCB* prc_createThread(PCB* pcb, ThreadEntryFuncWrapper entryFuncWrapper,
	ThreadEntryFunc entryFunc, u32 stackSize, void* userdata, bool privileged)
{
	u32 stackNumPages = SIZE_TO_PAGES(stackSize);
	u32 firstPage = mmu_findFreeAndSet(stackNumPages, pcb->info.pid, true, true,
		false);
	if (firstPage==-1) {
		KERNEL_DEBUG("Not enough contiguous pages to create process heap");
		return FALSE;
	}
	
	//
	// Allocate TCB+CpuCtx in one chunk
	TCB* tcb = calloc(sizeof(TCB)+sizeof(CpuCtx));
	if (!tcb) goto error;
	KERNEL_DEBUG("TCB for %s : %u", pcb->info.name, (uint32_t)tcb);
	
	//
	// Initialize TCB
	tcb->pcb = pcb;
	tcb->cpuctx = (CpuCtx*)(tcb+1); // Cpu state is right after the process data
	tcb->stackBottom = PAGE_TO_ADDR(firstPage);
	tcb->stackTop = tcb->stackBottom + PAGES_TO_SIZE(stackNumPages);	
	tcb->state = TCB_STATE_READY;
	// setup registers
	tcb->cpuctx->gregs[0] = (uint32_t)entryFunc;
	tcb->cpuctx->gregs[1] = (uint32_t)userdata;
	tcb->cpuctx->gregs[CPU_REG_DS] = (uint32_t)pcb->ds;
	tcb->cpuctx->gregs[CPU_REG_SP] = (uint32_t)tcb->stackTop;
	tcb->cpuctx->gregs[CPU_REG_PC] = (uint32_t)entryFuncWrapper;
	tcb->cpuctx->flags =
		0x00000000 |
		(privileged ? (1<<CPU_FLAGSREG_SUPERVISOR) : 0) |
		(pcb->info.pid<<16 | pcb->info.pid<<8 | pcb->info.pid);
	
	tcb->handle = handles_create(pcb->info.pid, kHandleType_Thread, tcb);
	if (tcb->handle==INVALID_HANDLE)
		goto error;
	
	// Setup the links
	linkedlist_addAfter(rootthread.previous, tcb);
	tcb_enqueue(tcb, &krn.tcbReady);
	
	// Initialize message queue
	queue_create(&tcb->msgqueue, sizeof(ThreadMsg), 0);

	return tcb;
	
error:
	if (tcb) free(tcb);	
	return NULL;
}

bool prc_postThreadMessage(TCB* tcb, u32 msgId, u32 param1, u32 param2)
{
	ThreadMsg* msg = queue_pushEmpty(&tcb->msgqueue);
	if (!msg)
		return false;
	msg->id = msgId;
	msg->param1 = param1;
	msg->param2 = param2;
	
	// If the thread is blocked waiting for a message, set it as ready
	if (tcb->state==TCB_STATE_BLOCKED &&
		tcb->wait.type==TCB_WAIT_TYPE_WAITING_FOR_MSG) {
		tcb_enqueue(tcb, &krn.tcbReady);
		tcb->state = TCB_STATE_READY;
		// Copy the message to the waiting thread, otherwise it will get garbage
		prc_giveAccessToKernel(tcb->pcb, true);
		queue_pop(&tcb->msgqueue, tcb->wait.d.msgDst);
		prc_giveAccessToKernel(tcb->pcb, false);
		//KERNEL_DEBUG("Thread %u unblocked for receiving message %d:%d:%d",
		//		tcb->handle, msgId, param1, param2);
	}
		
	return true;
}


// TODO : Make these static
void timedEvent_wakeupThread(void* tcb_, void* data2, void* data3)
{
	TCB* tcb = (TCB*)tcb_;
	tcb_enqueue(tcb, &krn.tcbReady);
	tcb->state = TCB_STATE_READY;
	/*
	KERNEL_DEBUG("WOKE UP THREAD %d. EventsQueueSize=%d", tcb->handle,
		krn.timedEvents.a.size);
	*/
}
// TODO : Make these static
void timedEvent_threadTimer(void* tcb_, void* timerId_, void* ms_)
{
	TCB* tcb = (TCB*)tcb_;
	//KERNEL_DEBUG("timedEvent_threadTimer(%d,%d,%d,%d). Current TCBHandle=%d", tcb->handle,
	//	(u32)timerId_, (u32)ms_ & TIMER_MAX_INTERVAL_MASK, (u32)ms_ >> 31, krn.interruptedTcb->handle);

	// Top bit is used to specify the repeat behaviour
	if ((u32)ms_ & (1<<31)) {
		prc_setThreadTimer(tcb, (u32)timerId_,
			(u32)ms_ & TIMER_MAX_INTERVAL_MASK, true);
	}
	
	prc_postThreadMessage(tcb, MSG_TIMER, (u32)timerId_, NULL);
}

static bool timedEvent_removeThreads(const void* element_, void* tcb_)
{
	KrnTimedEvent* evt = (KrnTimedEvent*)element_;
	if ( evt->data1==tcb_ &&
		(evt->func==timedEvent_wakeupThread ||
		 evt->func==timedEvent_threadTimer)) {		 
		KERNEL_DEBUG("REMOVING THREAD EVENT %d", ((TCB*)tcb_)->handle);
		return true;
	} else {
		return false;
	}
}

void prc_putThreadToSleep(TCB* tcb, u32 ms)
{
	tcb->state = TCB_STATE_BLOCKED;
	tcb->wait.type = TCB_WAIT_TYPE_SLEEP;
	// Instead of saving the duration, we save the time we need to wake up,
	// which is currenTime + duration
	tcb->wait.d.sleepTimeMarker = hw_clk_currSecs + (ms/1000.0f);
	
	// Remove from the ready queue and put it in the sleep array
	tcb_enqueue(tcb, NULL);
	krn_addTimedEvent(tcb->wait.d.sleepTimeMarker, &timedEvent_wakeupThread,
		tcb, NULL, NULL);
}

bool prc_setThreadTimer(TCB* tcb, uint32_t timerId, uint32_t ms, bool repeat)
{
	double time = hw_clk_currSecs + (ms/1000.0f);
	bool res = krn_addTimedEvent( time, &timedEvent_threadTimer, tcb,
		(void*)timerId, (void*) (repeat ? ms|(1<<31) : ms));
	return res;
}

PCB* prc_create(const char* name, PrcEntryFunc entryfunc, bool privileged,
				size_t stackSize, size_t heapSize)
{
	KERNEL_DEBUG("prc_create('%s', 0x%X, %u, %u, %u)",
		name, entryfunc, privileged, stackSize, heapSize);
	
	//
	// Allocate PCB
	PCB* pcb = calloc(sizeof(PCB));
	if (!pcb) goto out1;
	KERNEL_DEBUG("PCB for %s : %u", name, (uint32_t)pcb);

	//
	// Allocate TCB+CpuCtx
	TCB* tcb = calloc(sizeof(TCB)+sizeof(CpuCtx));
	if (!tcb) goto out2;
	KERNEL_DEBUG("TCB for %s : %u", name, (uint32_t)tcb);

	//
	// Initialize PCB
	strncpy(pcb->info.name, name, PRC_NAME_SIZE);
	pcb->info.name[PRC_NAME_SIZE-1] = '\0';
	pcb->info.pid = pidCounter+1; // We only increment the pidCounter itself if we succefully create the process
	pcb->mainthread = tcb;
	tcb->cpuctx = (CpuCtx*)(tcb+1); // Cpu state is right after the process data
	KERNEL_DEBUG("PCB %s: pid=%u, MainThread ctx=0x%X",
		pcb->info.name, pcb->info.pid, tcb->cpuctx );

	//
	// Setup heap area (for stack, dynamic memory, and shared data, stc)
	//
	HeapInfo heapInfo;
	bool ret = prc_setupMemory(pcb, stackSize, heapSize, 0, &heapInfo);
	if (!ret)
		goto out3;
	
	pcb->info.heap_start = heapInfo.start;
	pcb->info.heap_size = heapInfo.size;
		
	// Allocate handle the the main thread
	tcb->handle = handles_create(pcb->info.pid, kHandleType_Thread, tcb);
	if (tcb->handle==INVALID_HANDLE)
		goto out3;
	
	//
	// Initialize TCB
	tcb->pcb = pcb;
	tcb->state = TCB_STATE_READY;
	// setup registers
	tcb->cpuctx->gregs[0] = (uint32_t)entryfunc;
	tcb->cpuctx->gregs[CPU_REG_DS] = (uint32_t)pcb->ds;
	tcb->cpuctx->gregs[CPU_REG_SP] = (uint32_t)tcb->stackTop;
	tcb->cpuctx->gregs[CPU_REG_PC] = (uint32_t)app_startup;
	tcb->cpuctx->flags =
		0x00000000 |
		(privileged ? (1<<CPU_FLAGSREG_SUPERVISOR) : 0) |
		MMU_KEY(pcb->info.pid, pcb->info.pid, pcb->info.pid);
		
	
	// Setup the links
	linkedlist_addAfter(rootpcb.previous, pcb);
	linkedlist_addAfter(rootthread.previous, tcb);
	
	// Thread is ready to run
	tcb_enqueue(tcb, &krn.tcbReady);
	
	queue_create(&tcb->msgqueue, sizeof(ThreadMsg), 0);
	
	// We only increment this if we Successfully created the process, so it
	// doesn't waste IDs
	pidCounter++;
	
	return pcb;
	
	out3:
		free(tcb);
	out2:
		free(pcb);
	out1:
		return NULL;
}

PCB* prc_find(const char* name)
{
	LINKEDLIST_FOREACH(PCB,&rootpcb,
	{
		if (strcmp(it->info.name,name)==0)
			return it;
	});
	
	return NULL;
}

PCB* prc_findByPID(uint8_t pid)
{
	LINKEDLIST_FOREACH(PCB,&rootpcb,
	{
		if (it->info.pid == pid)
			return it;
	});

	return NULL;
}

void prc_deletethread(TCB* tcb)
{
	if (tcb==&rootthread)
		return;

	KERNEL_DEBUG("Deleting thread %Xh", tcb);

	// If this is not the process's main thread, than it has some pages on its
	// own for the stack, so free those pages
	if (tcb!=tcb->pcb->mainthread) {
		mmu_freePagesRange(
			ADDR_TO_PAGE(tcb->stackBottom),
			SIZE_TO_PAGES(tcb->stackTop - tcb->stackBottom));
	}
		
	// If we are destroying the currently interrupted thread, as for example
	// because the thread terminated and asked the kernel to be destroyed,
	// then we need to ask the scheduler to pick another thread right here, 
	// otherwise the kernel will try resuming that thread
	// Setting thread as done, so the scheduler does not pick this thread again
	tcb_enqueue(tcb, NULL);
	if (tcb->state==TCB_STATE_BLOCKED) {
		if (tcb->wait.type==TCB_WAIT_TYPE_SLEEP) {
			priorityQueue_delete(&krn.timedEvents, &timedEvent_removeThreads,
				tcb);
		}
	}
	tcb->state = TCB_STATE_DONE;
	linkedlist_remove(tcb);

	if (krn.interruptedTcb==tcb)
		krn_pickNextTcb();
		
	free(tcb);
}

void prc_deleteImpl(PCB* pcb)
{
	kernel_check(pcb!=&rootpcb);
	bool isCurrentPCB = (krn.interruptedTcb->pcb==pcb);
	KERNEL_DEBUG("Deleting process %s: %Xh...", pcb->info.name, pcb);
	
	handles_destroyPrcHandles(pcb->info.pid);
	
	mmu_freePages(pcb->info.pid);
	linkedlist_remove(pcb);
	prc_deletethread(pcb->mainthread);
	free(pcb);
	
	KERNEL_DEBUG("... process deleted");
	if (isCurrentPCB) {
		krn_pickNextTcb();
	}
	
	// If this process has focus, then we need to give focus to another one
	if (krn.focusedPcb==pcb) {
		prc_setDefaultFocus();
	}

}

void handles_thread_dtr(void* data)
{
	if (!data) return;
	TCB* tcb = (TCB*)data;
	if (tcb==tcb->pcb->mainthread) {
		prc_deleteImpl(tcb->pcb);
	} else {
		prc_deletethread((TCB*)data);
	}
}

void prc_delete(PCB* pcb)
{
	kernel_check(pcb!=&rootpcb);
	handles_destroy(pcb->mainthread->handle, pcb->info.pid);
	krn.info.mem_available = mmu_countPages(PID_NONE)*MMU_PAGE_SIZE;
}

void prc_calcCpuStats(PCB* pcb)
{
	double timeInPrc = pcb->stats.cpuTime - pcb->stats.cpuTimeMarker;
	double timeInSwi = pcb->stats.cpuTimeswi - pcb->stats.cpuTimeswiMarker;
	double total = hw_clk_currSecs - pcb->stats.lastUpdateTime;
	total = 100/total;
	pcb->stats.lastUpdateTime = hw_clk_currSecs;
	
	pcb->info.cpu = (uint8_t)( ((timeInPrc + timeInSwi) * total) + 0.5f );
	pcb->info.cpuswi = (uint8_t)( (timeInSwi*total) + 0.5f );

	pcb->stats.cpuTimeMarker = pcb->stats.cpuTime;
	pcb->stats.cpuTimeswiMarker = pcb->stats.cpuTimeswi;
}

PCB* prc_setDefaultFocus(void)
{
	krn.focusedPcb = NULL;
	LINKEDLIST_FOREACH(PCB, krn.kernelPcb, {
		if (it->canvas) {
			if (prc_setFocus(it))
				return it;
		}
	});
	
	return NULL;
}

bool prc_setFocus(PCB* pcb)
{
	if (!pcb->canvas || krn.focusedPcb==pcb || pcb->info.flags&APPFLAG_NOFOCUS)
		return false;
	
	// Notify the currently focused process that it is losing focus
	if (krn.focusedPcb) {
		prc_postThreadMessage(krn.focusedPcb->mainthread, MSG_FOCUSLOST,0,0);
	}
	
	// Set focus to the new process
	hw_scr_mapBuffer(pcb->canvas);
	prc_postThreadMessage(pcb->mainthread, MSG_FOCUSGAINED,0,0);
	krn.focusedPcb = pcb;
	KERNEL_DEBUG("Focus set to %s", pcb->info.name);
	return true;
}

void tcb_enqueue(TCB* tcb, Queue32* to)
{
	if (tcb->queue) {
		queue32_delete(tcb->queue, (int)tcb);
	}
	
	if (to) {
		queue32_push(to, (int)tcb);
		tcb->queue = to;
	} else {
		tcb->queue = NULL;
	}
}


#ifdef DEBUG
void prc_logAllThreads(const char* title)
{
	static const char* states[4] = {"READY","BLOCKED","DONE","KERNEL"};
	static const char* waitTypes[2] = {"SLEEP","WAITFORMSG"};
	
	KERNEL_DEBUG("**** prc_logAllThreads: %s", title);
	LINKEDLIST_FOREACH(TCB, krn.idlethread, {
		KERNEL_DEBUG("    %s:%d. State=%s WaitType=%s",
			it->pcb->info.name,
			it->handle,
			states[it->state],
			waitTypes[it->wait.type]
			);
	});	

}
#endif
