#ifndef _appsdk_syscalls_h_
#define _appsdk_syscalls_h_

#include <stdint_shared.h>
#include "kernel_shared/syscalls_shared.h"

/*
 * Emits a swi instruction
 * Don't use these directly. Use the macros bellow
 */
int app_syscall0(
	__reg("r10") int)
INLINEASM("\t\
swi");

int app_syscall1(
	__reg("r10") int,
	__reg("r0") int)
INLINEASM("\t\
swi");

int app_syscall2(
	__reg("r10") int,
	__reg("r0") int,
	__reg("r1") int)
INLINEASM("\t\
swi");

int app_syscall3(
	__reg("r10") int,
	__reg("r0") int,
	__reg("r1") int,
	__reg("r2") int)
INLINEASM("\t\
swi");

int app_syscall4(
	__reg("r10") int,
	__reg("r0") int,
	__reg("r1") int,
	__reg("r2") int,
	__reg("r3") int)
INLINEASM("\t\
swi");

#endif
