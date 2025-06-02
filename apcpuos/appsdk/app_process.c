#include "app_process.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdc_init.h>

void app_startup(PrcEntryFunc func, bool isKernelApp)
{
	if (!isKernelApp) {
		stdc_setLogFunc(app_outputDebugString);
	}
	
	// #TODO : Decide what parameters we need to pass to the application.
	func(NULL);
}

void app_threadEntry(ThreadEntryFunc func, void* userdata)
{
	// #TODO : Implement this
}

//
//
//

static int app_syscall1(
	__reg("r4") int,
	__reg("r0") int)
INLINEASM("\t\
swi r4");

static int app_syscall2(
	__reg("r4") int,
	__reg("r0") int,
	__reg("r1") int)
INLINEASM("\t\
swi r4");

void app_sleep(u32 ms)
{
	app_syscall1(kSysCall_Sleep, ms);
}

int app_outputDebugStringF(const char* fmt, ...)
{
	char buf[APPSDK_DEBUGSTRING_SIZE];
	va_list ap;
	va_start(ap, fmt);
	int len = vsnprintf(buf, APPSDK_DEBUGSTRING_SIZE, fmt, ap);
	app_syscall2(kSysCall_OutputDebugString, (uint32_t)(&buf[0]), len);
	return 0;
}

int app_outputDebugString(const char* str)
{
	int len = strlen(str);
	app_syscall2(kSysCall_OutputDebugString, (uint32_t)str, len);
	return 0;
}
