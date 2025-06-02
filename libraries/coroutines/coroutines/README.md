# What is this library

As the name suggests, this library implements coroutines.
See https://en.wikipedia.org/wiki/Coroutine for an explanation.

# Compilation options

## _COROUTINES_FULLCTXSWITCH

Set to 0 or 1. Default is 0.

If defined and not 0, it will use full context switching mode.
The APCPU cpu allows two types of context switch.

* Full context switch, which requires the cpu to be in supervisor mode.
* User mode context switch 

Full context switch swaps the entire register set (minus some control registers)
User mode context switch swaps the registers that only user mode has access to.

## _COROUTINES_SHARED_DATA

Set to 0 or 1. Default is 0

If defined and not 0, global data will be put into .data_shared/.bss_shared
sections instead of .data/.bss.

This is useful because if using the library as standalone in a small
application, there is no need use shared data and thus all the data can be in
.data/.bss

The library's project file doesn't have this defined, so it can by default be
directly used in small standalone applications.
This allows enabling this feature on a case-by-case basis, by adding the project
as-is, and then adding the define to the workspace itself, which is inherited
by the project when compiling.
