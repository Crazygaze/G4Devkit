

# Memory layout

Each process has it's own page table.
The kernel space is located at the bottom of the address space, and the user
space at the top.
This is so that the page table's size can vary per process. As-in, if a process
doesn't require much heap or stack, it's page table can be smaller.

Since the OS doesn't yet have a dymamic loader, any applications are built-in
into the firmware itself and are part of the OS.
This means that both the kernel and applications share some pages, such as the
`.text` and `.rodata` sections. This is considered bad practice, but despite
this, a process's page table is setup with restrictions that won't let a user
process corrupt the kernel.

Once the OS has a dynamic loader, the situation might change so that the kernel
and applications don't share any code or data pages.

```

#TODO : Consider getting rid of the Kernel IO Pages, since that can probably
be part of the kernel heap, and thus we automatically get alloc/free for IO
memory

----------------------------------------------------------------
|      .data_shared/.bss_shared                                |
| The start is NOT aligned, to save memory, since both stack   |
| and shared data might be small                               |
---------------------------------------------------------------- <- NOT aligned
|                  User dynamic stack                          |
| Starts as 1 page, grows as required, but has a predefined    |
| max size                                                     |
----------------------------------------------------------------
|      Unmapped user dynamic stack (N Pages)                   |
----------------------------------------------------------------
|             Always unmapped (stack guard)                    |
| By having this always unmapped page, we can detect stack     |
| overflows                                                    |
----------------------------------------------------------------
|      Unmapped user heap (N Pages)                            |
| The maximum heap size is set by the OS                       |
----------------------------------------------------------------
|                     User heap                                |
| Starts as 0 pages, and can grow up to a maximum of N pages,  |
| where N is specified by the OS.                              |
----------------------------------------------------------------
---------------------------------------------------------------- ^ user space
----------------------------------------------------------------
|         Kernel device IO pages (Kernel=RW, User=None)        |
| May contain unmapped pages, to allow plug and play devices   |
| that need memory mapping                                     |
----------------------------------------------------------------
|            Kernel heap (Kernel=RW, User=None)                |
| The start is NOT aligned, to save memory, since .data_shared |
| /.bss_shared is likely small                                 |
---------------------------------------------------------------- <- NOT aligned
| Kernel's .data_shared/.bss_shared (Kernel=RW, User=None)     |
| The start is NOT aligned, to save memory by not requiring the|
| kernel stack to be a multiple of the page size               |
---------------------------------------------------------------- <- NOT aligned
|            Kernel Stack (Kernel=RW, User=None)               |
| Right at the bottom so we can detect stack overflows, when   |
| trying to write over the region below                        |
----------------------------------------------------------------
|   original .data_shared/.bss_shared (Kernel=None, User None) |
|                                                              |           
| This stays as restricted and it's set to Kernel-Read when    |
| creating a process, so it can be copie to the process.       |
----------------------------------------------------------------
|           .data/.bss (Kernel=RW, User=None)                  |
----------------------------------------------------------------
|               .rodata (Kernel=R, User=R)                     |
----------------------------------------------------------------
|                .text (Kernel=X, User=X)                      |
---------------------------------------------------------------- <- Address 0

```
