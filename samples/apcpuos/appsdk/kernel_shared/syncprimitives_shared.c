#include "assert_shared.h"
#include "string_shared.h"
#include "syncprimitives_shared.h"
#include "app_process.h"



/*******************************************************************************
*		Atomic primitives
*******************************************************************************/

void atomic_increment(int* counter)
{
	bool success;
	int oldCount, newCount;
	do {
		oldCount = *counter;
		newCount = oldCount+1;
	} while( !compare_and_swap(counter, oldCount, newCount));
}

void atomic_decrement(int* counter)
{
	bool success;
	int oldCount, newCount;
	do {
		oldCount = *counter;
		newCount = oldCount-1;
	} while( !compare_and_swap(counter, oldCount, newCount));
}

/******************************************************************************
		Critical sections

This critical section implementation is not really ideal, but good enough for
now, and I'm quite sure it's reliable.
Getting locking primitives right is not an easy thing, but there is a list of
things that could be improved:

- There is no queue of threads waiting for the critical section. If a thread
can't get hold of the lock, it will keep yielding and retrying. This should be
ok for now, since due to the round-robin nature of the OS thread scheduler,
yielding will eventually cause the thread that has the lock to execute again
and very likely to release the lock before we try again.
The worst case scenario would be when the locking thread holds the lock for too
long and so we will end up wasting cpu time retrying and yielding.
By following good multithreading programming practices and not holding locks for
too long should make this solution good enough.

*******************************************************************************/

void criticalSectionInit(CriticalSection* cs)
{
	memset(cs, 0, sizeof(*cs));
}

int* intptr;
void criticalSectionEnter(CriticalSection* cs)
{
	HANDLE h = app_getThreadHandle();
	
	// This is to support recursivity. If this thread already owns the critical
	// section, then we only need to increment the counter
	if (cs->ownerThread!=h)
	{
		while(!compare_and_swap(
						(int*)(&cs->ownerThread), INVALID_HANDLE,
						(int)h) ) {
			app_yield();
		}
	}
	
	cs->lockCount++;
	
	//LOG("CS Locked: owner=%d, lockCount=%d", h, cs->lockCount);
}

void criticalSectionLeave(CriticalSection* cs)
{
	//LOG("CS unlocking: owner=%d, lockCount=%d",
	//		app_getThreadHandle(), cs->lockCount);

	assert(cs->ownerThread==app_getThreadHandle());
	assert(cs->lockCount>0);
	
	// This it to support recursivity.
	// We only release the critical section when the counter reaches 0
	if ((--cs->lockCount)==0) {
		cs->ownerThread = INVALID_HANDLE;
	}
}
