#ifndef _appsdk_syscalls_shared_h
#define _appsdk_syscalls_shared_h

#include <stdint.h>
#include "../app_config.h"

typedef enum SysCallID {

	//
	// Process management
	//
	kSysCall_SetupDS,
	kSysCall_SetTlsPtr,
	kSysCall_Sleep,
	kSysCall_CreateThread,
	kSysCall_SetBrk,
	
	//
	// Debug
	//
	kSysCall_OutputDebugString,

	kSysCall_Max,
} SysCallID;

/*!
 * Handle types available.
 * Don't use this directly. This is just an internal detail for the OS
 */
typedef enum HandleType
{
	kHandleType_NONE,
	kHandleType_Mutex,
	kHandleType_Thread,
	kHandleType_MAX
} HandleType;

#if 0

//! Application thread entry function
typedef void (*ThreadEntryFunc)(void* userdata);

//! Don't use this in the application. It's for internal use
typedef void (*ThreadEntryFuncWrapper)(ThreadEntryFunc, void* userdata);

// TODO : Revise this list as I refactor the OS
/*
 * Available System calls
 */
typedef enum SysCallID {

	//
	// Process management
	//
	kSysCall_CreateProcess,

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
	// Disk Drive
	//
	kSysCall_diskDriveRead,
	kSysCall_diskDriveWrite,
	kSysCall_diskDriveSetFlags,
	kSysCall_diskDriveGetFlags,
	kSysCall_diskDriveGetInfo,
	
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


#endif

#endif
