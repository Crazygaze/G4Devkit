#ifndef _assert_shared_h_
#define _assert_shared_h_

#ifdef NDEBUG
	#define assert(condition) ((void)0)
#else
	#define assert(condition) if (!(condition)) __asm("\tdbgbrk\n")
#endif

#define always_assert(condition)  if (!(condition)) __asm("\tdbgbrk\n")

#endif
