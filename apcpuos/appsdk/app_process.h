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

/*!
 * Allocates a thread local storage (TLS) index.
 * Any thread of the process can subsequently use this index to store and
 * retrieve values that are local to the thread.
 *
 * There are a total of TLS_MAXSLOTS slots available per process. Once the
 * has no need for a slot, it can free it with `app_tlsFree`
 *
 * The TLS api is implemented with minimal error checking. The purpose of the
 * `app_tlsAlloc` and `app_tlsFree` functions is just for the process's several
 * systems to be able to independently use TLS without stomping on on some TLS
 * slot that is already in use by another syste.
 *
 * \return The slot index or TLS_INVALID if no slots are free
 *
 */
int app_tlsAlloc(void);

/*!
 * Frees a TLS slot that was allocated with `app_tlsAlloc`
 *
 * \return True if the the function succeeded, false otherwise.
 */
bool app_tlsFree(int index);

/*!
 * Stores  a value in the specified TLS slot.
 * Each thread of the process has it's own slot for each TLS index.
 */
bool app_tlsSet(int index, u32 value);

/*!
 * Retrieves the value in the calling thread's local storage (TLS) slot for the
 * specified TLS index.
 *
 * Note that the only error checking done is if index is out of bounds, in which
 * case it returns 0.
 *
 */
u32 app_tlsGet(int index);


#endif
