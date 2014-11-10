/*******************************************************************************
 * Code for debugging kernel stuff
 ******************************************************************************/

#ifndef _APCPU_KERNELDEBUG_H_
#define _APCPU_KERNELDEBUG_H_

//////////////////////////////////////////////////////////////////////////
// Available tweaks
//////////////////////////////////////////////////////////////////////////

/*
 If 1, whenever a panic occurs, a debug message will be sent over
 the NIC debug channel
*/
#if DEBUG
	#define DEBUG_NICLOGPANIC 1
#else
	#define DEBUG_NICLOGPANIC 0
#endif

int krn_debugOutput(const char* fmt, ...);
#if DEBUG
	#define KERNEL_DEBUG krn_debugOutput
#else
	#define KERNEL_DEBUG(...) (void)(0)
#endif

void krn_PanicImpl(const char* file, int line, const char* fmt, ...);

#ifdef DEBUG
	#define kernel_assert(condition) \
		if(!(condition)) krn_PanicImpl(__FILE__, __LINE__ , #condition)
#else
	#define kernel_assert(condition)
#endif

#define kernel_check(condition) \
	if(!(condition)) krn_PanicImpl(__FILE__, __LINE__ , #condition)

#define krn_panic(fmt,...) krn_PanicImpl(__FILE__,__LINE__, fmt, __VA_ARGS__)

/*
Used during the boot process, to print simple messages to the screen.
The cursor screen advances and also processes \n characters.
*/
void krn_bootLog(const char* fmt, ...);


// Forces a crash to happen.
// Useful to test error handling
void krn_forceCrash(void);

#endif

