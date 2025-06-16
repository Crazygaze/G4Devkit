#ifndef _os_kernel_h
#define _os_kernel_h

#include "../process/process.h"
#include "utils/priorityqueue.h"

typedef void (*KrnTimedEventFunc)(void* data1, void* data2, void* data3);
typedef struct KrnTimedEvent {
	double execTime; // Time at which to execute the function
	KrnTimedEventFunc func;
	void* data0;
	void* data1;
	void* data2;
} KrnTimedEvent;
		

PQUEUE_TYPEDECLARE(KrnTimedEvent)

typedef struct Kernel  {

	// The currently running thread. This is set by the task scheduler.
	
	TCB* currTcb;
	
	// Thread to switch to when there is no work
	TCB* idleTcb;
	
	// Total number of times the interrupt handler was called
	u32 irqCount;

	// How often the scheduler was executed
	u32 schedulerTicks;
	
	// Process ID counter.
	// This is incremented every time a process is created.
	u32 pidCounter;
	
	FullCpuCtx krnCtx;
	
	// Queue with threads that are ready to run
	Queue32 tcbReady;
	
	// Set to the current time when entering the interrupt handler.
	double intrCurrSecs;
	
	// Queue with events that need to happen at specific times.
	// This can be used to trigger things at specified time intervals.
	PQueue_KrnTimedEvent timedEvents;
	
	struct
	{
		u32 reason;
		u32 data1;
		u32 data2;
	} intrData;
	
} Kernel;

extern Kernel krn;


/*!
 * Sets the tls pointer for the current thread
 */
void krn_setCurrTCBTlsSlots(void);

/*!
 * Picks the next thread to run
 */
void krn_pickNextTcb(void);

////////////////////////////////////////////////////////////////////////////////
//
//               Kernel Timed Events functionality
//
////////////////////////////////////////////////////////////////////////////////


/*!
 * Adds a timed events.
 *
 * \param execTime time at which the event should be executed
 * \param func Function to execute
 * \param data1,data2,data3 Parameters to pass to the function
 */
void krn_addTimedEvent(double execTime, KrnTimedEventFunc func, void* data1,
	void* data2, void* data3);

#endif

