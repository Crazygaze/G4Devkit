#include "app_process.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "kernel_shared/txtui_shared.h"

AppInfo* app_info;

/*
The way TLS is supported is:
- The thread wrapper function adds a struct of this type to the stack,
  and tells the kernel that this struct is TLS data.
- Whenever the kernel schedules the thread to run, it updates the pointer we are
  using for TLS (app_tls) to the address of the struct that's located in
  the stack.
- Any application code can then use the "app_tls" pointer to access the TLS
*/
AppTLS* app_tls;

int app_outputDebugString(const char* fmt, ...)
{
	char buffer[APPSDK_DEBUGSTRING_SIZE];
	va_list ap;
	va_start(ap, fmt);
	int len = vsprintf(buffer, fmt, ap);
	app_syscall2(kSysCall_OutputDebugString, (uint32_t)(&buffer[0]), len);
	return 0;
}

static void app_setupTLS(AppTLS* tls)
{
	app_tls = tls;
	app_tls->threadHandle = (HANDLE)app_syscall0(kSysCall_GetThreadHandle);
	// Tell the kernel where we have our TLS
	app_syscall2(kSysCall_SetThreadTLS,(uint32_t)(&app_tls),(uint32_t)app_tls);
}

/*
 * Entry point for new processes.
 * This is run in the context of the process already, so we can setup some stuff
 */
void app_startup(PrcEntryFunc func)
{

	//
	// Initialize AppInfo struct
	AppInfo localAppInfo;
	memset(&localAppInfo, 0, sizeof(localAppInfo));
	app_getProcessInfo(&localAppInfo.prcInfo, false);
	app_info = &localAppInfo;
	
	//
	// Setup TLS for this thread
	AppTLS tlsData;
	app_setupTLS(&tlsData);

	stdcshared_init(app_outputDebugString, app_info->prcInfo.heap_start,
	app_info->prcInfo.heap_size);
	
	LOG("Prc %s, flags=%u, cookie=%u", &app_info->prcInfo.name,
		app_info->prcInfo.flags, app_info->prcInfo.cookie);
	
	if (app_info->prcInfo.flags & APPFLAG_WANTSCANVAS) {
		txtui_init(NULL);
	}

	func(app_info->prcInfo.cookie);
	
	// Tell the kernel we are done with this process.
	// This causes the current thread to be destroyed, and therefore this call
	// never returns.
	// Also, since this is the main thread, it will cause the entire app to 
	// shutdown
	app_closeHandle( app_getThreadHandle() );
}


/*! Wraps application threads, so it can do any initialization and shutdown
 */
void app_threadWrapper(ThreadEntryFunc entryFunc, void* userdata)
{
	//
	// Setup TLS for this thread
	AppTLS tlsData;
	app_setupTLS(&tlsData);

	// Call the real thread function
	entryFunc(userdata);
	
	// Tell the kernel we are done with this thread.
	// This causes the current thread to be destroyed, and therefore this call
	// never returns
	app_closeHandle( app_getThreadHandle() );
}

HANDLE app_getThreadHandle(void)
{
	return app_tls->threadHandle;
}

bool app_getProcessInfo(ProcessInfo* info, bool updateStats)
{
	return app_syscall2(kSysCall_GetProcessInfo, (u32)info, (u32)updateStats);
}

HANDLE app_createThread(ThreadEntryFunc entryFunc, int stackSize,
	void* userdata)
{
	HANDLE res = (HANDLE) app_syscall4(
		kSysCall_CreateThread, (int)app_threadWrapper, (int)entryFunc,
		(int)stackSize, (int)userdata);
		
	return res;
}

bool app_closeHandle(HANDLE h)
{
	return app_syscall1(kSysCall_CloseHandle, (int)h);
}

bool app_getMessage(ThreadMsg* msg)
{
	app_syscall2(kSysCall_GetMessage, (int)msg, TRUE);
	return (msg->id==MSG_QUIT) ? false : true;
}

bool app_tryGetMessage(ThreadMsg* msg)
{
	auto res = app_syscall2(kSysCall_GetMessage, (int)msg, FALSE);
	return res;
}

void app_postMessage(HANDLE thread, uint32_t msgId, uint32_t param1,
	uint32_t param2)
{
	app_syscall4(kSysCall_PostMessage, (int)thread, msgId, param1, param2);
}

bool app_setTimer(int timerId, uint32_t ms, bool repeat)
{
	return app_syscall3(kSysCall_SetTimer, timerId, ms, (int)repeat);
}

bool app_focus(void)
{
	// 0 means this process
	return app_syscall1(kSysCall_SetFocusTo, 0);
}

bool app_setFocusTo(uint8_t pid)
{
	return app_syscall1(kSysCall_SetFocusTo, pid);
}

bool app_setStatusBar(const char* str, size_t size)
{
	return app_syscall2(kSysCall_SetStatusBar,(uint32_t)str, size);
}

int app_processScreenshot(uint8_t pid, void* dest, int size)
{
	return app_syscall3(kSysCall_ProcessScreenshot, pid, (uint32_t)dest, size);
}
