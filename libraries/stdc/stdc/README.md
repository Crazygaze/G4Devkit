# What is this library

This is rather minimal implementation of the standard C library.
Far from complete, and will be expanded as needed.

# Compilation options

## __STDC_HOSTED__

Set to 0 or 1. Default is 0.

If defined and not 0, the library will be compiled in hosted mode, which
requires an OS, so it can provide things such as file IO.

Default is freestanding mode (evaluated to 0), where only functionality that
doesn't need OS support is available.

NOTE: This uses naming different from the other macros, so it matches:
https://gcc.gnu.org/onlinedocs/gcc-3.3.1/cpp/Standard-Predefined-Macros.html

## _STDC_SHARED_DATA

Set to 0 or 1. Default is 0

If defined and not 0, global data will be put into .data_shared/.bss_shared
sections instead of .data/.bss.

This is useful because if using the library as standalone in a small
application, there is no need to use shared data and thus all the data can be in
.data/.bss

The library's project file doesn't have this defined, so it can by default be
directly used in small standalone applications.
This allows enabling this feature on a case-by-case basis, by adding the project
as-is, and then adding the define to the workspace itself, which is inherited
by the project when compiling.

## _STDC_LOG_VERBOSITY

Controls how much log verbosity is compiled into the code.
If not set, it is defined to a sensible value for Debug and Release builds, 
where Debug build has more verbosity than Release.
