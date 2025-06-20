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
 * \param params
 * Parameters for the thread creation
 *
 * \param outStack
 * If the call succeeds, on exit this will contain the pointer to the stack,
 * which was allocated on the heap.
 * Once the thread exits, the process should call `free` to release this.
 * This is necessary at the moment, because the thread's stack is allocated by 
 * the process and not the kernel itself.
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
HANDLE app_createThread(const CreateThreadParams* params, void** outStack);

/*!
 * Calculates how much stack is being used by a process's secondary thread.
 * This should NOT be called for the process's main thread. Doing so is
 * undefined behaviour.
 *
 * The reason why it should not be called for the main thread is because a
 * process's main thread stack does not have a fixed size. It does have a
 * specified maximum size, but the kernel initially allocated only 1 page.
 * Further pages are allocated as the stack grows.
 * Calling this on a main-thread can cause the kernel to attempt to allocate
 * the maximum stack size for the process.
 *
 * Also, this is not an 100% accurate method of calculating the used stack.
 * It works by filling the stack with a magic value and then checking how much
 * of the stack space still has that magic value.
 *
 * \param thread One of the process's secondary threads
 *
 * \return 0 on error, or the used stack size on success.
 * 
 */
u32 app_calcUsedStack(HANDLE thread);


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

/*!
 * Gets the handle to the current thread
 */
HANDLE app_getCurrentThread(void);

/*!
 * Gets the the information of the specified thread
 *
 * \param info
 *	On entry, the `thread` field must be set to the thread handle.
 *	On exit, it will contain the thread information, if the return value was
 *	true.
 */
bool app_getThreadInfo(ThreadInfo* info);

/*!
 * Closes the specified handle, freeing any resources.
 *
 * If the handle is a thread handle and it's the current thread, the thread will
 * be destroyed and the the function doesn't return.
 * In addition, if it's the main thread of the process, the process will be
 * terminated.
 *
 * \param h
 *	Handle to close
 *
 * \return true if the handle was closed, false otherwise.
 *
 */
bool app_closeHandle(HANDLE h);

#endif
