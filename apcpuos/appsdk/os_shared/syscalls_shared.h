#ifndef _appsdk_syscalls_shared_h
#define _appsdk_syscalls_shared_h

#include <stdint.h>

typedef enum SysCallID {

	//
	// Process management
	//
	kSysCall_SetupDS,
	kSysCall_SetTlsPtr,
	kSysCall_Sleep,
	kSysCall_CreateThread,
	kSysCall_SetBrk,
	kSysCall_GetCurrentThread,
	kSysCall_GetThreadInfo,
	kSysCall_CloseHandle,
	kSysCall_CreateMutex,
	kSysCall_WaitForMutex,
	kSysCall_MutexUnlocked,
	kSysCall_GetMsg,
	kSysCall_PostMsg,
	kSysCall_SetTimer,
	
	//
	// System information
	//
	
	//
	// Hardware
	//
	
	//
	// Disk drive
	//
	kSysCall_OpenFile,
	kSysCall_FileWrite,
	kSysCall_FileRead,
	
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
	 // Starting at one, since 0 has a special meaning
	kHandleType_Mutex = 1,
	kHandleType_Thread,
	kHandleType_File,
	kHandleType_MAX
} HandleType;

#endif
