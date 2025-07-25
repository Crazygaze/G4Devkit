#include "syscalls.h"
#include "../kernel/kernel.h"
#include "../kernel/mmu.h"
#include "../kernel/handles.h"
#include "../fs/extern/fatfs/source/ff.h"
#include <stdlib.h>

/*!
 * Checks if a userspace addrs and size are valid. If invalid, it causes the
 * caller to return false.
 *
 * NOTE: Since this does a `return false` for the caller, it should NOT be
 * called while in the scope of a `ADD_USER_KEY` call.
 */
#define CHECK_USER_PTR(needsWrite, addr, size) \
	if (!mmu_checkUserPtr(krn.currTcb->pcb, needsWrite, addr, size)) return false;

bool syscall_setupDS(void)
{
	const PTKrnLayout* layout = mmu_getKrnPTLayout();
	u32 size = layout->oriSharedDataEnd - layout->oriSharedDataBegin;
	
	ADD_USER_KEY;
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
		
	ADD_USER_KEY;
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
		
	
	ADD_USER_KEY;
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

	ADD_USER_KEY;
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

bool syscall_getMsg(void)
{
	TCB* tcb = krn.currTcb;
	int* regs = tcb->ctx.gregs;
	
	ThreadMsg* outMsg = (ThreadMsg*)regs[0];
	bool waitForMsg = (bool)regs[1];
	CHECK_USER_PTR(true, outMsg, sizeof(*outMsg));
	
	ADD_USER_KEY;
	bool hasMsg = queue_ThreadMsg_pop(&tcb->msgqueue, outMsg);
	REMOVE_USER_KEY;
	
	// If there is a message, then we pass control back to the application.
	// If there isn't, then we put the thread to sleep if required
	if (hasMsg) {
		regs[0] = true;
	} else {
	
		// We need to set the result to false, because when a message awakes
		// the thread, the kernel will schedule the thread for execution, and
		// the process will see `false` and call getMsg again to get the actual
		// message.
		
		regs[0] = false;
		if (waitForMsg) {
			// Put the thread to sleep until a message is available
			tcb->state = TCB_STATE_BLOCKED;
			tcb->wait.type = TCB_WAIT_TYPE_WAITING_FOR_MSG;
			tcb_enqueue(krn.currTcb, NULL);
			krn_pickNextTcb();
		}
	}
	
	return true;
}

bool syscall_postMsg(void)
{
	TCB* tcb = krn.currTcb;
	int* regs = tcb->ctx.gregs;
	
	HANDLE target = (HANDLE)regs[0];
	u32 msgId  = regs[1];
	u32 param1 = regs[2];
	u32 param2 = regs[3];
	
	// The target thread needs to belong to the same process, otherwise any
	// process could mess up with other processes by sending random messages
	TCB* targetTcb =
		handles_getData(target, krn.currTcb->pcb, kHandleType_Thread);
	if (!targetTcb) {
		regs[0] = false;
		return true;
	}
	
	prc_postThreadMsg(targetTcb, msgId, param1, param2);
	regs[0] = true;
	
	return true;
}

bool syscall_setTimer(void)
{
	TCB* tcb = krn.currTcb;
	int* regs = tcb->ctx.gregs;
	u32 ms = regs[0];
	bool repeat = regs[1];
	void* cookie = (void*)regs[2];
	regs[0] = prc_addThreadTimer(tcb, ms, repeat, cookie);
	return true;
}

bool syscall_outputDebugString(void)
{
	// #TODO : Test if the pointer validation is working
	const char* usrStr= (const char*)krn.currTcb->ctx.gregs[0];
	
	// #TODO : Make use of size, or think hard if we can safely ignore it.
	u32 size = krn.currTcb->ctx.gregs[1];
	size = min(size, _STDC_LOG_MAXSTRINGSIZE - 1);
	CHECK_USER_PTR(false, usrStr, size); 
	
	ADD_USER_KEY;
	// Copy the string to kernel pages, since hardware functions expect physical
	// addresses and kernel pages are not setup with translation.
	char str[_STDC_LOG_MAXSTRINGSIZE];
	memcpy(str, usrStr, size);
	str[size] = 0;
	REMOVE_USER_KEY;
	
	hwnic_sendDebug((phys_addr)str);
	
	return true;
}

// #TODO : Implement the actual file closing when it destroys the handle

bool syscall_openFile(void)
{
	int* regs = krn.currTcb->ctx.gregs;
	const FileOpenParams_* inParams = (const FileOpenParams_*)regs[0];
	CHECK_USER_PTR(false, inParams, sizeof(FileOpenParams_)); 
	
	ADD_USER_KEY;
	// Copy the parameters over to a local variable, so we can have a very
	// small scope for the adding/removing USER_KEY
	// Also, make sure the strings are null terminated, to catch string
	// overflows
	FileOpenParams_ params = *inParams;
	REMOVE_USER_KEY;
	
	params.filename[MAX_PATH - 1] = 0;
	params.mode[MAX_FILEMODE - 1] = 0;
	
	// Conversion according to https://elm-chan.org/fsw/ff/doc/open.html
	BYTE mode = 0;
	if (strcmp(params.mode, "r") == 0)
		mode = FA_READ;
	else if (strcmp(params.mode, "r+") == 0)
		mode = FA_READ | FA_WRITE;
	else if (strcmp(params.mode, "w") == 0)
		mode = FA_CREATE_ALWAYS | FA_WRITE;
	else if (strcmp(params.mode, "w+") == 0)
		mode = FA_CREATE_ALWAYS | FA_WRITE | FA_READ;
	else if (strcmp(params.mode, "a") == 0)
		mode = FA_OPEN_APPEND | FA_WRITE;
	else if (strcmp(params.mode, "a+") == 0)
		mode = FA_OPEN_APPEND | FA_WRITE | FA_READ;
	else if (strcmp(params.mode, "wx") == 0)
		mode = FA_CREATE_NEW | FA_WRITE;
	else if (strcmp(params.mode, "w+x") == 0)
		mode = FA_CREATE_NEW | FA_WRITE | FA_READ;
	else 
		goto out1;
	
	FIL* fp = calloc(sizeof(FIL));
	if (!fp)
		goto out1;
		
	HANDLE h = handles_create(krn.currTcb->pcb, kHandleType_File, fp);
	if (!h)
		goto out2;
		
	FRESULT fr = f_open(fp, params.filename, mode);
	if (fr) {
		OS_ERR("f_open failed with error %d", fr);
		OS_ERR("Filename='%s', mode='%s'.", params.filename, params.mode);
		goto out3;
	}
	
	regs[0] = (u32)h;
	return true;
	
	out3:
		handles_destroy(h, krn.currTcb->pcb);
		// the handle destruction will automatically free fp, so lets set it to
		// NULL so the next `free` doesn't do anything
		fp = NULL;
	out2:
		free(fp);
	out1:
		regs[0] = 0;
		return false;
}

bool syscall_fileWrite(void)
{
	int* regs = krn.currTcb->ctx.gregs;
	
	const u8* buffer = (const u8*)regs[0];
	size_t bytes = regs[1];
	HANDLE h = (HANDLE)regs[2];
	
	CHECK_USER_PTR(false, buffer, bytes);
	
	// Get the file struct
	FIL* fp = (FIL*)handles_getData(h, krn.currTcb->pcb, kHandleType_File);
	if (!fp) {
		regs[0] = 0;
		return true;
	}
	
	//
	// We can't pass the userspace buffer directly to FatFs, because it will
	// then pass that directly to hwf, which requires physical addresses.
	// So, we fix this by writing it in chunks, from our own buffer.
	// 
	s8 localBuf[FF_MAX_SS];
	size_t bytesDone = 0;
	while (bytes) {
		size_t len = min(bytes, sizeof(localBuf));
		
		ADD_USER_KEY;
		memcpy(localBuf, buffer, len);
		REMOVE_USER_KEY;
		
		buffer += len;
		bytes -= len;
		
		UINT written = 0;
		FRESULT fr = f_write(fp, localBuf, len, &written);
		bytesDone += written;
		// If there is an error, or we didn't write as many bytes as we expected
		// then we exit earlier
		if (fr != FR_OK || written != len)
			break;
	}

	regs[0] = bytesDone;
	return true;
}

bool syscall_fileRead(void)
{
	int* regs = krn.currTcb->ctx.gregs;
	
	u8* buffer = (u8*)regs[0];
	size_t bytes = regs[1];
	HANDLE h = (HANDLE)regs[2];
	
	CHECK_USER_PTR(true, buffer, bytes);
	
	// Get the file struct
	FIL* fp = (FIL*)handles_getData(h, krn.currTcb->pcb, kHandleType_File);
	if (!fp) {
		regs[0] = 0;
		return true;
	}
	
	//
	// We can't pass the userspace buffer directly to FatFs, because it will
	// then pass that directly to hwf, which requires physical addresses.
	// So, we fix this by reading it in chunks into our local buffer
	// 
	s8 localBuf[FF_MAX_SS];
	size_t bytesDone = 0;
	while (bytes) {
		size_t len = min(bytes, sizeof(localBuf));
		
		UINT read = 0;
		FRESULT fr = f_read(fp, localBuf, len, &read);
		bytesDone += read;
		
		// If there is an error, or we didn't write as many bytes as we expected
		// then we exit earlier
		if (fr != FR_OK || read != len)
			break;
		
		ADD_USER_KEY;
		memcpy(buffer, localBuf, read);
		REMOVE_USER_KEY;
		
		buffer += len;
		bytes -= len;
	}

	regs[0] = bytesDone;
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
	syscall_getMsg,
	syscall_postMsg,
	syscall_setTimer,
	
	//
	// System information
	//
	
	//
	// Hardware
	//
	
	// 
	// Disk Drive
	//
	syscall_openFile,
	syscall_fileWrite,
	syscall_fileRead,

	//
	// Rendering
	//
	
	//
	// Debug
	//
	syscall_outputDebugString
};
