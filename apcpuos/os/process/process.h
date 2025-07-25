#ifndef _os_process_h
#define _os_process_h

#include "../os_config.h"
#include "appsdk/os_shared/process_shared.h"
#include "utils/queue.h"
#include "utils/queue32.h"
#include "../kernel/mmu.h"

#define PRC_MIN_STACKSIZE 64

#define PID_IDLE 1


//
// PCB stands for Process Control Block
// See http://en.wikipedia.org/wiki/Process_control_block
//
typedef struct PCB
{
	struct PCB* next;
	struct PCB* previous;

	struct TCB* mainthread;
	
	PageTable* pt;

	int numActiveTimers;
	
	// Putting this in its own struct, so I can use memcpy to copy it over to
	// any calling applications
	ProcessInfo info;
	
} PCB;

/*!
 * Thread is ready to run
 */
#define TCB_STATE_READY 0

// #TODO : Document this better
/*!
 * Thread is currently blocked.
 * The exact reason for the block can be determined by inspecting the TCBWait
 */
#define TCB_STATE_BLOCKED 1

/*!
 * Thread finished execution
 */
#define TCB_STATE_DONE 2

/*!
 * Threads marked with this don't participate in the scheduler and are
 * switched to explicitily
 */
#define TCB_STATE_KERNEL 3

QUEUE_TYPEDECLARE(ThreadMsg)

/*!
 * The thread is NOT waiting for anything
 */
#define TCB_WAIT_TYPE_NONE 0

/*!
 * The thread is currently sleeping
 */
#define TCB_WAIT_TYPE_SLEEP 1

/*!
 * The thread is blocked waiting for a mutex to be unlocked
 */
#define TCB_WAIT_TYPE_WAIT 2

// #MSG: Make use of this
/*!
 * The thread is blocked waiting for a message to arrive
 */
#define TCB_WAIT_TYPE_WAITING_FOR_MSG 3

/*!
 * Wait data
 */
typedef union TCBWaitData {

	// Used if type is TCB_WAIT_TYPE_SLEEP
	// Time at which the thread should stop sleep.
	double sleepEnd;
	
	// Used if type is TCB_WAIT_TYPE_WAIT
	HANDLE mtx;
} TCBWaitData;

/*!
 * Data for when a thread is blocked for whatever reason
 */
typedef struct TCBWait {
	// A value of one of the TCB_WAIT_TYPE_X macros
	uint8_t type;
	TCBWaitData d;
} TCBWait;

//
// TCB stands for Thread control block
// See http://en.wikipedia.org/wiki/Thread_control_block
typedef struct TCB
{
	struct TCB* next;
	struct TCB* previous;
	
	FullCpuCtx ctx;
	struct PCB* pcb; // Process this thread belongs to
	
	// Where the stack starts and ends.
	// Intentionally an u32 so the kernel doesn't try to touch this memory by
	// mistake
	u32 stackEnd;
	u32 stackBegin;
	
	// Pointer to the tls array.
	// This points to where in the thread's stack the tls array is.
	// When the kernel switches threads, it sets a global pointer to this
	u32* tlsSlotsPtr;
	
	// #TODO : Initialize this for the main thread too
	// #TODO : This is not beging initialized at all
	// Handle to be used by the application to refer to this thread
	HANDLE handle;

	// A value from the TCB_STATE_XXX defines
	uint8_t state;
	// Wait information.
	TCBWait wait;
	
	// Message queue for the thread
	Queue_ThreadMsg msgqueue;
	
	// Queue the thread is currently in.
	// When a thread is destroyed, it needs to be removed from this queue.
	// A given thread can only be in one queue at a time.
	Queue32* queue;
} TCB;

PCB* prc_initRoobPCB(void);

/*!
 * Sets the mmu keys of the specified process
 */
void prc_setMMUKeys(PCB* pcb, u32 keys);

/*!
 * Creates a process.
 *
 * \param name Name of the the process
 *
 * \param entryFunc Process's entry function
 *
 * \param kernelMode If set to true, it will create a kernel process.
 * Kernel processes share one single page table that only has the kernel space
 * range.
 * They are also not included in the scheduler. Switching to those processes is
 * only done explicitily.
 *
 * \param stackSize Stack size required by the process.
 * - If `kernelMode` is true the stack will simply simply be allocated from
 * kernel heap, and thus there is no way to detect a stack overflow.
 * - If `kernelMode` is false, the stack will have its own pages, and only 1
 * page is allocated from the start. It then grows dynamically as required, up
 * this specified value.
 *
 * \param heapNPages Number of heap pages that should be set in the page table.
 *	This is only used if `kernelMode` is false.
 *	A value of 0 means the process doesn't need heap, and thus no space for heap
 *	will exist in the page table.
 *	If >0, then the page table will be set to support that heap size, BUT only 1
 *	page will be preallocated to begin with. Additional pages can then be
 *	requested by the application
 */
PCB* prc_createPCB(const char* name, PrcEntryFunc entryFunc, bool kernelMode,
	u32 stackSize, u32 heapNPages);
	
/*!
 * Creates a new thread.
 *
 * \param pcb Owning process
 * \param stackBegin Where the stack starts.
 * \param stackEnd value to set the `sp` register to.
 * \param func Entry function for the thread.
 * \param crflags Value to set the crflags register to.
 * \param crirqmsk Value to set the crirqmsk register to.
 *
 * \return the TCB or NULL on error.
 */
TCB* prc_createTCB(PCB* pcb, ThreadEntryFunc func, u32 stackBegin, u32 stackEnd,
	u32 crflags, u32 crirqmsk, u32 cookie);

/*!
 * Destroys the specified process
 */
void prc_destroyPCB(PCB* pcb);

/*!
 * Destroys the specified Thread
 */
void prc_destroyTCB(TCB* pcb);

/*!
 * Puts the specified thread to sleep for the specified interval.
 */
void prc_putThreadToSleep(TCB* tcb, u32 ms);

/*!
 * Sets a thread as waiting on the specified mutex.
 * The thread will not execute until the mutex is unlocked
 */
void prc_putThreadToWait(TCB* tcb, HANDLE mtx);

/*!
 * Wakes up to ONE thread that is waiting on the specified mutex
 */
void prc_wakeOneWaitingThread(HANDLE mtx);

/*!
 * Adds a timer to the specified thread, so that it pushes a MSG_TIMER message
 * to the thread message queue when the timer expires
 */
bool prc_addThreadTimer(TCB* tcb, u32 ms, bool repeat, void* cookie);

//#TODO This can in theory fail, since it pushes stuff to a queue, which might
// need to expand. Therefore, it should return false.
/*!
 * Moves the thread between queuese.
 * A thread can only be in one queue at a time;
 *
 * \param tcb Thread to mov
 * \param to Queue to move to. If nulled, the thread will be removed from the
 * current queue and not put into any queue.
 */
void tcb_enqueue(TCB* tcb, Queue32* to);


/*!
 * Changes the program break for the specified process
 */
bool prc_setBrk(PCB* pcb, u32 newbrk);

/*!
 * Posts a message to the specified thread
 */
bool prc_postThreadMsg(TCB* tcb, u32 msgId, u32 param1, u32 param2);


#endif
