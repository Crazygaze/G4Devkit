#ifndef _os_syscalls_h
#define _os_syscalls_h

#include "appsdk/os_shared/syscalls_shared.h"

/*!
 *  System calls don't take any parameters, since they use the global kernel
 * structures to do their work
 */
typedef bool (*krn_syscallFunc)(void);

extern const krn_syscallFunc krn_syscalls[kSysCall_Max];

// By default the kernel doesn't have access to user space, to help catch
// bugs in the kernel.
// Whenever the kernel needs access to user space memory, it gives itself
// temporary acess to the user space
#define ADD_USER_KEY hwcpu_addMMUKeys(MMU_PTE_KEY_USR)
#define REMOVE_USER_KEY hwcpu_removeMMUKeys(MMU_PTE_KEY_USR)

#endif

