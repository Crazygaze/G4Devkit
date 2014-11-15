/*******************************************************************************
 * Main Kernel code
 ******************************************************************************/

#ifndef _APCPU_KERNEL_H_
#define _APCPU_KERNEL_H_

#include "kerneldefs.h"
#include "utilshared/queue32.h"
#include "utilshared/priorityqueue32.h"
#include "utilshared/priorityqueue.h"
#include <stddef_shared.h>
#include "utilshared/dynamicarray.h"
#include "appsdk/kernel_shared/process_shared.h"

typedef enum KrnStatsMode {
	kKrnStats_None,
	kKrnStats_Cpu,
	kKrnStats_ProcessPages,
	kKrnStats_KernelMem,
	kKrnStats_MAX
} KrnStatsMode;

typedef struct Kernel {

	struct PCB* kernelPcb;
	
	// While handling an interrupt, points to the thread that was interrupted
	// NOTE: Only valid while handling an interrupt
	struct TCB* interruptedTcb;

	// The thread to run next. This is set by the task scheduler
	struct TCB* nextTcb;
	
	// The cpu idle process, which runs whenever there are no other processes
	// ready to run.
	struct TCB* idlethread;
	
	// TODO : delete this
	KrnStatsMode statsMode;

	// Process with focus
	struct PCB* focusedPcb;

	Queue32 tcbReady;
	
	// Queue with events that need to happen at specific times.
	// This can be used to trigger things at specified time intervals.
	PriorityQueue timedEvents;
	
	// Struct that can be passed to applications with misc OS information
	OSInfo info;
} Kernel;


extern Kernel krn;

/*! Sets the thread to run next 
*/
void krn_pickNextTcb(void);

//! Spins for the specified milliseconds
void krn_spin(unsigned int ms);


#endif


