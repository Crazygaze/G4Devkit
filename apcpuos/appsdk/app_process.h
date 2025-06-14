#ifndef _appsdk_app_process_h
#define _appsdk_app_process_h

#include "app_config.h"
#include "os_shared/process_shared.h"

/*!
 * Causes the current thread to sleep for the specifed milliseconds.
 * Sleep duration isn't necessarily accurate, as it depends on Kernel's quantum,
 * and overlal load on the system.
 * It is however guaranteed to be >= ms
 */
void app_sleep(u32 ms);

/*!
 * Sends a string to the debug output.
 *
 * \note The return type is unused. It is just to patch the printf signature.
 */
int app_outputDebugStringF(const char* fmt, ...);
int app_outputDebugString(const char* fmt);


/*!
 * Creates a new thread.
 *
 * \param entryFunc
 * Thread entry function
 *
 * \param stackSize
 * Thread's stack size, in bytes. Make sure this is big enough for the thread's
 * needs.
 * This will be adjusted to be a multiple of 4.
 *
 * \param cookie
 * Data to pass as a parameter to the thread entry function.
 * This provides context to the thread, if required
 *
 * \return HANDLE to the thread
 *	The thread can be explicitly destroyed with app_closeHandle
 *
 * \note
 * Unlike for the process's main thread, stack overflows for new threads are
 * not guaranteed to be detected.
 * This is because unlike for the the main thread, the stack for new thread is
 * simply allocated from process's heap.
 *
 */
HANDLE app_createThread(const CreateThreadParams* params);


/*!
 * Changes the program break.
 * See https://en.wikipedia.org/wiki/Sbrk
 * Intentionally using a different name and signature.
 *
 * There is no need to use this directly. This is used internally by the memory
 * allocator to request more pages from the OS.
 */
bool app_setBrk(void* brk);

#endif
