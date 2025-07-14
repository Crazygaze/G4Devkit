#ifndef _appsdk_process_shared_h
#define _appsdk_process_shared_h

#include "syscalls_shared.h"
#include "utils/bitset.h"


/*!
 * Operating system handle.
 * An HANDLE can represent a thread, process, mutex or any other system resource.
 * Several functions return handles when creating a system resource.
 * To destroy the resource, the function app_closeHandle can be used.
 */
struct HANDLE_t { u32 unused; };
typedef struct HANDLE_t* HANDLE;

/*!
 * How many tls slots are available
 */
#define TLS_MAXSLOTS 8

/*!
 * Value returns by `app_tlsAlloc` when there aren't any tls slots availble
 */
#define TLS_INVALID -1


/*!
 * An invalid handle.
 * This can be returns by functions that generate an handle, in which case it
 * indicates a failure.
 */
#define INVALID_HANDLE 0L

/*!
 * Entry point for a new application
 */
typedef int (*PrcEntryFunc)(void* cookie);

/*!
 * Thread entry function
 */
typedef void (*ThreadEntryFunc)(void* cookie);

/*!
 * For internal use only. No need to use it directly.
 */
typedef void (*ThreadEntryFuncWrapper)(ThreadEntryFunc func, void* cookie);

/*!
 * Called by the OS as the entry point to a process.
 * You should not call this directly.
 *
 * \param func Actual app entry function
 * \param cookie Passed to the entry function
 * \param heapStart Pointer to the where the heap starts
 * \param initialHeapStart How much heap to start with. If 0, then no heap is
 * givin the process at all
 */
void app_startup(
	PrcEntryFunc func, void* cookie, void* heapStart, u32 initialHeapSize);

/*!
 * Called by the OS as the entry point for new threads.
 * You should not call this directly.
 */
void app_threadEntry(ThreadEntryFunc func, void* cookie);

/*!
 * A process's maximum name size, including the null terminator
 */
#define PRC_NAME_SIZE 9

/*!
 * Miscellaneous process information
 */
typedef struct ProcessInfo {
	// process id
	u32 pid;

	// Process name
	char name[PRC_NAME_SIZE];

	// Percentage of the cpu used by the process
	// This include the time spent in SWI calls
	u8 cpuUsr;

	// Percentage of cpu used by the process in kernel mode
	u8 cpuKrn;

	// A combination of APP_FLAG_x values
	u32 flags;
} ProcessInfo;

// #TODO : Check if this is used. If not, then remove it. 
/*!
 * Information required to create a process.
 */
typedef struct CreateProcessParms{
	char name[PRC_NAME_SIZE];

	// The process's `main` function
	PrcEntryFunc startFunc;

	// How much stack (in bytes), the process is allowed to have.
	// This is just an hint to the OS.
	// NOTE: This affects the process's page table size.
	u32 maxStack;

	// Maximum heap size (in bytes).
	// A value of 0 is valid, and it means the process is not allowed to
	// allocate dynamic memory.
	// NOTE: This affects the process's page table size.
	u32 maxHeap;
} CreateProcessParams;

/*!
 * Information required to create a thread
 */
typedef struct CreateThreadParams {

	/*!
	 * Entry function for the thread
	 */
	ThreadEntryFunc entryFunc;
	
	/*!
	 * Stack size, in bytes.
	 * Make sure this is large enough for the thread's requirements, as there
	 * is no stack overflow protection.
	 * Unlike for the process's main thread, stack overflows for new thread are
	 * not detected.
	 * This is because unlike for the the main thread, the stack for new thread
	 * is simply allocated from process's heap, and thus when the stack grows
	 * too large, it will corrupt other parts of the heap.
	 */ 
	u32 stackSize;
	
	/*!
	 * Data to pass as a parameter to the thread entry function.
	 * This provides context to the thread, if required.
	 */
	void* cookie;
	
} CreateThreadParams;

typedef struct ThreadInfo {
	HANDLE thread;
	void* stackBegin;
	void* stackEnd;
} ThreadInfo;


// For internal use
// Applications don't need to use this directly
typedef struct CreateThreadParams_ {
	ThreadEntryFunc entryFunc;
	void* stackBegin;
	void* stackEnd;
	void* cookie;
} CreateThreadParams_;


/*!
 * Maximum length for any path string, including null terminator
 */
#define MAX_PATH 128

/*!
 * Maximum length for the `mode` string passed to fopen, including the null
 * terminator
 */
#define MAX_FILEMODE 4

#define EOF -1

// For internal use
// Applications don't need to use this directly
typedef struct FileOpenParams_
{
	char filename[MAX_PATH];
	char mode[MAX_FILEMODE];
} FileOpenParams_;


/******************************************************************************/
//
// Keyboard
//
/******************************************************************************/

//
// Keys
//
#define KEY_BACKSPACE 0x01
#define KEY_RETURN 0x02
#define KEY_INSERT 0x03
#define KEY_DELETE 0x04
#define KEY_UP 0x05
#define KEY_DOWN 0x06
#define KEY_LEFT 0x07
#define KEY_RIGHT 0x08
#define KEY_SHIFT 0x09
#define KEY_CONTROL 0x0A
#define KEY_TAB 0x0B
/*
 * These are the printable characters as defined in
 * http://en.wikipedia.org/wiki/ASCII ,
 * in the table "ASCII printable characters"
 */
#define KEY_ASCII_FIRST 0x20  // decimal 32 (space)
#define KEY_ASCII_LAST 0x7E

#define KEY_MOD_CTRL (1<<0)
#define KEY_MOD_SHIFT (1<<1)

/******************************************************************************/
//
// App flags
//
/******************************************************************************/

#define APPFLAG_WANTSKEYS (1<<0)

/******************************************************************************/
//
// Thread message queue
//
/******************************************************************************/

/*!
 * Thread message.
 * The OS uses this to communicate with the process.
 */
typedef struct ThreadMsg
{
	u32 id;
	u32 param1;
	u32 param2;
} ThreadMsg;

//
// Messages
// 
#define MSG_QUIT 0
#define MSG_KEY_PRESSED 1
#define MSG_KEY_RELEASED 2
#define MSG_KEY_TYPED 3
// #MSG : Implement this
#define MSG_TIMER 4
// Any message ids >= to this one are reserved for application use.
#define MSG_FIRST_CUSTOM 50


/******************************************************************************/
//
// Timers
//
/******************************************************************************/

#define TIMER_MAX_INTERVAL_MASK ((1 << 31) -1)
#define TIMER_MAX_INTERVAL TIMER_MAX_INTERVAL_MASK

/*!
 * How many timers a process can have active at one time
 */
#define TIMER_MAX_TIMERS 10

#endif
