#ifndef _os_config_h
#define _os_config_h

#include "hwcrt0.h"
#include <stdc_init.h>

// How many handles the OS can have at one time
#define OS_MAXHANDLES 64

// Time in milliseconds spent per thread.
// The smaller it is, the more the OS will be doing context switches, which adds
// overhead
// See https://en.wikipedia.org/wiki/Scheduling_%28computing%29
#define KERNEL_QUANTUM 100

// Kernel's stack size in bytes
// This is specified in bytes instead of pages, so we can potentially save some
// space because of the way the page tables are arranged.
#define KERNEL_STACK_SIZE 2048

// How many pages to use for the kernel heap
#define KERNEL_HEAP_NUMPAGES 4

// How many pages to reserve for device mapped memory.
// If at boot the OS detects this is not enough for all the attached devices,
// it will fail to boot.
// Specifying an higher value than what's required for the devices attached at
// boot allows new devices to be plugged at runtime.
#define KERNEL_IO_PAGES 2

// Stack size (in bytes) for the page fault handler
#define PFHANDLER_STACK_SIZE 1024

#define krnassert assert

#if DEBUG
	#define krnverify(expr)                     \
		if (!(expr)) {                          \
			OS_FTL("Verify failed: %s", #expr); \
			hwcpu_hlt();                        \
		}
#else
	#define krnverify(expr) \
		if (expr) { }
#endif


#if DEBUG
	#define OS_DOLOG 1
#else
	#define OS_DOLOG 1
#endif

#ifdef OS_DOLOG
	#define OS_FTL(...) LOG_FTL("OS:" __VA_ARGS__)
	#define OS_ERR(...) LOG_ERR("OS:" __VA_ARGS__)
	#define OS_WRN(...) LOG_WRN("OS:" __VA_ARGS__)
	#define OS_LOG(...) LOG_LOG("OS:" __VA_ARGS__)
	#define OS_VER(...) LOG_VER("OS:" __VA_ARGS__)

	#define HW_FTL(...) LOG_FTL("HW:" __VA_ARGS__)
	#define HW_ERR(...) LOG_ERR("HW:" __VA_ARGS__)
	#define HW_WRN(...) LOG_WRN("HW:" __VA_ARGS__)
	#define HW_LOG(...) LOG_LOG("HW:" __VA_ARGS__)
	#define HW_VER(...) LOG_VER("HW:" __VA_ARGS__)
#else
	#define OS_FTL(...)
	#define OS_ERR(...)
	#define OS_WRN(...)
	#define OS_LOG(...)
	#define OS_VER(...)

	#define HW_FTL(...)
	#define HW_ERR(...)
	#define HW_WRN(...)
	#define HW_LOG(...)
	#define HW_VER(...)
#endif

#endif
