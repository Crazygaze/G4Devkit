/*
 * Manages memory pages on an high level
 */
 
#include "multitasking/process.h"
#include "mmu.h"
#include "boot/boot.h"
#include "extern/tlsf/tlsf.h"
#include "kernel/kerneldebug.h"
#include "hw/hwscreen.h"
#include "hw/hwcpu.h"
#include "kernel/kernel.h"

#include <stdlib_shared.h>
#include "utilshared/misc.h"

typedef struct MMUData
{
	uint32_t* table;
	uint32_t numPages;
} MMUData;
MMUData mmu;

/*
MMU NOTES

The mmu table consists of a word for every page, with the keys for access, which
are compared against the keys in the flags register.
The format for words in the mmu table is as follow
this format:
.. XX XX XX
|  |  |  !-- Execute key
|  |  !----- Write key
|  !-------- Read key
!----------- Not used by the hardware. The code can use it for whatever it wants
             In our case, we use it to tell if the page is used or not.

The keys contained in the flags register are in the same format:
.. XX XX XX
|  |  |  !-- Execute key for the current process
|  |  !----- Write key for the current process
|  !-------- Read key for the current process
!----------- The other cpu flags

	--- Values in the mmu table entries ---
	
- A key of 0 in the mmu table means no restrictions to any processes.
- A key of 0xFF (255) in the mmu table means no access at all.
- A key of 1..254 means only a process with the same key as access, or a process
  with an override (see bellow)

Examples of mmu entries:
0x00 00 00 00
	No Restrictions at all
0x00 FF FF 00
	No read or write permission, but any process can execute
0x00 01 01 00
	Process with key 0x01 can read and write, and any process can execute

	--- Values in the flags register ---
	
The keys in the flags register can override the mmu table, using the special
value of 0x00 in that key. E.g:

0x .. 00 00 00
	The process has free access to any pages

0x .. 01 01 00
	- The process has read and write access to pages that specify also have the
	   read and write key as 0x01
	- The process has free execute permissions on any page


Now, with the considerations above, and considering the kernel process key is
0x01, the MMU code sets the memory permission as follow:

     SECTIONS                             MMU TABLE KEY
---------------------------------- |--------------------------------------------
| ... SCREEN BUFFER ...          | | 0x000000FF (free read/write)
|                                | | 
---------------------------------- | -------------------------------------------
|                                | | 0x00FFFFFF (No permissions at all)
| ...   N PAGES  ...             | | Once given to a process, page permissions
|                                | | are set according to that process, like:
|                                | | 0x00kkkkFF (read/write for that process)
---------------------------------- |--------------------------------------------
| ... kernel heap pages          | | 0x000101FF (Kernel can read/write)
|                                | |
|   |           .              | | |
|   |           .              | | |
|   |           .              | | |
|   |           .              | | |
|   | Kernel dynamic memory    | | |
|   |--------------------------| | |
|   | MMU table                | | |
|   |--------------------------| | |
|   | Kernel (data/bss)_shared | | |
|   |--------------------------| | |
|   | Kernel stack             | | | -> Stack is right a the bottom so we can
|   |                          | | | detect stack overflow, since it will crash
|   !--------------------------! | | trying to write to the pages bellow
---------------------------------- | -------------------------------------------
| .data_shared/.bss_shared pages | | 0x0001FFFF (Kernel can read)
---------------------------------- |--------------------------------------------
| .data/.bss pages               | | 0x000101FF (kernel can read/write)
---------------------------------- |--------------------------------------------
| .text/.rodata pages            | | 0x0000FF00 (no write at all)
---------------------------------- ---------------------------------------------


*/

bool mmu_check_user(struct PCB *pcb, MMUMemAccess access, void* addr, size_t size)
{
	uint32_t page1 = ADDR_TO_PAGE(addr);
	uint32_t page2 = ADDR_TO_PAGE((uint8_t*)addr+size-1);
	if (page1>=mmu.numPages || page2>=mmu.numPages)
		return FALSE;

	uint32_t processKeys = pcb->mainthread->cpuctx->flags & 0x00FFFFFF;
	uint32_t prckeymask = processKeys & access;
	uint32_t p1mask = access & mmu.table[page1];
	uint32_t p2mask = access & mmu.table[page2];
	
	if ( prckeymask==0 || // Current ctx has access, overriding whatever the mmu table says
		((p1mask==0 || p1mask==prckeymask) && (p2mask==0 || p2mask==prckeymask))){
		return TRUE;
	} else {
		return FALSE;
	}
}

void mmu_setPages(int firstPage, int numPages,
	uint8_t pid, bool read, bool write, bool execute)
{
	uint32_t key = pid<<24 |
		MMU_KEY(read ? pid : MMU_NONE, write ? pid : MMU_NONE, execute ? pid : MMU_NONE);
		
	for(int i=firstPage; i<firstPage+numPages; i++) {
		mmu.table[i] = key;
	}	
}

int mmu_findFreePages(int numPages)
{
	int firstPage=0;
	int found=0;
	for(int i=0; i<mmu.numPages; i++) {
		if (mmu.table[i]>>24 == PID_NONE) {
			if (found==0)
				firstPage = i;
			found++;
			if (found==numPages)
				return firstPage;
		} else {
			found = 0;
		}
	}
	
	return -1;
}

int mmu_findFreeAndSet(int numPages, uint8_t pid, bool read, bool write,
	bool execute)
{
	int firstPage = mmu_findFreePages(numPages);
	if (firstPage==-1)
		return -1;
	mmu_setPages(firstPage, numPages, pid, read, write, execute);
	return firstPage;
}

void mmu_freePages(uint8_t pid)
{
	uint32_t key = PID_NONE<<24 | MMU_KEY(MMU_NONE, MMU_NONE, MMU_NONE);
	for(int i=0; i<mmu.numPages; i++) {
		if ((mmu.table[i]>>24)==pid)
			mmu.table[i] = key;
	}
}

void mmu_freePagesRange(int firstPage, int numPages)
{
	uint32_t key = PID_NONE<<24 | MMU_KEY(MMU_NONE, MMU_NONE, MMU_NONE);
	for(int i=firstPage; i<firstPage+numPages; i++) {
		mmu.table[i] = key;
	}
}

int mmu_countPages(uint8_t pid)
{
	int count=0;
	for(int i=0; i<mmu.numPages; i++) {
		if ((mmu.table[i]>>24)==pid)
			count++;
	}
	return count;
}



/*
Sets permissions for pages in the memory range [startAddr ... startAddr+size[
*/
void mmu_setPagesInMemRange(
	void* startAddr, uint32_t size, uint8_t pid, uint32_t key)
{
	uint32_t firstPage = ADDR_TO_PAGE(startAddr);
	uint32_t numPages = SIZE_TO_PAGES(size);
	
	for(int i=firstPage; i<firstPage+numPages; i++) {
		mmu.table[i] = pid<<24 | key;
	}
}

void mmu_init(size_t krnFirstPage, size_t krnNumPages, void* mmutableaddr)
{
	mmu.numPages = SIZE_TO_PAGES(ramAmount);
	mmu.table = (uint32_t*)mmutableaddr;
	
	assert( isAligned(processInfo.readOnlyAddr,MMU_PAGE_SIZE) );
	assert( isAligned(processInfo.readWriteAddr,MMU_PAGE_SIZE) );
	assert( isAligned(processInfo.sharedReadWriteAddr,MMU_PAGE_SIZE) );
	
	/* By default, no access at all */
	mmu_setPagesInMemRange(0, ramAmount, PID_NONE, MMU_KEY(MMU_NONE, MMU_NONE, MMU_NONE));

	/* setup .text/.rodata */
	mmu_setPagesInMemRange(
		(void*)processInfo.readOnlyAddr, processInfo.readOnlySize,
		PID_KERNEL, MMU_KEY(MMU_ANY, MMU_NONE, MMU_ANY));

	/* setup .data/.bss */
	// Kernel can read/write
	mmu_setPagesInMemRange(
		(void*)processInfo.readWriteAddr, processInfo.readWriteSize,
		PID_KERNEL, MMU_KEY(PID_KERNEL,PID_KERNEL,MMU_NONE));
	
	/* setup .data_shared/.bss_shared */
	// Kernel can read. No write at all (so we can use this data to initialize
	// other processes shared data)
	mmu_setPagesInMemRange(
		(void*)processInfo.sharedReadWriteAddr, processInfo.sharedReadWriteSize,
		PID_KERNEL, MMU_KEY(PID_KERNEL,MMU_NONE,MMU_NONE));
		
	/* set kernel heap pages */
	mmu_setPages( krnFirstPage, krnNumPages, PID_KERNEL, true, true, false);

	/* Reserve the last few pages for the screen buffer */
	uint32_t screenBufferSize = hw_scr_getBufferSize();	
	mmu_setPagesInMemRange((void*)(ramAmount-screenBufferSize),screenBufferSize,
		PID_KERNEL, MMU_KEY(MMU_ANY,MMU_ANY,255));

	// set the mmu table
	hw_cpu_setMMUTable((uint32_t)(mmu.table));

	// Switch back to our own context, to trigger the mmu to update
	hw_cpu_ctxswitch( (CpuCtx*)(&intrCtxStart) );
	
}
