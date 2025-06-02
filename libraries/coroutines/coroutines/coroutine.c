#include "coroutine.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifndef _COROUTINES_SHARED_DATA
	#define _COROUTINES_SHARED_DATA 0
#endif

#if _COROUTINES_SHARED_DATA
	_Pragma("shareddata-on")
#endif

typedef struct {
	// To save the main state
	Coroutine main;
	Coroutine* curr;
} Coroutines;
static Coroutines gCo;

//
// Internal functions
//
static void setState(Coroutine* co, u32 state)
{
	CO_LOGF("setting state of %p to %u", co, state);
	co->flags = (co->flags & ~CO_STATE_MASK) | state;
}

static void finish(Coroutine* co, int yielded)
{
	CO_LOGF("finishing %p with %d", co, yielded);
	setState(co, CO_STATE_FINISHED);
	co->result = yielded;

	// Remember the next in the chain
	Coroutine* next = co->next;

	// Remove from chain
	co->next->previous = co->previous;
	co->previous->next = co->next;
	// nullify next/previous, to help catch any bugs
	co->next = NULL;
	co->previous = NULL;

	// If it's the currently executing coroutine, then we need to switch
	// execution to the next one
	if (co == gCo.curr)
		co_yield(next, co->result);
}

static void runCoroutine(int yielded, CoroutineEntry entry)
{
	CO_LOG("started");
	Coroutine* co = gCo.curr;
	setState(co, CO_STATE_ACTIVE);
	CO_LOGF("calling user function with %d", yielded);
	u32 ret = entry(yielded);
	CO_LOGF("finished user function with %u", ret);
	finish(co, ret);
}

//
// USER API
//
void co_create(Coroutine* co, void* stack, int stackSize, CoroutineEntry entry,
			   void* cookie)
{
	CO_LOGF("creating %p, stack=%p, stackSize=%d, entryFunc=%p, cookie=%p",
		co, stack, stackSize, entry, cookie);
		
	// When we create the first coroutine, we initialize what we need to save
	// the state of the main program
	if (co_getState(&gCo.main) != CO_STATE_ACTIVE) {
		CO_LOGF("initializing main co as %p", &gCo.main);
		setState(&gCo.main, CO_STATE_ACTIVE);
		gCo.main.flags |= CO_ISMAIN;
		gCo.main.next = &gCo.main;
		gCo.main.previous = &gCo.main;
		gCo.curr = &gCo.main;
	}

	memset(co, 0, sizeof(*co));

	setState(co, CO_STATE_STARTING);

	//
	// setup stack
	//
	co->stackBuffer = stack;
	co->stackSize = stackSize;
	memset(stack, 0xCC, stackSize);
	co->entryFunc = entry;
	co->cookie = cookie;

	//
	// Insert the new coroutine at the end of the chain
	//
	// Main "previous" points to the last coroutine
	Coroutine* last = gCo.main.previous;
	// setup next direction
	last->next = co;
	co->next = &gCo.main;
	// setup previous direction
	gCo.main.previous = co;
	co->previous = last;
}

void co_destroy(Coroutine* co, int yielded)
{
	// Nothing to do if already finished, and we can't touch anything
	// if its the main program
	if (co_getState(co) == CO_STATE_FINISHED) {
		CO_LOGF("can't destroy %p because it is finished already", co);
		return;
	}

	if (co->flags & CO_ISMAIN) {
		CO_LOGF("can't destroy %p because it is main co", co);
		return;
	}
	
	CO_LOGF("destroying %p with %d", co, yielded);
	finish(co, yielded);
}

Coroutine* co_this(void)
{
	return gCo.curr;
}

bool co_hasAlive(void)
{
	// If "main" points to itself, then it's the only one in the chain
	return gCo.main.next == &gCo.main ? false : true;
}

static void co_prepareFirstExecution(Coroutine* co)
{
	CO_LOGF("preparing %p for first execution", co);
	// When we are switching execution for the first time to a coroutine, it
	// doesn't yet have a context saved, so we use this global one as a
	// kickstarter. It can be shared because we only use it once per coroutine
	static CoCpuCtx gStarterCtx;
	co->ctx = &gStarterCtx;
	co->ctx->gregs[CPU_REG_SP] = (u32)co->stackBuffer + co->stackSize;
	// setup where to start executing, and parameters to the execution function
	co->ctx->gregs[CPU_REG_PC] = (u32)&runCoroutine;
	
	// Setup the shared data base register to match the creator, otherwise
	// if the coroutine tries to use any data in the .data_shared/.bss_shared,
	// it will corrupt things.
	co->ctx->gregs[CPU_REG_DS] = hwcpu_get_ds();

#if _COROUTINES_FULLCTXSWITCH
	// We set flag register to match the creator, keeping everything except the
	// compare bits
	co->ctx->crregs[CPU_CRREG_FLAGS] =
		hwcpu_get_crflags() & (~CPU_CRREG_FLAGS_USERMODE_MASK);
#endif // _COROUTINES_FULLCTXSWITCH

	// Setup parameters for runCoroutine
	// NOTE: We don't need to set r0, because that's set when running the
	// coroutine, as part of co_yield
	co->ctx->gregs[1] = (u32)co->entryFunc;
}

int co_yield(Coroutine* co, int yielded)
{
	// By design, if a coroutine is still in the chain, then it can't be
	// finished, so the FINISHED check is not needed in that case.
	// That check is only needed if using the coroutine passed by the caller
	if (co == NULL) {
		co = gCo.curr->next;
		CO_LOGF("yielding to next in chain (%p)", co);
	} else if (co_getState(co) == CO_STATE_FINISHED) {
		CO_LOGF("yield to %p failed because its already finished", co);
		return 0;
	}
	else
	{
		CO_LOGF("yielding to %p", co);
	}

	// If we are just switching to this coroutine for the first time we need
	// to prepare the temporary cpu context
	if (co_getState(co) == CO_STATE_STARTING) {
		co_prepareFirstExecution(co);
	}
	
#if _COROUTINES_FULLCTXSWITCH
	CO_LOGF("fullctxswitch to %p", co);
#else
	CO_LOGF("ctxswitch to %p", co);
#endif

	// Set r0 of the context to resume, so that it gets our yield value
	co->ctx->gregs[0] = yielded;
	Coroutine* curr = gCo.curr;
	gCo.curr = co;
	
	// Setup a place to save the cpu context of "this" coroutine
	CoCpuCtx currCtx;
	curr->ctx = &currCtx;

	// This saves the currently running coroutine to the stack (&currCtx),
	// and resumes specified coroutine (co)
#if _COROUTINES_FULLCTXSWITCH
	uint32_t retRegs[4];
	int ret = hwcpu_fullctxswitch(co->ctx, &currCtx, retRegs);
#else
	int ret = hwcpu_ctxswitch(co->ctx, &currCtx);
#endif
	CO_LOG("resumed");

	// When resuming execution and leaving co_yield, we need to clear "ctx",
	// because it is located in the stack.
	curr->ctx = NULL;
	
	return ret;
}

int co_calcUsedStack(Coroutine* co)
{
	if (co == NULL) {
		return 0;
	}

	// When inspecting the stack buffer, we use ints instead of bytes, since
	// the changes of the program using a full 0xCCCCCCCC value are lower than
	// single 0xCC bytes.
	// As a bonus, this also makes the code slightly faster (less iterations
	// and comparisons).
	int words = co->stackSize / sizeof(int);
	u32* buf = co->stackBuffer;

	int i = 0;
	for (i = 0; i < words; i++) {
		if (buf[i] != 0xCCCCCCCC) {
			break;
		}
	}

	u32 freeBytes = i * sizeof(int);
	int used = co->stackSize - freeBytes;
	
	CO_LOGF("co %p used %d bytes of stack", co, used);
	return used;
}
