Library with miscellaneous code that is shared between the OS and the
applications.

# What is this library

Miscellaneous utility code that require no OS support, and can thus be easily
shared between projects.

# Compilation options

## _UTILS_SHARED_DATA

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
