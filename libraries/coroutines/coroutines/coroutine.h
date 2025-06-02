#ifndef _coroutine_h_
#define _coroutine_h_

#include <stddef.h>
#include <hwcrt0.h>

//
// Compile with _COROUTINES_LOG set to 0 or 1 to force a specific behaviour
//
#ifndef _COROUTINES_LOG
	// If not specified, then we enable or disable depending on build type
	#ifdef _DEBUG
		#define _COROUTINES_LOG 1
	#else
		#define _COROUTINES_LOG 0
	#endif
#endif

//
// Define logging macros that automatically logs the current's coroutine's
// pointer.
// Note that the automatically added information is co_this(), which is the
// current context, NOT necessarily what coroutine the function is modifying.
//
#if _COROUTINES_LOG
	#include <stdc_init.h>

	#define CO_LOGF(fmt, ...) _stdc_log("LOG:CO_%p: " fmt, co_this(), __VA_ARGS__)
	#define CO_LOG(msg) _stdc_log("LOG:CO_%p: " msg, co_this())
#else
	#define CO_LOGF(...)
	#define CO_LOG(...)
#endif


/*!
 * If set to 1, it uses full context switch mode, which requires the cpu to be
 * in supervisor mode.
 */
#ifndef _COROUTINES_FULLCTXSWITCH
	#define _COROUTINES_FULLCTXSWITCH 0
#endif

/*! Bit mask to extract the state from the "flags" field */
#define CO_STATE_MASK 0x3

/*! Coroutine was created, but waiting to be executed for the first time */
#define CO_STATE_STARTING 0

/*! If this bit is 1, it indicates the  Coroutine is active */
#define CO_STATE_ACTIVE 1

/*!
 * If this bit is 1, it indicates the coroutine finished execution, either by
 * returning from the entry function, an explicit co_destroy or co_exit.
 */
#define CO_STATE_FINISHED 2

/*! The coroutine is the main program */
#define CO_ISMAIN 0x4

#if _COROUTINES_FULLCTXSWITCH
	#define CoCpuCtx FullCpuCtx
#else
	#define CoCpuCtx UserCpuCtx
#endif // _COROUTINES_FULLCTXSWITCH

/*!
 * Type for the coroutine entry function
 *
 * \param yielded
 * Value supplied in the "co_yield" call that switched execution to this
 * coroutine.
 */
typedef int (*CoroutineEntry)(int yielded);

/*!
 * The entire coroutine's state
 */
typedef struct Coroutine {
	//
	// Instead of having the cpu state in the struct itself, we place it in the
	// stack when suspending a coroutine. This has the advantage of potentially
	// saving memory since it's reusing stack space.
	// The downside is the programmer needs to specify a stack big enough to
	// account for the worst case scenario where co_yield is called deep in the
	// coroutine's callstack. If the programmer will be calling co_yield at the
	// top level, instead of deep in the coroutine callstack, that's where more
	// memory can be saved.
	CoCpuCtx* ctx;

	uint32_t flags;
	void* stackBuffer;
	uint32_t stackSize;
	CoroutineEntry entryFunc;
	int result;
	struct Coroutine* previous;
	struct Coroutine* next;
	void* cookie;
} Coroutine;

/*!
 * Creates a new coroutine
 *
 * \param co Coroutine to initialize
 *
 * \param stack
 * Buffer to use for stack. Make sure the supplied stack is big enough for
 * what the coroutine needs to do. There are no checks in place to avoid a stack
 * overflow whatsoever.
 *
 * \param stack Size of the stack buffer
 *
 * \param entry Entry function.
 *
 * \param cookie
 * "cookie" for the coroutine. The Coroutine's "cookie" field will be set to
 * this, and allows passing a context to the coroutine.
 *
 * The created coroutine will added to the chain and will be ready to run with a
 * call to co_yield.
 */
void co_create(Coroutine* co, void* stack, int stackSize, CoroutineEntry entry, void* cookie);

/*!
 * Destroys the specified coroutine.
 *
 * \param co Coroutine to destroy.
 * \param yielded Value to finish the coroutine with.
 *
 * NOTE: If "co" is the currently executing coroutine, this function never
 * returns, since the coroutine is destroyed and execution transfered to the
 * next coroutine in the chain. That coroutine that is resumed will receive the
 * `yielded` value specified in this co_destroy call.
 */
void co_destroy(Coroutine* co, int yielded);

/*!
 * Switches execution to the specified coroutine
 *
 * \param co
 * Coroutine to switch to.
 * If this is a coroutine which is no longer active or it's the main program
 * this function call returns immediately with a return value of 0. If this is
 * NULL, it will switch to the next coroutine in the chain.
 *
 * \param yielded
 * Value to pass to the coroutine that will be resumed.
 * If the coroutine to execute is a new coroutine that will start execution
 * with this call, this value will be passed as a parameter to the entry
 * function. If its a previously suspended coroutine, it will be the return
 * value of the co_yield function that caused that coroutine to be suspended.
 *
 * \return
 * Value passed back from whatever coroutine return control back to the
 * caller. Note that this value might not necessarily come from the coroutine
 * that this co_yield call resumed, since that coroutine that was resumed might
 * have in turn resume another coroutine.
 */
int co_yield(Coroutine* co, int yielded);

/*!
 * Calculates how much stack the specified coroutine is using.
 *
 * This is done by initializing the stack buffer with a magic value of
 * 0xCCCCCCCC, so that when this function is called it can detect what parts of
 * the buffer changed. Please note that this method is not 100% reliable, since
 * the used magic value is after all a value that the program might put on the
 * stack and thus it can cause the calculation to fail and return a lower value
 * than what is really being used.
 */
int co_calcUsedStack(Coroutine* co);

/*!
 * Gets the state of the specified coroutine.
 * 
 * The returned value can e.g compared with CO_STATE_FINISHED to check if the
 * coroutine has finished execution.
 */
#define co_getState(co) ((co)->flags&CO_STATE_MASK)

/*!
 * Returns the currently executing coroutine, or NULL if no coroutines were
 * created yet.
 * \Note The main program itself is also treated as a coroutine.
 */
Coroutine* co_this(void);

/*!
 * Returns true if there are any coroutines left alive (excluding the main
 * program), or false otherwise. A coroutine is considered alive if it's in any
 * state other than CO_STATE_FINISHED.
 */
bool co_hasAlive(void);

#endif
