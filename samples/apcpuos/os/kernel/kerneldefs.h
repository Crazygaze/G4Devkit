/*******************************************************************************
 * Miscellaneous kernel and low level settings
 ******************************************************************************/

#ifndef _APCPUOS_KERNELDEFS_H_
#define _APCPUOS_KERNELDEFS_H_

#define CPU_NUM_GREGS 16
#define CPU_NUM_FREGS 16

#define CPU_REG_DS 11
#define CPU_REG_SP 13
#define CPU_REG_PC 15

#define MMU_PAGE_SIZE (1024*1)

#define CPU_FLAGSREG_SUPERVISOR 26

#define INTRCTX_ADDR 32

// Stack space for the kernel process/thread.
// Needs to be 4 word aligned
#define KERNEL_STACKSIZE (1024*4)
#define KERNEL_HEAPSIZE (1024*10)


// Time slice in milliseconds
// The smaller it is, more often the threads get interrupted, and more time
// wasted in the kernel scheduling
#define THREAD_TIME_SLICE 100

//
// Interval between process stats updates
//
#define STATS_UPDATE_INTERVAL 1.0f

//////////////////////////////////////////////////////////////////////////
//  Various flags for tweaking things
//////////////////////////////////////////////////////////////////////////


/*!
* Whenever the kernel needs to read/write data from a process address space,
* it can either do one of the following:
*
* - Change the mmu table entries relative to that process.
*   This is the safest option to make sure the kernel code is correct, since we
*   will be giving the kernel access to the bare minimum it needs, therefore
*   helping catching bugs in the kernel. It's the default option for debug
*   builds.
*
* - Override the mmu settings.
*   This is the fastest option, but will disable memory protection for the
*	kernel, and if that portion of the kernel code that disables the memory
*	protection has memory overruns and such, it won't be detected.
*	Best used for release builds, or places where the code that needs access is
*	minimal.
*
*/
#define DEBUGBUILD_FAST_USERSPACE_CHECK 0
#define RELEASEBUILD_FAST_USERSPACE_CHECK 1


// If 1, causes the kernel initialization to fail, to test failure detection
#define TEST_KERNEL_INIT_FAIL 0

// If 1, causes task initialization to fail, to test task failure detection
#define TEST_TASKBOOT_FAIL 0

// If 1, causes the an explicit ctxswitch to the kernel (which is not supported)
#define TEST_UNEXPECTED_CTXSWITCH 0

#endif
