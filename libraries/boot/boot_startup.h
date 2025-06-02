#ifndef _boot_startup_h_
#define _boot_startup_h_

#include <hwcrt0.h>

/*!
 * How many bytes to reserve for the stack.
 *
 * At boot, the boot code, decides how much of the free memory should be used
 * for the heap and for the stack.
 */
#define BOOT_STACK_SIZE (20*1024)

/*!
 * If set to 1, the MMU will be enabled.
 * This causes the MMU key in the flags to be set to 1, and it creates a 1 to 1
 * page table, but without any RWX restrictions.
 * In short, this doesn't create any restrictions and is simply useful to
 * measure what overhead the MMU has on the VM execution speed.
 */
#define MMU_ENABLED 0

/*!
 * Value of the SP register at boot
 */
extern uint32_t initialSP;

#endif
