#include "app_process.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdc_init.h>
#include "utils/bitops.h"
#include "hwcrt0.h"

void app_setupDS(void);

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
	
	// #TODO : Implement TLS
	
	// #TODO : Decide what parameters we need to pass to the application.
	func(cookie);
}

// Entry point for extra threads the process creates
void app_threadEntry(ThreadEntryFunc func, void* cookie)
{
	// #TODO : Implement TLS

	func(cookie);
}

//
//
//
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

void app_setupDS(void)
{
	app_syscall0(kSysCall_SetupDS);
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

HANDLE app_createThread(const CreateThreadParams* params)
{
	if (params->entryFunc == NULL || params->stackSize==0) {
		LOG_ERR("Invalid parameters");
		return INVALID_HANDLE;
	}
	
	u32 stackSize = ALIGN(params->stackSize, 4);
	u8* stackBottom = malloc(stackSize);
	if (!stackBottom) {
		LOG_ERR("Out of memory");
		return INVALID_HANDLE;
	}
	u8* stackTop = stackBottom + stackSize;

	HANDLE res = (HANDLE) app_syscall2(
		kSysCall_CreateThread, (u32)params, (u32)stackTop);
		
	return res;
}

bool app_setBrk(void* brk)
{
	return app_syscall1(kSysCall_SetBrk, (u32)brk);
}
