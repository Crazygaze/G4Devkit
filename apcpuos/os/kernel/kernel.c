#include "os_config.h"
#include "kernel.h"
#include "mmu.h"

#include "boot/boot.h"
#include "hw/hwcpu.h"
#include "hw/hwclk.h"
#include "hw/hwnic.h"
#include "utils/bitops.h"
#include "osapps/osapps.h"
#include "handles.h"

#include <stdc_init.h>
#include <stdlib.h>

Kernel krn;

// This is defined in assembly, in the .rodata
extern u32* appTlsSlots;

void krn_setCurrTCBTlsSlots(void)
{
	int page = MMU_ADDR_TO_PAGE((u32)&appTlsSlots);
	volatile u32* pte = &((u32*)hwcpu_get_crpt())[1+page];
	u32 original = *pte;
	// Give temporary write access, because it is in the .rodata section
	*pte |= MMU_PTE_W;
	appTlsSlots = krn.currTcb->tlsSlotsPtr;
	*pte = original;
}

/*
 * TODO
 * - When entering the kernel, set irqtsk to a context that will handle double
 *   fault (kernel crash)
 */
void krn_pickNextTcb(void)
{
	// Get the next tcb to run, if any is available
	if (queue32_pop(&krn.tcbReady, (u32*)&krn.currTcb)) {
		// Put the one we picked back at the end of the queue
		queue32_push(&krn.tcbReady, (u32)krn.currTcb);
	} else {
		// No tcb was ready to run, so we select the idle tcb
		if (krn.currTcb != krn.idleTcb) {
			OS_VER("No threads ready to run. Switching to idle task");
			krn.currTcb = krn.idleTcb;
		}
	}
	
	krn_setCurrTCBTlsSlots();
}

static void krn_initTimedEvents(void);
static void krn_checkTimedEvents(void);

bool krn_taskScheduler(void* userData)
{
	krn.schedulerTicks++;
	OS_VER("%s: %u", __func__, krn.schedulerTicks);
	krn_checkTimedEvents();
	krn_pickNextTcb();
	return true;
}

PCB* krn_launchKernelApp(KernelAppID id)
{
	const KernelAppInfo* info = &krnApps[id];

	PCB* pcb = prc_createPCB(info->name, info->startFunc, info->privileged,
		info->stacksize, info->heapNPages);
	if (!pcb) {
		OS_ERR("Failed to create app '%s'", info->name);
		return NULL;
	}
	
	pcb->info.flags = info->flags;
	return pcb;
}

static void krn_logStackUse(void* data0, void* data1, void *data2)
{
	OS_LOG("Kernel stack: %u/%u", mmu_calcKrnUsedStack(), mmu_getKrnStackSize());
	krn_addTimedEvent(krn.intrCurrSecs + 10.0f, krn_logStackUse, NULL, NULL, NULL);
}

/*
 * Initializes the kernel and returns the first context to switch to
 */
FullCpuCtx* krn_init()
{
	stdc_setLogFunc((LibCDebugLogFunc)hwnic_sendDebug);
	mmu_init();
	handles_init();
	
	queue32_create(&krn.tcbReady, 32);
	krn_initTimedEvents();
	
	boot_ui_init();
	boot_ui_displayOSLogo();
	
	hw_initAll();
	
	hwclk_addCallback(0, krn_taskScheduler, NULL);
	hwclk_startTimer(0, KERNEL_QUANTUM, true, true);

	krn.intrCurrSecs = hwclk_getSecsSinceBoot();
	// Kick start the timed event to log the stack usage.
	krn_logStackUse(NULL, NULL, NULL);
	
	// Disable IRQ generation on send (when the outgoing buffer is empty)
	hwnic_setIRQMode(false, true);
	
	//
	// Set where to save the kernel context
	hwcpu_set_crtsk(&krn.krnCtx);
	
	//
	// Set the irq handler context as also being the kernel
	hwcpu_set_crirqtsk(&krn.krnCtx);
	
	PCB* idlePCB = krn_launchKernelApp(kKernelAppID_Idle);
	krnverify(idlePCB);
	
	PCB* helloworldPCB = krn_launchKernelApp(kKernelAppID_HelloWorld);
	
	krn.idleTcb = idlePCB->mainthread;
	krn.currTcb = idlePCB->mainthread;
	
	// Before passing control back to a process, we set the kernel's page table
	// as being the same as the process, so that when we get an interrupt, the
	// the page table to to use for the interrupt handler is the interrupted
	// process's page table.
	hwcpu_set_crpt((u32)krn.currTcb->pcb->pt->data);
	
	return &krn.currTcb->ctx;
}

/*!
 * Forces a crash by acessing invalid memory.
 * This is just for test kernel crashes
 */
void krn_doCrash(void)
{
	static int* ptr = 0;
	int* p = ptr - 1;
	*p = 0;
}

/*!
 * Whenever an interrupt happens (cpu exception or IRQ), execution switches to
 * the irq handling context and this function is called from assembly.
 * Signature for a Driver's irq handler.
 * \param reason Interrupt reason
 * \param interruptedCtx Physical address where the interrupted context as saved to
 * \param data1 Last decoded pc value.
 * \param data2 The failed address (if applicable)
 */
FullCpuCtx* krn_irqHandler(u32 reason, FullCpuCtx* interruptedCtx, u32 data1, u32 data2)
{
	// Setting to NO irq context, so that it will use the default blue screen
	// This signifies a double fault (kernel crash)
	// We could setup things so the crash shows OS specific things, but this is
	// good enough for now.
	hwcpu_set_crirqtsk(NULL);
	
	krn.irqCount++;
	
	//OS_LOG("IRQ Handler %u: BEGIN", count);
	krnassert(interruptedCtx == &krn.currTcb->ctx);
	TCB* previousTCB = krn.currTcb;
	
	krn.intrCurrSecs = hwclk_getSecsSinceBoot();
	
	krn.intrData.reason = reason;
	krn.intrData.data1 = data1;
	krn.intrData.data2 = data2;

	int bus = reason & HWBUS_MASK;
	hw_handleIRQ(bus);
	
	// Before passing control back to a process, we set the kernel's page table
	// as being the same as the process, so that when we get an interrupt, the
	// interrupt's context is using the current's process page table
	hwcpu_set_crpt((u32)krn.currTcb->pcb->pt->data);
	
	if (krn.currTcb != previousTCB) {
		OS_LOG("Switching to process '%p:%s', TCB %p", krn.currTcb->pcb,
			krn.currTcb->pcb->info.name, krn.currTcb);
	}
	
	// Right before return, we set the irq ctx to be the kernel itself
	hwcpu_set_crirqtsk(&krn.krnCtx);

	return &krn.currTcb->ctx;
}

////////////////////////////////////////////////////////////////////////////////
//
//               Kernel Timed Events functionality
//
////////////////////////////////////////////////////////////////////////////////

/*!
 * Used as a callback by PQueue to sort entries
 */
static int timedEventCmp(const KrnTimedEvent* a, const KrnTimedEvent* b)
{
	if (a->execTime < b->execTime)
		return 1;
	if (a->execTime > b->execTime)
		return -1;
	else
		return 0;
}

/*!
 * Initializes the timed events system
 */
static void krn_initTimedEvents(void)
{
	pqueue_KrnTimedEvent_create(&krn.timedEvents, 4, timedEventCmp);
}

bool krn_addTimedEvent(double execTime, KrnTimedEventFunc func, void* data0,
	void* data1, void* data2)
{
	KrnTimedEvent evt;
	evt.execTime = execTime;
	evt.func = func;
	evt.data0 = data0;
	evt.data1 = data1;
	evt.data2 = data2;
	
	bool res = pqueue_KrnTimedEvent_push(&krn.timedEvents, &evt);
	if (!res) {
		OS_ERR("Out of memory");
	}
	return res;
}

#pragma dontwarn 323
/*!
 * Exectes any due events
 */
static void krn_checkTimedEvents(void)
{
	const KrnTimedEvent* evtPtr;
	while ((evtPtr = pqueue_KrnTimedEvent_peek(&krn.timedEvents)) &&
		   (evtPtr->execTime <= krn.intrCurrSecs)) {
		   
		// Copy the data to the local variable BEFORE calling the function,
		// because the function itselt might want to add more events to the
		// queue
		KrnTimedEvent evt;
		pqueue_KrnTimedEvent_pop(&krn.timedEvents, &evt);
		evt.func(evt.data0, evt.data1, evt.data2);
	}
}
#pragma popwarn
