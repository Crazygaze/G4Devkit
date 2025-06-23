#include "app_process.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdc_init.h>
#include "utils/bitops.h"
#include "hwcrt0.h"

// Bitset to tel which slots are in use or not.
// There is only of these per process, since the purpose is to control across
// the process what TLS slots are in use or not.
static u8 appTlsStatus[BS_NUMSLOTS(TLS_MAXSLOTS)];

// TLS slot reserved to save the current thread handle.
// This makes it possible to implement a mutex that doesn't need to make
// a system call when there is no contention.
#define TLS_IDX_CURRENTTHREAD 0

// This is intentionally in the .bss section (not .bss_shared), so that the
// kernel can easily set it when switching threads, since setting data
// for one process while running in the context of another its harder.
extern u32* const appTlsSlots;

void app_setupDS(void);
void app_setTlsPtr(u32* tlsSlots);


static u32 app_syscall0(
	__reg("r4") u32)
INLINEASM("\t\
swi r4");

static u32 app_syscall1(
	__reg("r4") u32,
	__reg("r0") u32)
INLINEASM("\t\
swi r4");

static u32 app_syscall2(
	__reg("r4") u32,
	__reg("r0") u32,
	__reg("r1") u32)
INLINEASM("\t\
swi r4");

static u32 app_syscall3(
	__reg("r4") u32,
	__reg("r0") u32,
	__reg("r1") u32,
	__reg("r2") u32)
INLINEASM("\t\
swi r4");

static u32 app_syscall4(
	__reg("r4") u32,
	__reg("r0") u32,
	__reg("r1") u32,
	__reg("r2") u32,
	__reg("r3") u32)
INLINEASM("\t\
swi r4");


// Entry point for extra threads the process creates
void app_threadEntry(ThreadEntryFunc func, void* cookie)
{
	// The thread's own stack is a nice place to store the TLS slots.
	// All we need is to tell the kernel where that is, and when the kernel
	// switches to a thread, it sets the right pointer
	u32 ownTlsSlots[TLS_MAXSLOTS];
	memset(ownTlsSlots, 0, sizeof(ownTlsSlots));
	app_setTlsPtr(ownTlsSlots);
	
	// Cache the thread HANDLE in a TLS slot
	HANDLE h = (HANDLE)app_syscall0(kSysCall_GetCurrentThread);
	app_tlsSet(TLS_IDX_CURRENTTHREAD, (u32)h);
	
	func(cookie);
}

// Entry point for a new process
void app_startup(
	PrcEntryFunc func, void* cookie, void* heapStart, u32 initialHeapSize)
{
	if (!(hwcpu_get_crflags() & CPU_CRREG_FLAGS_S)) {
		app_setupDS();
		
		// Only set the logging function if running in user space mode.
		// This is because if in kernel mode, we already have the right thing
		// set by the kernel.
		stdc_setLogFunc(app_outputDebugString);
	}

	stdc_init(heapStart, initialHeapSize, &app_setBrk);
	
	verify(app_tlsAlloc() == TLS_IDX_CURRENTTHREAD);
	app_threadEntry((ThreadEntryFunc)func, cookie);
}

//
//
//
/*!
 * \param num Syscall id
 * \param in Input : Pointer to whatever struct we need to pass to the kernel
 * \param out Output (array of 4 words)
 */
u32 app_syscallGeneric(u32 num, void* in, u32* out);

void app_setupDS(void)
{
	app_syscall0(kSysCall_SetupDS);
}

/*!
 * This needs to be called after setting up the .data_shared/.bss_shared
 * sections
 */
void app_setTlsPtr(u32* tlsSlots)
{
	app_syscall1(kSysCall_SetTlsPtr, (u32)tlsSlots);
}

void app_sleep(u32 ms)
{
	app_syscall1(kSysCall_Sleep, ms);
}

int app_outputDebugStringF(const char* fmt, ...)
{
	char buf[_STDC_LOG_MAXSTRINGSIZE];
	va_list ap;
	va_start(ap, fmt);
	int len = vsnprintf(buf, _STDC_LOG_MAXSTRINGSIZE, fmt, ap);
	app_syscall2(kSysCall_OutputDebugString, (u32)(&buf[0]), len);
	return 0;
}

int app_outputDebugString(const char* str)
{
	int len = strlen(str);
	app_syscall2(kSysCall_OutputDebugString, (u32)str, len);
	return 0;
}

HANDLE app_createThread(const CreateThreadParams* params, void** outStack)
{
	if (params->entryFunc == NULL || params->stackSize==0 || outStack==NULL) {
		LOG_ERR("Invalid parameters");
		return INVALID_HANDLE;
	}
	
	CreateThreadParams_ p;
	p.entryFunc = params->entryFunc;
	u32 stackSize = ALIGN(params->stackSize, 4);
	p.stackBegin = malloc(stackSize);
	if (!p.stackBegin) {
		LOG_ERR("Out of memory");
		return INVALID_HANDLE;
	}
	p.stackEnd = (u8*)p.stackBegin + stackSize;
	p.cookie = params->cookie;
	*outStack = p.stackBegin;
	
	// #TODO : Implement a function to calculate's a thread's stack use
	
	// set to a magic number we can look for to calculate how
	// much stack is being used. It's not 100% accurate, but it gives us an
	// idea.
	memset(p.stackBegin, 0xCC, stackSize);

	u32 out[4] = { 0};
	
	HANDLE res = (HANDLE) app_syscallGeneric(kSysCall_CreateThread, &p, out);
		
	return res;
}

u32 app_calcUsedStack(HANDLE thread)
{
	ThreadInfo tinfo;
	tinfo.thread = thread;
	if (!app_getThreadInfo(&tinfo))
		return 0;
	
	u32* ptr = (u32*)tinfo.stackBegin;
	u32* end = (u32*)tinfo.stackEnd;
	while(*ptr == 0xCCCCCCCC && ptr < end)
		ptr++;
	u32 used = (u32)end - (u32)ptr;
	return used;
}

bool app_setBrk(void* brk)
{
	return app_syscall1(kSysCall_SetBrk, (u32)brk);
}


int app_tlsAlloc(void)
{
	for (int i = 0; i < TLS_MAXSLOTS; i++) {
		if (!BS_ISBITSET(appTlsStatus, i))
		{
			BS_SETBIT(appTlsStatus, i);
			return i;
		}
	}
	
	return TLS_INVALID;
}

bool app_tlsFree(int index)
{
	if (index >=0 && index < TLS_MAXSLOTS && BS_ISBITSET(appTlsStatus, index)) {
		BS_CLEARBIT(appTlsStatus, index);
		return true;
	} else {
		return false;
	}
}


// Dummy struct, so we can see the Tls slots content in the debugger
typedef struct TlsSlots {
	u32 a[TLS_MAXSLOTS];
} TlsSlots;

bool app_tlsSet(int index, u32 value)
{
	// Put it in a local variable, so we can look at it in the debugger, since
	// the variable is only defined in one of the asm files and thus it doesn't
	// have debug symbols
	TlsSlots* slots = (TlsSlots*)appTlsSlots;
	
	if (index >=0 && index < TLS_MAXSLOTS && BS_ISBITSET(appTlsStatus, index)) {
		slots->a[index] = value;
		return true;
	} else {
		return false;
	}
}

u32 app_tlsGet(int index)
{
	// Put it in a local variable, so we can look at it in the debugger, since
	// the variable is only defined in one of the asm files and thus it doesn't
	// have debug symbols
	TlsSlots* slots = (TlsSlots*)appTlsSlots;
	
	if (index >=0 && index < TLS_MAXSLOTS && BS_ISBITSET(appTlsStatus, index)) {
		return slots->a[index];
	} else {
		return 0;
	}
}

HANDLE app_getCurrentThread(void)
{
	return (HANDLE)app_tlsGet(TLS_IDX_CURRENTTHREAD);
}

bool app_getThreadInfo(ThreadInfo* inout)
{
	u32 out[4] = { 0};
	return app_syscallGeneric(kSysCall_GetThreadInfo, inout, out);
}

bool app_closeHandle(HANDLE h)
{
	return app_syscall1(kSysCall_CloseHandle, (u32)h);
}

bool app_createMutex(Mutex* mtx)
{
	mtx->counter = 0;
	mtx->owner = 0;
	mtx->h = (HANDLE)app_syscall0(kSysCall_CreateMutex);
	return true;
}

void app_destroyMutex(Mutex* mtx)
{
	assert(mtx->owner == 0);
	mtx->counter = 0;
	mtx->owner = 0;
	app_closeHandle(mtx->h);
}

void app_lockMutex(Mutex* mtx)
{
	const HANDLE cth = app_getCurrentThread();
	
	HANDLE previousOwner = (HANDLE)hwcpu_cmpxchg((u32*)&mtx->owner, 0, (u32)cth);

	if (previousOwner == 0 || // If it was unlocked and we locked it
		previousOwner == cth) { // If we were already the owner
		// Just increment the counter and nothing else to do, since we acquired
		// the mutex
		++mtx->counter;
		return;
	}

	//
	// We failed to acquire the mutex
	//
	for(;;) {
		// Since the architecture is single CPU only, we go to sleep. There is
		// no point in spinning
		app_syscall1(kSysCall_WaitForMutex, (u32)mtx->h);
		
		// We got woken. Lets try again
		// This time the return value needs to be 0, which means that it was
		// unlocked and we just locked it.
		if (hwcpu_cmpxchg((u32*)&mtx->owner, 0, (u32)cth) == 0) {
			++mtx->counter;
			return;
		}
		
		// If we were spuriosly woken or another thread won the race, just
		// loop back and sleep again
	}
}

void app_unlockMutex(Mutex* mtx)
{
	const HANDLE cth = app_getCurrentThread();
	
	// We can only unlock it if we own it
	if (mtx->owner != cth)
		return;
	
	if (--mtx->counter != 0)
		return; // Still held recursively
		
	// Set as unlocked (no owner)
	mtx->owner = 0;
	
	// Wait a thread that might be waiting for the mutex
	app_syscall1(kSysCall_MutexUnlocked, (u32)mtx->h);
}
