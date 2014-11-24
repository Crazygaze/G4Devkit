#include "multitasking/process.h"
#include "kernel/mmu.h"
#include "kernel/kerneldebug.h"
#include "syscalls.h"
#include "kernel/kernel.h"
#include "hw/hwscreen.h"
#include "hw/hwnic.h"
#include "hw/hwclock.h"
#include "hw/hwdisk.h"
#include "kernel/handles.h"
#include "appslist.h"
#include "appsdk/kernel_shared/txtui_shared.h"

static bool check_user_ptr(struct PCB *pcb, MMUMemAccess access, void* addr,
	size_t size)
{
	if (mmu_check_user(pcb, access, addr, size))
		return TRUE;

	KERNEL_DEBUG(
		"%s passed a non-user space pointer to a system call. Ptr %Xh, Size %u",
		pcb->info.name, addr, size);
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
//				Process Management
////////////////////////////////////////////////////////////////////////////////

bool syscall_createProcess(void)
{
	PCB* pcb = krn.interruptedTcb->pcb;
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	
	AppInfo* info = (AppInfo*) regs[0];

	// Make sure the process can write to the address it has given us
	if (!check_user_ptr(pcb, kMMUAccess_Write, info, sizeof(*info)))
		return FALSE;

	prc_giveAccessToKernel(pcb, true);
	PCB * pcb_created = prc_create(info->name, info->startFunc, info->privileged, info->stacksize, info->memsize);
	if (pcb_created == NULL){
		regs[0] = 0;
		
		prc_giveAccessToKernel(pcb, false);
		return FALSE;
	}
	
	prc_giveAccessToKernel(pcb_created, true);
	pcb_created->info.flags = info->flags;
	pcb_created->info.cookie = info->cookie;
	
	prc_giveAccessToKernel(pcb_created, false);
	prc_giveAccessToKernel(pcb, false);
	
	regs[0] = pcb_created->info.pid;
	
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
//				Process Control
////////////////////////////////////////////////////////////////////////////////

bool syscall_yield(void)
{
	// Yield means the thread gives away the remaining of the cpu time.
	krn_pickNextTcb();
	return TRUE;
}

bool syscall_sleep(void)
{
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	int ms = regs[0]; // sleep duration in milliseconds
	prc_putThreadToSleep(krn.interruptedTcb, regs[0]);	
	// Grab the next thread to run
	krn_pickNextTcb();
	return TRUE;
}

bool syscall_getStackSize(void)
{
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	regs[0] = krn.interruptedTcb->stackTop - krn.interruptedTcb->stackBottom;
	return TRUE;
}

bool syscall_getUsedStackSize(void)
{
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	regs[0] = (u32)krn.interruptedTcb->stackTop - regs[CPU_REG_SP];
	return TRUE;
}

bool syscall_getThreadHandle(void)
{
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	regs[0] = (u32)krn.interruptedTcb->handle;
	return TRUE;
}

bool syscall_createThread(void)
{
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	uint8_t pid = krn.interruptedTcb->pcb->info.pid;

	TCB* tcb = prc_createThread(
		krn.interruptedTcb->pcb,
		// Thread entry function wrapper for the appsdk
		(ThreadEntryFuncWrapper)regs[0],
		// Real thread entry function
		(ThreadEntryFunc)regs[1],
		regs[2], // stackSize
		(void*)regs[3], // userData
		false // No Supervisor mode
		);
		
	if (tcb) {	
		regs[0] = (u32)tcb->handle;
	} else {
		regs[0] = INVALID_HANDLE;
	}
	
	return TRUE;
}

bool syscall_setThreadTLS(void)
{
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	uint8_t pid = krn.interruptedTcb->pcb->info.pid;
	uint32_t* tlsVarPtr = (uint32_t*)regs[0];
	uint32_t tlsVarValue = regs[1];
	PCB* pcb = krn.interruptedTcb->pcb;

	// Make sure the process can write to the address it has given us
	if (!check_user_ptr(pcb, kMMUAccess_Write, tlsVarPtr, sizeof(*tlsVarPtr)))
		return FALSE;
	
	krn.interruptedTcb->tlsVarPtr = tlsVarPtr;
	krn.interruptedTcb->tlsVarValue = tlsVarValue;
		
	return TRUE;
}

bool syscall_closeHandle(void)
{
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	HANDLE h = (HANDLE)regs[0];
	bool res = handles_destroy(h, krn.interruptedTcb->pcb->info.pid);
	regs[0] = res;
	return TRUE;
}

bool syscall_getMessage(void)
{
	PCB* pcb = krn.interruptedTcb->pcb;
	TCB* tcb = krn.interruptedTcb;
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	ThreadMsg* msg = (ThreadMsg*)regs[0];
	bool waitForMessage = regs[1];

	// Make sure the process can write to the address it has given us
	if (!check_user_ptr(pcb, kMMUAccess_Write, msg, sizeof(*msg)))
		return FALSE;

	prc_giveAccessToKernel(pcb, true);
	bool res = queue_pop(&tcb->msgqueue, msg);
	prc_giveAccessToKernel(pcb, false);

	// If there is a message, then we pass control back to the application.
	// Otherwise, we put this thread to sleep until there is a message
	if (res) {
		regs[0] = TRUE;
	} else {
		if (waitForMessage) {
			regs[0] = TRUE;
			//
			// Block the thread until a message is posted to its message queue
			//
			//KERNEL_DEBUG("Thread %8s:%u blocked waiting for message",
			//	tcb->pcb->stats.info.name, tcb->handle);
			// Remove from the ready queue
			tcb_enqueue(krn.interruptedTcb, NULL);
			// and set it to blocked
			tcb->state = TCB_STATE_BLOCKED;
			tcb->wait.type = TCB_WAIT_TYPE_WAITING_FOR_MSG;		
			// Since R0 is used both as a parameter, and return value,
			// we need to remember where the thread wants us to copy the
			// message to once it arrives.
			tcb->wait.d.msgDst = msg;
			// Grab the next thread to run
			krn_pickNextTcb();			
		} else {
			regs[0] = FALSE;
		}
	}

	return TRUE;
}

bool syscall_postMessage(void)
{
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	HANDLE threadHandle = (HANDLE)regs[0];
	uint32_t msgId = regs[1];
	uint32_t param1 = regs[2];
	uint32_t param2 = regs[3];
	
	// NOTE: A process can only post messages to threads it owns, otherwise it
	// could crash some other process by exploiting how it handles those messages.
	TCB* tcb = handles_getData(threadHandle, krn.interruptedTcb->pcb->info.pid,
		kHandleType_Thread);
	if (tcb) {
		regs[0] = prc_postThreadMessage(tcb, msgId, param1, param2);
	} else {
		regs[0] = FALSE;
	}
	
	return TRUE;
}

bool syscall_setTimer(void)
{
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	uint32_t timerId = regs[0];
	uint32_t ms = regs[1];
	bool repeat = (bool)regs[2];
	regs[0] = prc_setThreadTimer(krn.interruptedTcb, timerId, ms, repeat);
	return TRUE;
}

bool syscall_setFocusTo(void)
{
	//PCB* pcb = krn.interruptedTcb->pcb;
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	
	uint8_t pid = regs[0];
	PCB* pcb=NULL;
	if (pid==0) {
		pcb = krn.interruptedTcb->pcb;
	} else {
		pcb = prc_findByPID(pid);
	}
	
	if (pcb) {
		regs[0] = prc_setFocus(pcb) ? TRUE : FALSE;
	} else {
		regs[0] = FALSE;
	}
	
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
//				System Information
////////////////////////////////////////////////////////////////////////////////
bool syscall_getProcessCount(void)
{
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	regs[0] = linkedlist_size((LinkedListNode*)krn.kernelPcb);
	return TRUE;
}

bool syscall_getProcessInfo(void)
{
	PCB* pcb = krn.interruptedTcb->pcb;
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	ProcessInfo* dst = (ProcessInfo*)regs[0];
	bool updateStats = (bool)regs[1];

	// Check if the process can write to the address it has given us
	if (!check_user_ptr(pcb, kMMUAccess_Write, dst, sizeof(*dst)))
		return FALSE;

	if (updateStats)
		prc_calcCpuStats(pcb);
	
	prc_giveAccessToKernel(pcb, true);
	memcpy(dst, &pcb->info, sizeof(*dst));
	prc_giveAccessToKernel(pcb, false);

	return TRUE;
}

bool syscall_getOSInfo(void)
{
	PCB* pcb = krn.interruptedTcb->pcb;
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	OSInfo* dst_osinfo = (OSInfo*)regs[0];
	ProcessInfo* dst_prc = (ProcessInfo*)regs[1];
	int dst_size = regs[2];
	uint32_t flags = regs[3];

	// Make sure the process can write to the address it has given us
	if (!check_user_ptr(pcb, kMMUAccess_Write, dst_osinfo, sizeof(*dst_osinfo)))
		return FALSE;
	if (!check_user_ptr(pcb, kMMUAccess_Write, dst_prc,
						dst_size*sizeof(*dst_prc)))
		return FALSE;

	if (flags & OSINFO_KERNELMEM) {
		_getmemstats(&krn.info.krn_mem_used, &krn.info.krn_mem_free,
			&krn.info.krn_mem_maxAlloc);
	}
	
	// Update process stats
	// This is done as a seperate step from the copying itself, so I don't
	// need to give access to the kernel yet.
	// Also, giving access to the kernel before calculating the stats would
	// skew the stats, as the kernel would have more pages than it really does
	if (flags & OSINFO_CPU) {
		
		krn.info.cpu_usage = 0;
		LINKEDLIST_FOREACH(PCB, krn.kernelPcb, {
			prc_calcCpuStats(it);
			krn.info.cpu_usage += it->info.cpu;
		});	

		// Remove the cpu time used by the idle process
		krn.info.cpu_usage -= krn.idlethread->pcb->info.cpu;
		if (krn.info.cpu_usage>100)
			krn.info.cpu_usage = 100;
	}

	krn.info.focusedPid = krn.focusedPcb ? krn.focusedPcb->info.pid : 0;

	//
	// Copy the stats to the application space
	prc_giveAccessToKernel(pcb, true);
	memcpy(dst_osinfo, &krn.info, sizeof(krn.info));
	int done=0;
	LINKEDLIST_FOREACH(PCB, krn.kernelPcb, {			
		memcpy(dst_prc, &(it->info), sizeof(*dst_prc));
		dst_prc++;
		done++;
		if (done==dst_size)
			break;			
	});
	prc_giveAccessToKernel(pcb, false);
	
	regs[0] = done;
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
//				System Information
////////////////////////////////////////////////////////////////////////////////
// TODO : Remove this. Applications will have their own screen buffer
bool syscall_getScreenBuffer(void)
{
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	regs[0] = (int)hw_scr_getScreenBuffer();
	return TRUE;
}
bool syscall_getScreenXRes(void)
{
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	regs[0] = (int)hw_scr_getScreenXRes();
	return TRUE;
}
bool syscall_getScreenYRes(void)
{
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	regs[0] = (int)hw_scr_getScreenYRes();
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
//				Disk Drive
////////////////////////////////////////////////////////////////////////////////

bool syscall_diskDriveRead(void)
{
	PCB* pcb = krn.interruptedTcb->pcb;
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	
	u32 diskNum = (u32)regs[0];
	u32 sectorNum = (u32)regs[1];
	char * data = (char *)regs[2];
	int size = (int) regs[3];
	
	if (!check_user_ptr(pcb, kMMUAccess_Write, data, size))
		return FALSE;
	
	prc_giveAccessToKernel(pcb, true);
	hw_dkc_read_sync(diskNum, sectorNum, data, size);
	prc_giveAccessToKernel(pcb, false);
	
	return TRUE;
}

bool syscall_diskDriveWrite(void)
{
	PCB* pcb = krn.interruptedTcb->pcb;
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	
	u32 diskNum = (u32)regs[0];
	u32 sectorNum = (u32)regs[1];
	const char * data = (char *)regs[2];
	int size = (int) regs[3];
	
	if (!check_user_ptr(pcb, kMMUAccess_Read, data, size))
		return FALSE;
	
	prc_giveAccessToKernel(pcb, true);
	hw_dkc_write_sync(diskNum, sectorNum, data, size);
	prc_giveAccessToKernel(pcb, false);
	
	return TRUE;
}

bool syscall_diskDriveSetFlags(void)
{
	PCB* pcb = krn.interruptedTcb->pcb;
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	
	u32 diskNum = (u32)regs[0];
	u32 flags = (u32)regs[1];
	
	hw_dkc_setCustomFlags(diskNum, flags);
	
	return TRUE;
}

bool syscall_diskDriveGetFlags(void)
{
	PCB* pcb = krn.interruptedTcb->pcb;
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	
	u32 diskNum = (u32)regs[0];

	regs[0] = hw_dkc_getFlags(diskNum);
	
	return TRUE;
}

bool syscall_diskDriveGetInfo(void)
{
	PCB* pcb = krn.interruptedTcb->pcb;
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	
	u32 diskNum = (u32)regs[0];
	DISK_INFO * disk_info = (DISK_INFO *)regs[1];
	
	if (!check_user_ptr(pcb, kMMUAccess_Write, disk_info, sizeof(*disk_info)))
		return FALSE;
	
	prc_giveAccessToKernel(pcb, true);
	*disk_info = hw_dck_getDiskInfo(diskNum);
	prc_giveAccessToKernel(pcb, false);
		
	regs[0] = 0;
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
//				Rendering
////////////////////////////////////////////////////////////////////////////////

bool syscall_setCanvas(void)
{
	PCB* pcb = krn.interruptedTcb->pcb;
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	
	void* canvas = (void*)regs[0];
	u32 size = regs[1];
	
	if (
		size!= hw_scr_getBufferSize() ||
		!check_user_ptr(pcb, kMMUAccess_Write, canvas, size))
	{
		regs[0] = FALSE;
	} else {
		regs[0] = TRUE;
		pcb->canvas = canvas;
		if (!krn.focusedPcb) {
			prc_setFocus(pcb);
		}
	}

	return TRUE;
}

bool syscall_setStatusBar(void)
{
	PCB* pcb = krn.interruptedTcb->pcb;
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	
	const char* str = (const char*)regs[0];
	size_t size = regs[1];
	
	if (!check_user_ptr(pcb, kMMUAccess_Write, str, size)
		|| krn.focusedPcb==NULL || krn.focusedPcb->canvas==NULL) {
		regs[0] = FALSE;
	} else {
		prc_giveAccessToKernel(krn.focusedPcb, true);
		prc_giveAccessToKernel(pcb, true);
		_txtui_setStatusBar(krn.focusedPcb->canvas, kTXTCLR_BRIGHT_WHITE,
			kTXTCLR_BLUE, str);
		prc_giveAccessToKernel(pcb, false);
		prc_giveAccessToKernel(krn.focusedPcb, false);
		regs[0] = TRUE;
	}

	return TRUE;
}

bool syscall_processScreenshot(void)
{
	PCB* pcb = krn.interruptedTcb->pcb;
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	
	uint8_t  pid = regs[0];
	void* dst = (const char*)regs[1];
	int size = regs[2];
	
	// Clamp the size to the size of the screen, otherwise the asking
	// application could pass a size bigger than the screen, causing the kernel
	// to just copy private data that might be after the source's screen buffer.
	int screenSize = hw_scr_getBufferSize();
	size = min(size, screenSize);
	
	PCB* sourcePcb = prc_findByPID(pid);
	
	if ( !sourcePcb || !sourcePcb->canvas ||
		!check_user_ptr(pcb, kMMUAccess_Write, dst, size)
		) {
		regs[0] = 0;
	} else {
		// Need to give the kernel access to both processes
		prc_giveAccessToKernel(sourcePcb, true);
		prc_giveAccessToKernel(pcb, true);
		memcpy(dst, sourcePcb->canvas, size);
		prc_giveAccessToKernel(pcb, false);
		prc_giveAccessToKernel(sourcePcb, false);
		regs[0] = size;
	}

	return TRUE;
}


////////////////////////////////////////////////////////////////////////////////
//				Debug
////////////////////////////////////////////////////////////////////////////////

bool syscall_outputDebugString(void)
{
	PCB* pcb = krn.interruptedTcb->pcb;
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	
	// Grab user string to print, and force zero termination, to avoid possible
	// exploits
	char* userstr = (char*)regs[0];
	uint32_t size = regs[1];
	if (!check_user_ptr(pcb, kMMUAccess_Read, userstr, size))
		return FALSE;
	
	prc_giveAccessToKernel(pcb, true);
	hw_nic_sendDebugV("%s: %s", pcb->info.name, userstr);
	prc_giveAccessToKernel(pcb, false);

	return TRUE;
}

krn_syscallFunc krn_syscalls[kSysCall_Max] =
{
	// 
	// Process Management
	//
	
	syscall_createProcess,
	
	//
	// Process control
	//
	syscall_yield,
	syscall_sleep,
	syscall_getStackSize,
	syscall_getUsedStackSize,
	syscall_getThreadHandle,
	syscall_createThread,
	syscall_setThreadTLS,
	syscall_closeHandle,
	syscall_getMessage,
	syscall_postMessage,
	syscall_setTimer,
	syscall_setFocusTo,
	
	//
	// System information
	//
	syscall_getProcessCount,
	syscall_getProcessInfo,
	syscall_getOSInfo,
	
	//
	// Hardware
	//
	syscall_getScreenBuffer, // TODO : Remove this. Applications will have their own screen buffer
	syscall_getScreenXRes,
	syscall_getScreenYRes,
	
	// 
	// Disk Drive
	//

	syscall_diskDriveRead,
	syscall_diskDriveWrite,
	syscall_diskDriveSetFlags,
	syscall_diskDriveGetFlags,
	syscall_diskDriveGetInfo,

	//
	// Rendering
	//
	syscall_setCanvas,
	syscall_setStatusBar,
	syscall_processScreenshot,
	
	//
	// Debug
	//
	syscall_outputDebugString,
};

