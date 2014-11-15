#ifndef _APCPU_SYSCALLS_H_
#define _APCPU_SYSCALLS_H_

#include "appsdk/kernel_shared/syscalls_shared.h"
#include <stddef_shared.h>

//
// System calls don't take any parameters, since they use the global kernel
// structures to do their work.
//
// The return value indicates if the process is allowed to continue.
// Note that it doesn't mean the call is successfull from the calling app
// perspective. That will be available to the app through the r0 register.
// The intention of the return value is to inform the kernel if the calling app
// did something that is not allowed, and should be killed
typedef bool (*krn_syscallFunc)(void);
extern krn_syscallFunc krn_syscalls[kSysCall_Max];

#endif

