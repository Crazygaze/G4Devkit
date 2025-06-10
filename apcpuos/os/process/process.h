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
 * The thread is currently sleeping
 */
#define TCB_WAIT_TYPE_SLEEP 0

/*!
 * Wait data
 */
typedef struct TCBWaitData {

	// Time at which the thread should stop sleep
	double sleepEnd;
	
	//
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
 * \param heapSize Heap size required by the process.
 * - If `kernelMode` is true, this needs to be 0.
 * - If `kernelMode` is false, the process will have a memory range for the
 heap, but no pages are allocated until the heap is actually used.
 */
PCB* prc_createPCB(const char* name, PrcEntryFunc entryFunc, bool kernelMode,
	u32 stackSize, u32 heapSize);
	
/*!
 * Creates a new thread.
 *
 * \param pcb Owning process
 * \param stackTop value to set the `sp` register to.
 * \param func Entry function for the thread.
 * \param crflags Value to set the crflags register to.
 * \param crirqmsk Value to set the crirqmsk register to.
 *
 * \return the TCB or NULL on error.
 */
TCB* prc_createTCB(PCB* pcb, ThreadEntryFunc func, u32 stackTop, u32 crflags,
	u32 crirqmsk, u32 cookie);

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
	

#endif
