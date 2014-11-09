/*******************************************************************************
* Common code with all definitions the App SDK needs in order to communicate
* with the Kernel.
* This is also included from the Kernel, so we are sure the SDK matches the OS
*******************************************************************************/

#ifndef _APCPU_SYSCALLS_SHARED_H_
#define _APCPU_SYSCALLS_SHARED_H_

#include "stddef_shared.h"

typedef void* HANDLE;
#define INVALID_HANDLE 0L

// What register we get the syscall id in.
#define SYSCALL_ID_REGISTER 10

//! Entry point for a new application
typedef int (*PrcEntryFunc)(int p1);
void app_startup(PrcEntryFunc func);

//! Application thread entry function
typedef void (*ThreadEntryFunc)(void* userdata);

//! Don't use this in the application. It's for internal use
typedef void (*ThreadEntryFuncWrapper)(ThreadEntryFunc, void* userdata);

/*
Available System calls
*/
typedef enum SysCallID {
	//
	// Process control
	//
	kSysCall_Yield,
	kSysCall_Sleep,
	kSysCall_GetStackSize,
	kSysCall_GetUsedStackSize,
	kSysCall_GetThreadHandle,
	kSysCall_CreateThread,
	kSysCall_SetThreadTLS,
	kSysCall_CloseHandle,
	kSysCall_GetMessage,
	kSysCall_PostMessage,
	kSysCall_SetTimer,
	kSysCall_SetFocusTo, // Gives focus to specified process
	
	//
	// System information
	//
	kSysCall_GetProcessCount, // Number of processes running
	kSysCall_GetProcessInfo, // Information about the current process
	kSysCall_GetOSInfo, // Information about all processes
	
	//
	// Hardware
	//
	kSysCall_GetScreenBuffer, // TODO : Remove this. Applications will have their own screen buffer
	kSysCall_GetScreenXRes,
	kSysCall_GetScreenYRes,
	
	//
	// Rendering
	//
	kSysCall_SetCanvas,
	kSysCall_SetStatusBar,
	kSysCall_ProcessScreenshot,
	
	//
	// Debug
	//
	kSysCall_OutputDebugString,
	
	kSysCall_Max,
} SysCallID;


//
// What handle types we have available
// WARNING: If this changes, update the functions array in handles.c
typedef enum HandleType
{
	kHandleType_NONE,
	kHandleType_Mutex,
	kHandleType_Thread,
	kHandleType_MAX
} HandleType;

#endif
