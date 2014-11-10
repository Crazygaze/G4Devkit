
/*******************************************************************************
* Common code shared between the APPSDK and the Kernel
*******************************************************************************/
#ifndef _APCPU_SYNCPRIMITIVES_SHARED_H_
#define _APCPU_SYNCPRIMITIVES_SHARED_H_

#include "stddef_shared.h"
#include "syscalls_shared.h"

/*!
* Does an atomic compare and swap.
* \param address
*	int* address to change
* \param key
*	The value the address must have in order for the operation to succedeed
* \param newval
*	The value to be copied to the address, if [address]==key
* \return
* 	TRUE if successfully changed the value. False otherwise.
*/
#define compare_and_swap(address,key,newval) \
	(compare_and_swap_helper(address,key,newval)==(key))

/*!
* Helper to compare_and_swap. Most probably you don't need to use this directly.
* Checks the value at the specified address with the provided key. If they
* match, newval is copied to the address. It's preferable to you 
* \return
*	It returns the value encountered at the address prior to trying the change.
*	This means that to check if the operation succeeded the caller should do:
*	if (compare_and_swap_helper(addr, key)==key) {
*		// success
*	}
*/
int compare_and_swap_helper( 
	__reg("r1") int* /*address*/, // Note that I'm changing the order of the
	__reg("r0") int /*key*/     , // registers, so I don't need to move the key
	__reg("r2") int /*newval*/  ) // back to r0 ( Function return with r0)
INLINEASM("\t\
cmpxchg [r1], r0, r2");
// mov r0, r1
// This extra instruction would be required if I used the typical r0,r1,r2,r3
// register order for parameters.
// This way, the return value is already in r0 as a result of the cmpxchg
// instruction


/*! Atomically increments the integer at the specified address
*/
void atomic_increment(int* counter);

/*! Atomically decrements the integer at the specified address
*/
void atomic_decrement(int* counter);


/*!
 * Non recursive critical section
 * Critical sections allows exclusive access to a resource, but only on the same
 * process.
 * They are seriously faster than Mutexes, and should be prefered, whenever
 * possible
 */ 
typedef struct CriticalSection {
	// Thread that has this CriticalSection currently locked (if it's locked)
	HANDLE ownerThread;
	int* ownerThread2;
	
	// Counter to detect recursive locks in the same thread
	// For every "Enter" this is increased.
	// For every "Leave" this is decreased. If it reaches 0, it performs the
	// real unlock
	int lockCount;
} CriticalSection;

void criticalSectionInit(CriticalSection* cs);
void criticalSectionEnter(CriticalSection* cs);
void criticalSectionLeave(CriticalSection* cs);


#endif
