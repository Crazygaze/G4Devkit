

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


# TLS (Thread Local Storage)

Thread Local storage is implemented with minimal help from the kernel :

* There is a single pointer (`u32* appTlsSlots;`) in the .rodata section, defined
  in the boot assembly, so we can make sure it's in the first .rodata page.
	* The reason it is in the .rodata section is because the page tables don't
	  use address translation for .rodata, so the address is fixed for every
	  process, and therefore easiser for the kernel to udpate that when
	  switching threads.
* The TCB struct has a `u32* tlsSlotsPtr;` pointer.
* When a thread starts, pushes an `u32` array to its stack. This array is the
  tls slots. It then passes that array's pointer to the `app_setTlsPtr system
  call. This system calls sets the TCB's `tlsSlotsPtr` to that pointer.
* When the kernel switches threads, it sets the global `appTlsSlots` pointer
  to the TCB's own pointer.
* Any manipulation of the TLS slots is then done completely in user space

# Pitfalls when working in the kernel

When dealing with kernel code, especially when the code needs to temporarily
enable/disable something, execute some code, then revert the temporary change,
care must be taken so that the compiler doesn't optimize away the temporary
change.
E.g, this code that given a PTE pointer, addes temporary Write access, makes
the desired change, then removes the temporary Write access:
```
	int page = MMU_ADDR_TO_PAGE((u32)&appTlsSlots);
	u32* pte = &((u32*)hwcpu_get_crpt())[1+page];
	u32 original = *pte;
	*pte |= MMU_PTE_W; // 1. Give temporary Write permissions
	appTlsSlots = krn.currTcb->tlsSlotsPtr; // Do the desired change
	*pte = original; // 2. Restore the original PTE value
```

Line (1.) can be optimized out by the compiler, since it seems that the value
will be overwritten by (2.)

Possible solutions are marking using `volatile u32* pte`, or inserting
some inlined assembly after (1.), so the compiler can't optimize it away since
it won't be able to prove it doesn't change the behaviour.

