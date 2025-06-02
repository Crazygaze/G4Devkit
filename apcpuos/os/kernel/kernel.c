#include "os_config.h"
#include "kernel.h"
#include "mmu.h"

#include "boot/boot.h"
#include "hw/hwcpu.h"
#include "hw/hwclk.h"
#include "hw/hwnic.h"
#include "utils/bitops.h"
#include "osapps/osapps.h"

#include <stdc_init.h>
#include <stdlib.h>

Kernel krn;

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
		OS_VER("No threads ready to run. Switching to idle task");
		krn.currTcb = krn.idleTcb;
	}
	
	// #TODO : Setup TLS
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

	PCB* pcb = prc_create(info->name, info->startFunc, info->privileged,
		info->stacksize, info->heapsize);
	if (!pcb) {
		OS_ERR("Failed to create app '%s'", info->name);
		return NULL;
	}
	
	// #TODO : Remove this. Is just temporary while I don't have system calls
	//pcb->mainthread->ctx.crregs[CPU_CRREG_FLAGS] |= CPU_CRREG_FLAGS_S;
	//pcb->mainthread->ctx.crregs[CPU_CRREG_IRQMSK] = 0x00000002; // Enable Clock only
	//pcb->mainthread->ctx.crregs[CPU_CRREG_IRQMSK] = 0x00000012; // Enable Clock only
	
	pcb->info.flags = info->flags;
	return pcb;
}

/*
 * Initializes the kernel and returns the first context to switch to
 */
FullCpuCtx* krn_init()
{
	stdc_setLogFunc(hwnic_sendDebug);
	mmu_init();
	
	queue32_create(&krn.tcbReady, 32);
	krn_initTimedEvents();
	
	boot_ui_init();
	boot_ui_displayOSLogo();
	
	hw_initAll();
	
	hwclk_addCallback(0, krn_taskScheduler, NULL);
	hwclk_startTimer(0, KERNEL_QUANTUM, true, true);
	
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
 * \param interruptedCtxPa Physical address where the interrupted context as saved to
 * \param lastDecodedPC Last decoded pc value.
 * \param info Extra info, 
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

void krn_addTimedEvent(double execTime, KrnTimedEventFunc func, void* data1,
	void* data2, void* data3)
{
	KrnTimedEvent evt;
	evt.execTime = execTime;
	evt.func = func;
	evt.data1 = data1;
	evt.data2 = data2;
	evt.data3 = data3;
	
	bool res = pqueue_KrnTimedEvent_push(&krn.timedEvents, &evt);
	krnassert(res);
}

#pragma dontwarn 323
/*!
 * Exectes any due events
 */
static void krn_checkTimedEvents(void)
{
	const KrnTimedEvent* evt;
	while ((evt = pqueue_KrnTimedEvent_peek(&krn.timedEvents)) &&
		   (evt->execTime <= krn.intrCurrSecs)) {
		pqueue_KrnTimedEvent_pop(&krn.timedEvents, NULL);
		evt->func(evt->data1, evt->data2, evt->data3);
	}
}
#pragma popwarn
