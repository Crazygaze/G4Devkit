#ifndef _assert_h_
#define _assert_h_

#include <stddef.h>

#ifdef NDEBUG
	// For Release build, asserts don't do anything
	#define assert(condition) ((void)0)
#else
	#define assert(condition) if (!(condition)) __asm("\tdbgbrk\n")
#endif

/*!
* This one is not a standard function, but it's handy for when you want to
* assert something even on Release build
*/
#define always_assert(condition)  if (!(condition)) __asm("\tdbgbrk\n")

#endif
