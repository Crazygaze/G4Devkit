#ifndef _app_process_h_
#define _app_process_h_

#include "appsdkconfig.h"
#include "app_syscalls.h"
#include "kernel_shared/process_shared.h"
#include "kernel_shared/syncprimitives_shared.h"

typedef struct AppTLS
{
	HANDLE threadHandle;
} AppTLS;

typedef struct AppInfo
{
	ProcessInfo prcInfo;
} AppInfo;

extern AppInfo* app_info;
extern AppTLS* app_tls;

/*! Yields the remaining cpu time slice to the kernel
 * 
 */
#define app_yield() app_syscall0(kSysCall_Yield)

/*! Sleeps the application for the specified number of milliseconds
 * Sleep duration isn't necessarly accurate, as it depends on the time slice
 * duration used by the operating system
 */
#define app_sleep(ms) app_syscall1(kSysCall_Sleep,ms)

/*! Gets the stack size of the current thread, in bytes
 */
#define app_getStackSize() app_syscall0(kSysCall_GetStackSize)

/*! Gets the used stack size of the current thread, in bytes
 */
#define app_getUsedStackSize() app_syscall0(kSysCall_GetUsedStackSize)


/*! Returns the handle of the current thread
*/
HANDLE app_getThreadHandle(void);

/*! Returns information about the process
\param info Where to write the information to
\param updateStats If TRUE, it will recalculate the process stats
*/
bool app_getProcessInfo(ProcessInfo* info, bool updateStats);

/*! Creates a new thread
* \param entryFunc
*	Thread entry function
* \param stackSize
*	Thread's stack size, in bytes. It will be rounded up to the MMU page
*	size.
* \param userdata
*	Data to pass as a parameter to the thread entry function.
*	This provides context to the thread, if required
* \return HANDLE to the thread
*	The thread can be explicitly destroyed with app_closeHandle
* \note
*	Unlike for the process's main thread, stack overflows for new threads are
*	not guaranteed to be detected.
*	If the memory page right before the page(s) allocated for the stack belongs 
*	to another process, then stack overflow will be detected, as the application
*	will try to write to another process's memory space. On the other hand, if
*	that page belongs to the same process, then writing over that memory will
*	not generate an access violation, and therefore will fail to detect the
*	stack overflow.
*/
HANDLE app_createThread(ThreadEntryFunc entryFunc, int stackSize,
	void* userdata);

/*! Closes a generic OS handle
* \return True if successful, false it the handle is invalid
*/
bool app_closeHandle(HANDLE h);

/*!
 * Note: Return type is unused. It's just to match the printf signature
 */
int app_outputDebugString(const char* fmt, ...);

/*! Retrieves a message from the current Thread's message queue
* \param msg
*	Where to get the message information
* \return
*	TRUE if the thread should continue running.
*	FALSE if the message retrieved was a Quit message, and the thread should
*	shutdown.
*/
bool app_getMessage(ThreadMsg* msg);

/*! Tries to retrieve a message, without blocking.
* \return
*	True if a message was retrieved. False it there was no message
*/
bool app_tryGetMessage(ThreadMsg* msg);

/*! Post a message to the specified thread
*/
void app_postMessage(HANDLE thread, uint32_t msgId, uint32_t param1,
	uint32_t param2);

/*! Sets a timer for the current thread
* A MSG_TIMER message will be posted to the current's thread message queue after
* the specified time elapses
*
* \param timerId
*	Timer ID. This is application dependent. It's just a way for the application
*	to know what what timer caused the MSG_TIMER message.
* \param ms
*	Time interval in milliseconds. Maximum supported value is TIMER_MAX_INTERVAL
*	Also, granularity depends on the OS time slice, and can't be expected to be
*	very accurate.
* \param repeat
*	If true, then the timer will be kept and repeatedly post MSG_TIMER at the
*	specified interval.
*	If false, the timer will cause a single MSG_TIMER message, and be removed.
*/
bool app_setTimer(int timerId, uint32_t ms, bool repeat);


/*! Gives focus to the process
\return TRUE if sucessfull
*/
bool app_focus(void);

/*! Set focus to the specified process
*/
bool app_setFocusTo(uint8_t pid);

/*! Changes the status bar to the specified text
*/
bool app_setStatusBar(const char* str, size_t size);

/*! Takes a screenshot of the specified process's screen
\param pid
	ID of the process to capture
\param dest
	Destination buffer where the screenshot will be saved
\param size
	Size of the destination buffer
\return
	Number of bytes copied
*/
int app_processScreenshot(uint8_t pid, void* dest, int size);

#endif
