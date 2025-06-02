#ifndef _os_syscalls_h
#define _os_syscalls_h

#include "appsdk/os_shared/syscalls_shared.h"

/*!
 *  System calls don't take any parameters, since they use the global kernel
 * structures to do their work
 */
typedef bool (*krn_syscallFunc)(void);

extern const krn_syscallFunc krn_syscalls[kSysCall_Max];

#endif

