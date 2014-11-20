
#ifndef _utilshared_config_h_
#define _utilshared_config_h_


#ifdef APCPU
	#include <assert_shared.h>
	#include <string_shared.h>
	#include <stdint_shared.h>
	#include <stdlib_shared.h>
#else
	#include <stdint.h>
	#include <string.h>
	#include <stdlib.h>
	#include <stddef.h>
	#include <stdio.h>
	#include <assert.h>

	typedef int bool;
	#define true 1
	#define false 0
	#define TRUE 1
	#define FALSE 0
	#define FALSE 0

	#define TEST_RINGBUFFER 1
	#define TEST_QUEUE 1
	#define TEST_LINKEDLIST 1
#endif

#define utilshared_memset(ptr, val, size) memset(ptr, val, size)
#define utilshared_malloc(size) malloc(size)
#define utilshared_realloc(ptr,size) realloc(ptr,size)
#define utilshared_free(ptr) free(ptr)
#define utilshared_memcpy(dst,src,size) memcpy(dst,src,size)
#define utilshared_log printf

#ifndef min
	#define min(a,b) ((a)<=(b) ? (a) : (b))
#endif
#ifndef max
	#define max(a,b) ((a) >(b) ? (a) : (b))
#endif

#define utilshared_check(expr) \
	if (!(expr)) { \
		utilshared_log("check(%s) failed", ##expr); \
		assert(0); \
	}

#endif
