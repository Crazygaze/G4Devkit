/*******************************************************************************
 *  Code for managing programs
 ******************************************************************************/

#ifndef _APCPUOS_PROGRAM_H_
#define _APCPUOS_PROGRAM_H_

#include <stddef_shared.h>
#include "kernel/kerneldefs.h"
#include "extern/tlsf/tlsf.h"
#include "utilshared/linkedlist.h"
#include "appsdk/kernel_shared/syscalls_shared.h"
#include "appsdk/kernel_shared/process_shared.h"
#include "utilshared/queue32.h"
#include "utilshared/queue.h"

#define PRC_MIN_STACKSIZE 64

#define PID_NONE 0
#define PID_KERNEL 1
#define PID_INVALID 255

typedef struct CpuCtx
{
	uint32_t gregs[CPU_NUM_GREGS]; // general purpose registers
	uint32_t flags; // flags register
	double fregs[CPU_NUM_FREGS]; // floating point registers
} CpuCtx;

//
// PCB stands for Process Control Block
// See http://en.wikipedia.org/wiki/Process_control_block
//

typedef struct PCBStatsHelper
{
	// Used to help with the stats calculation
	double cpuTimeMarker;
	double cpuTimeswiMarker;

	// How much time in seconds spent on this task
	double cpuTime;
	// How much time in seconds spent serving swi calls
	double cpuTimeswi;
	
	// When we last updated the stats
	double lastUpdateTime;
} PCBStatsHelper;

typedef struct PCB
{
	struct PCB* next;
	struct PCB* previous;

	struct TCB* mainthread;

	// Putting this in its own struct, so I can use memcpy to copy it over to
	// any calling applications
	ProcessInfo info;
	PCBStatsHelper stats;

	// Data segment
	void* ds;
	
	int firstPage;
	int numPages;
	
	// Screen canvas if this process has one
	void* canvas;

	uint8_t padding2[2];
} PCB;


// Thread is ready to run
#define TCB_STATE_READY 0
// Thread is currently blocked.
// The exact reason for the block can be determined by inspecting the TCBWait
#define TCB_STATE_BLOCKED 1
// Thread finished execution
#define TCB_STATE_DONE 2
// Kernel thread
#define TCB_STATE_KERNEL 3

//
// Reasons a thread can be blocked
//
#define TCB_WAIT_TYPE_SLEEP 0
#define TCB_WAIT_TYPE_WAITING_FOR_MSG 1

typedef union TCBWaitData {
	double sleepTimeMarker;
	ThreadMsg* msgDst;
} TCBWaitData;

typedef struct TCBWait
{
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
	
	struct CpuCtx* cpuctx;
	struct PCB* pcb; // Process this thread belongs to
	
	// Handle to be used by the application to refer to this thread
	HANDLE handle;
	
	// Thread local storage. This is set the application sdk with a system call
	// The application thread startup code calls a system call to specify the
	// address of the variable to set, and the the value to set it to.
	// Then, every time a thread is scheduled in, the kernel will set that
	// variable to the specified value, thus allowing the thread to access a
	// context with anything it needs, as for example, a pointer to a structure
	// with all the information it needs.
	uint32_t* tlsVarPtr;
	uint32_t tlsVarValue;

	// Stack is a full descending stack, like in the arm calling conventions
	uint8_t* stackTop; // SP is valid for any SP<=stacktop
	uint8_t* stackBottom; // SP is valid for any SP>=stackbottom
	
	// A value from the TCB_STATE_XXX defines
	uint8_t state;
	uint8_t padding1[3];
	
	// If thread is blocked waiting for something, this has all the information
	// about it
	TCBWait wait;
	
	// Message queue for the thread
	Queue msgqueue;
	
	// Queue the thread is currently in.
	// When a thread is destroyed, it needs to be removed from this queue.
	// A given thread can only be in one queue at a time.
	Queue32* queue;
} TCB;

/*! Moves the thread between queues
* A thread can only be in one queue at a time.
* This functions does all the bookeeping for moving threads between queues.
* \param tcb
*	Thread to move
* \param to
*	Queue to move the thread to, or NULL if the thread is to be removed from the
*	queue it's currently in.
*/
void tcb_enqueue(TCB* tcb, Queue32* to);

PCB* prc_initKernelPrc(void);
PCB* prc_create(const char* name, PrcEntryFunc entryfunc, bool privileged,
				   size_t stackSize, size_t heapSize);
void prc_delete(PCB* prc);

/*! Creates a thread on the current process.
* \note
*	This is used to create extra threads, since a process's main thread is
*	created differently as implicitly as part of the process creation
*/
TCB* prc_createThread(PCB* pcb, ThreadEntryFuncWrapper entryFuncWrapper,
	ThreadEntryFunc entryFunc, u32 stackSize, void* userdata, bool privileged);

/*! Posts a message to the specified thread's message queue
* If the queue doesn't exist, it will be created.
* \return
*	True if successfull, false if the message could not be posted (e.g: Out of
*	memory expanding the message queue size)
*/	
bool prc_postThreadMessage(TCB* tcb, u32 msgId, u32 param1, u32 param2);

/*! Puts the specified thread to sleep for the specified interval
*/
void prc_putThreadToSleep(TCB* tcb, u32 ms);

/*! Sets an application timer for the specified thread
* This is responsible for setting up the MSG_TIMER messages postings.
*/
bool prc_setThreadTimer(TCB* tcb, uint32_t timerId, uint32_t ms, bool repeat);

/*! Find a process by its name
*/
PCB* prc_find(const char* name);

/*! Find a process by its PID
*/
PCB* prc_findByPID(uint8_t pid);

/*! Updates the cpu stats fields for the specified process */
void prc_calcCpuStats(PCB* pcb);

/*! Sets focus to the specified process, doing all the required
work such as posting the required messages */
bool prc_setFocus(PCB* pcb);

/*! Sets the focus to any suitable process */
PCB* prc_setDefaultFocus(void);

/*!
* Gives/Removes kernel access to the specified process's memory.
* This is used when the kernel needs temporary access to a process's memory,
* as for example during a system call
*
* \param PCB
*	process the kernel needs access to
* \param status
*	If TRUE, the kernel will have access to the specified process's pages.
*	If FALSE, access will transfer to the processa again
*
* \note
*	A matching call with the "status" set as false must happen after the kernel
*	finishes using the pages.
*/
void prc_giveAccessToKernel(PCB* pcb, bool status);


/*!
* Gives the address of a variable that belongs to shared data, in the context
* of the given process
*
* For example, suppose you have a global variable named "someFoo" that is part
* of shared data. To get the address of that variable in the address space of
* the process somePcb, you do
*
* void* addr = prc_getPtrToShared(somePcb, &someFoo);
*/
void* prc_getPtrToShared(PCB* pcb, void* var);

#ifdef DEBUG
void prc_logAllThreads(const char* title);
#endif

#endif
