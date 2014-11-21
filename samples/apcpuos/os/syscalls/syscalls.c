#include "multitasking/process.h"
#include "kernel/mmu.h"
#include "kernel/kerneldebug.h"
#include "syscalls.h"
#include "kernel/kernel.h"
#include "hw/hwscreen.h"
#include "hw/hwnic.h"
#include "hw/hwclock.h"
#include "kernel/handles.h"
#include "appsdk/kernel_shared/txtui_shared.h"

#include "extern/fatfs/src/diskio.h"
#include "extern/fatfs/src/ff.h"

#define DEBUG_FATFS 1

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

void printFResult(char * info, FRESULT value)
{
	switch (value){
		case FR_OK:
			LOG ("%s: FR_OK", info);
			break;
		case FR_DISK_ERR:
			LOG ("%s: FR_DISK_ERR", info);
			break;
		case FR_INT_ERR:
			LOG ("%s: FR_INT_ERR", info);
			break;
		case FR_NOT_READY:
			LOG ("%s: FR_NOT_READY", info);
			break;
		case FR_NO_FILE:
			LOG ("%s: FR_NO_FILE", info);
			break;
		case FR_NO_PATH:
			LOG ("%s: FR_NO_PATH", info);
			break;
		case FR_INVALID_NAME:
			LOG ("%s: FR_INVALID_NAME", info);
			break;
		case FR_DENIED:
			LOG ("%s: FR_DENIED", info);
			break;
		case FR_EXIST:
			LOG ("%s: FR_EXIST", info);
			break;
		case FR_INVALID_OBJECT:
			LOG ("%s: FR_INVALID_OBJECT", info);
			break;
		case FR_WRITE_PROTECTED:
			LOG ("%s: FR_WRITE_PROTECTED", info);
			break;
		case FR_INVALID_DRIVE:
			LOG ("%s: FR_INVALID_DRIVE", info);
			break;
		case FR_NOT_ENABLED:
			LOG ("%s: FR_NOT_ENABLED", info);
			break;
		case FR_NO_FILESYSTEM:
			LOG ("%s: FR_NO_FILESYSTEM", info);
			break;
		case FR_MKFS_ABORTED:
			LOG ("%s: FR_MKFS_ABORTED", info);
			break;
		case FR_TIMEOUT:
			LOG ("%s: FR_TIMEOUT", info);
			break;
		case FR_LOCKED:
			LOG ("%s: FR_LOCKED", info);
			break;
		case FR_NOT_ENOUGH_CORE:
			LOG ("%s: FR_NOT_ENOUGH_CORE", info);
			break;
		case FR_TOO_MANY_OPEN_FILES:
			LOG ("%s: FR_TOO_MANY_OPEN_FILES", info);
			break;
		case FR_INVALID_PARAMETER:
			LOG ("%s: FR_INVALID_PARAMETER", info);
			break;
		default:
			LOG ("%s: unexpected return value", info);
			break;
	}
}

FRESULT diskDriveMount(const TCHAR * drive_id)
{
	FATFS fs; 
	
	FRESULT res_mount = f_mount(&fs, drive_id, 0);
	
#if DEBUG_FATFS
	printFResult("f_mount", res_mount);	
#endif

	return res_mount;
}

void disk_initialize_wait(int drv){
	/*
		TODO: fix. it block kernel !!
	*/

	while (disk_initialize(drv)){
		prc_putThreadToSleep(krn.interruptedTcb, 250);	
	};
}

bool syscall_diskDriveMountDrive(void)
{
	PCB* pcb = krn.interruptedTcb->pcb;
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	const TCHAR * drive_name = (TCHAR *)regs[0];

	if (!check_user_ptr(pcb, kMMUAccess_Read, drive_name, sizeof(*drive_name)))
		return FALSE;

	prc_giveAccessToKernel(pcb, true);
	regs[0] = (int) diskDriveMount(drive_name);
	prc_giveAccessToKernel(pcb, false);
	
	return TRUE;
}

bool syscall_diskDriveMakeFAT(void)
{
	PCB* pcb = krn.interruptedTcb->pcb;
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	const TCHAR * drive_name = (TCHAR *)regs[0];
	
	if (!check_user_ptr(pcb, kMMUAccess_Read, drive_name, sizeof(*drive_name)))
		return FALSE;
	
	prc_giveAccessToKernel(pcb, true);
	/* Register work area */
	FRESULT res_mount = diskDriveMount(drive_name);
	if (res_mount != FR_OK){
		regs[0] = (int) res_mount;
		
		prc_giveAccessToKernel(pcb, false);
		
		return FALSE;
	}
	    
	/* Make FAT file system*/
	FRESULT res_mkfs = f_mkfs(drive_name, 0, 0);	
	prc_giveAccessToKernel(pcb, false);

#if DEBUG_FATFS
	printFResult("f_mkfs", res_mkfs);	
#endif

	regs[0] = (int)res_mkfs;

	return TRUE;
}

bool syscall_diskDriveOpenFile(void)
{	
	PCB* pcb = krn.interruptedTcb->pcb;
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	
	FIL * fil = (FIL *) regs[0];
	const TCHAR * file_name = (TCHAR *)regs[1];
	BYTE flags = (BYTE) regs[2];
	
	// Check if the process can write to the address it has given us
	if (!check_user_ptr(pcb, kMMUAccess_Write, fil, sizeof(*fil)))
		return FALSE;
		
	if (!check_user_ptr(pcb, kMMUAccess_Read, file_name, sizeof(*file_name)))
		return FALSE;

	prc_giveAccessToKernel(pcb, true);
	disk_initialize_wait(fil->fs->drv);
	FRESULT res = f_open(fil, file_name, flags);
	prc_giveAccessToKernel(pcb, false);
	
#if DEBUG_FATFS
	printFResult("f_open", res);	
#endif	
	
	regs[0] = (int)res;
	
	return TRUE;
}

bool syscall_diskDriveCloseFile(void)
{
	PCB* pcb = krn.interruptedTcb->pcb;
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	FIL * fil = (FIL *) regs[0];

	// Check if the process can write to the address it has given us
	if (!check_user_ptr(pcb, kMMUAccess_Write, fil, sizeof(*fil)))
		return FALSE;
		
	prc_giveAccessToKernel(pcb, true);
	disk_initialize_wait(fil->fs->drv);
	FRESULT res = f_close(fil);
	prc_giveAccessToKernel(pcb, false);
	
#if DEBUG_FATFS
	printFResult("f_close", res);	
#endif		

	regs[0] = (int)res;

	return TRUE;
}

bool syscall_diskDriveReadFromFile(void)
{
	PCB* pcb = krn.interruptedTcb->pcb;
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	FIL * fil = (FIL *) regs[0];
	void * buff = (void *) regs[1];
	UINT buff_size = (UINT)regs[2];
	UINT * byteWritten = (UINT *)regs[3];
	
	// Check if the process can write to the address it has given us
	if (!check_user_ptr(pcb, kMMUAccess_Write, fil, sizeof(*fil)))
		return FALSE;
		
	if (!check_user_ptr(pcb, kMMUAccess_Write, byteWritten, sizeof(*byteWritten)))
		return FALSE;
		
	if (!check_user_ptr(pcb, kMMUAccess_Write, buff, sizeof(*buff)))
		return FALSE;
	
	prc_giveAccessToKernel(pcb, true);
	disk_initialize_wait(fil->fs->drv);
	FRESULT res = f_read(fil, buff, buff_size, byteWritten);
	prc_giveAccessToKernel(pcb, false);	
	
#if DEBUG_FATFS
	printFResult("f_read", res);	
#endif		

	regs[0] = (int)res;

	return TRUE;
}

bool syscall_diskDriveWriteToFile(void)
{
	PCB* pcb = krn.interruptedTcb->pcb;
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	FIL * fil = (FIL *) regs[0];
	const void * buff = (void *) regs[1];
	UINT buff_size = (UINT)regs[2];
	UINT * byteWritten = (UINT *)regs[3];
	

	// Check if the process can write to the address it has given us
	if (!check_user_ptr(pcb, kMMUAccess_Write, fil, sizeof(*fil)))
		return FALSE;
		
	if (!check_user_ptr(pcb, kMMUAccess_Write, byteWritten, sizeof(*byteWritten)))
		return FALSE;
		
	if (!check_user_ptr(pcb, kMMUAccess_Read, buff, sizeof(*buff)))
		return FALSE;
		
	prc_giveAccessToKernel(pcb, true);
	disk_initialize_wait(fil->fs->drv);
	FRESULT res = f_write(fil, buff, buff_size, byteWritten);
	prc_giveAccessToKernel(pcb, false);	
	
#if DEBUG_FATFS
	printFResult("f_write", res);	
#endif		

	regs[0] = (int)res;

	return TRUE;
}

bool syscall_diskDriveFileSeek(void)
{
	PCB* pcb = krn.interruptedTcb->pcb;
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	
	FIL * fil = (FIL *) regs[0];
	DWORD offset = (DWORD)regs[1];
	
	// Check if the process can write to the address it has given us
	if (!check_user_ptr(pcb, kMMUAccess_Read, fil, sizeof(*fil)))
		return FALSE;
	
	prc_giveAccessToKernel(pcb, true);
	disk_initialize_wait(fil->fs->drv);
	FRESULT res = f_lseek(fil, offset);
	prc_giveAccessToKernel(pcb, false);
	
#if DEBUG_FATFS
	printFResult("f_lseek", res);	
#endif	

	regs[0] = (int)res;

	return TRUE;
}

bool syscall_diskDriveSync(void)
{
	PCB* pcb = krn.interruptedTcb->pcb;
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	
	FIL * fil = (FIL *) regs[0];
	
	// Check if the process can write to the address it has given us
	if (!check_user_ptr(pcb, kMMUAccess_Read, fil, sizeof(*fil)))
		return FALSE;
	
	prc_giveAccessToKernel(pcb, true);
	disk_initialize_wait(fil->fs->drv);
	FRESULT res = f_sync(fil);
	prc_giveAccessToKernel(pcb, false);
	
#if DEBUG_FATFS
	printFResult("f_sync", res);	
#endif	

	regs[0] = (int)res;

	return TRUE;
}

bool syscall_diskDriveOpenDir(void)
{
	PCB* pcb = krn.interruptedTcb->pcb;
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	
	DIR * dir = (DIR *) regs[0];
	const TCHAR * dir_name = (TCHAR *) regs[1];
	
	// Check if the process can write to the address it has given us
	if (!check_user_ptr(pcb, kMMUAccess_Write, dir, sizeof(*dir)))
		return FALSE;
		
	if (!check_user_ptr(pcb, kMMUAccess_Read, dir_name, sizeof(*dir_name)))
		return FALSE;
	
	prc_giveAccessToKernel(pcb, true);
	disk_initialize_wait(dir->fs->drv);
	FRESULT res = f_opendir(dir, dir_name);
	prc_giveAccessToKernel(pcb, false);
	
#if DEBUG_FATFS
	printFResult("f_opendir", res);	
#endif	

	regs[0] = (int)res;

	return TRUE;
}

bool syscall_diskDriveCloseDir(void)
{
	PCB* pcb = krn.interruptedTcb->pcb;
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	
	DIR * dir = (DIR *) regs[0];
	
	// Check if the process can write to the address it has given us
	if (!check_user_ptr(pcb, kMMUAccess_Read, dir, sizeof(*dir)))
		return FALSE;
	
	prc_giveAccessToKernel(pcb, true);
	disk_initialize_wait(dir->fs->drv);
	FRESULT res = f_closedir(dir);
	prc_giveAccessToKernel(pcb, false);
	
#if DEBUG_FATFS
	printFResult("f_closedir", res);	
#endif	

	regs[0] = (int)res;

	return TRUE;
}

bool syscall_diskDriveReadDir(void)
{
	PCB* pcb = krn.interruptedTcb->pcb;
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	
	DIR * dir = (DIR *) regs[0];
	FILINFO * file_info = (FILINFO *) regs[1];
	
	// Check if the process can write to the address it has given us
	if (!check_user_ptr(pcb, kMMUAccess_Write, dir, sizeof(*dir)))
		return FALSE;
		
	if (!check_user_ptr(pcb, kMMUAccess_Write, file_info, sizeof(*file_info)))
		return FALSE;
	
	prc_giveAccessToKernel(pcb, true);
	disk_initialize_wait(dir->fs->drv);
	FRESULT res = f_readdir(dir, file_info);
	prc_giveAccessToKernel(pcb, false);
	
#if DEBUG_FATFS
	printFResult("f_readdir", res);	
#endif	

	regs[0] = (int)res;

	return TRUE;
}

bool syscall_diskDriveMakeDir(void)
{
	PCB* pcb = krn.interruptedTcb->pcb;
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	const TCHAR * dir_name = (TCHAR *)regs[0];
	
	if (!check_user_ptr(pcb, kMMUAccess_Read, dir_name, sizeof(*dir_name)))
		return FALSE;
	
	prc_giveAccessToKernel(pcb, true);
	FRESULT res = f_mkdir(dir_name);
	prc_giveAccessToKernel(pcb, false);
	
#if DEBUG_FATFS
	printFResult("f_mkdir", res);	
#endif

	regs[0] = (int)res;
	return TRUE;
}

bool syscall_diskDriveDeleteFileOrDir(void)
{
	PCB* pcb = krn.interruptedTcb->pcb;
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	const TCHAR * dir_name = (TCHAR *)regs[0];
	
	if (!check_user_ptr(pcb, kMMUAccess_Read, dir_name, sizeof(*dir_name)))
		return FALSE;
	
	prc_giveAccessToKernel(pcb, true);
	FRESULT res = f_unlink(dir_name);
	prc_giveAccessToKernel(pcb, false);
	
#if DEBUG_FATFS
	printFResult("f_unlink", res);	
#endif

	regs[0] = (int)res;
	return TRUE;
}

bool syscall_diskDriveRenameOrMove(void)
{
	PCB* pcb = krn.interruptedTcb->pcb;
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	const TCHAR * old_name = (TCHAR *)regs[0];
	const TCHAR * new_name = (TCHAR *)regs[1];
	
	if (!check_user_ptr(pcb, kMMUAccess_Read, old_name, sizeof(*old_name)))
		return FALSE;
	
	if (!check_user_ptr(pcb, kMMUAccess_Read, new_name, sizeof(*new_name)))
		return FALSE;
	
	prc_giveAccessToKernel(pcb, true);
	FRESULT res = f_rename(old_name, new_name);
	prc_giveAccessToKernel(pcb, false);
	
#if DEBUG_FATFS
	printFResult("f_rename", res);	
#endif

	regs[0] = (int)res;
	return TRUE;
}

bool syscall_diskDriveGetFileInfo(void)
{
	PCB* pcb = krn.interruptedTcb->pcb;
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	const TCHAR * file_name  = (TCHAR *)regs[0];
	FILINFO * file_info = (FILINFO *)regs[1];
	
	if (!check_user_ptr(pcb, kMMUAccess_Read, file_name, sizeof(*file_name)))
		return FALSE;
	
	if (!check_user_ptr(pcb, kMMUAccess_Write, file_info, sizeof(*file_info)))
		return FALSE;
	
	prc_giveAccessToKernel(pcb, true);
	FRESULT res = f_stat(file_name, file_info);
	prc_giveAccessToKernel(pcb, false);
	
#if DEBUG_FATFS
	printFResult("f_stat", res);	
#endif

	regs[0] = (int)res;
	return TRUE;
}

bool syscall_diskDriveGetFreeClustersNum(void)
{
	PCB* pcb = krn.interruptedTcb->pcb;
	int* regs = (int*)krn.interruptedTcb->cpuctx;
	const TCHAR * drive_number = (TCHAR *) regs[0];
	DWORD * num_of_free_clusters = (DWORD *) regs[1];
	
	if (!check_user_ptr(pcb, kMMUAccess_Read, drive_number, sizeof(*drive_number)))
		return FALSE;
		
	if (!check_user_ptr(pcb, kMMUAccess_Write, num_of_free_clusters, sizeof(*num_of_free_clusters)))
		return FALSE;	
	
	prc_giveAccessToKernel(pcb, true);
	FATFS * fatfs = malloc(sizeof(FATFS));
	FATFS ** pp_ffs = & fatfs;
	FRESULT res = f_getfree(drive_number, num_of_free_clusters, pp_ffs);
	free(fatfs);
	prc_giveAccessToKernel(pcb, false);
	
#if DEBUG_FATFS
	printFResult("f_getcwd", res);	
#endif

	regs[0] = (int)res;
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
	
	syscall_diskDriveMountDrive,	// FatFS wrapper
	syscall_diskDriveMakeFAT,	
	syscall_diskDriveOpenFile,	
	syscall_diskDriveCloseFile,
	syscall_diskDriveReadFromFile,
	syscall_diskDriveWriteToFile,
	syscall_diskDriveFileSeek,
	syscall_diskDriveSync,
	syscall_diskDriveOpenDir,
	syscall_diskDriveCloseDir,
	syscall_diskDriveReadDir,
	syscall_diskDriveMakeDir,
	syscall_diskDriveDeleteFileOrDir,
	syscall_diskDriveRenameOrMove,
	syscall_diskDriveGetFileInfo,
	syscall_diskDriveGetFreeClustersNum,

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

