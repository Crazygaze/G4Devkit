/*
 * Manages memory pages on an high level
 */
 
#ifndef _APCPU_MMU_H_
#define _APCPU_MMU_H_

#include "kernel/kerneldefs.h"
#include "stddef_shared.h"
#include "stdint_shared.h"


typedef enum MMUMemAccess {
	kMMUAccess_Read=0xFF<<(2*8),
	kMMUAccess_Write=0xFF<<(1*8),
	kMMUAccess_Execute=0xFF<<(0*8)
} MMUMemAccess;

#define MMU_TABLE_SIZE (SIZE_TO_PAGES(ramAmount)*4)

/*!
 * Checks if the specified process has access to the specified pointer.
 * This is used to validate user pointers passed to the kernel, otherwise,
 * a malicious process can exploit system calls by passing pointers pointing to
 * wherever it wants.
 */
bool mmu_check_user(struct PCB *pcb, MMUMemAccess access, void* addr, size_t size);


/*!
 * Initializes the MMU
 * \param krnFirstPage First page to reserve for kernel memory
 * \param krnNumPages How many pages to reserve for kernel memory
 * \param mmutableaddr
 *	Address to use for the mmu table. The memory pointed to be at least 
 * the size of MMU_TABLE_SIZE
 */
 void mmu_init(size_t krnFirstPage, size_t krnNumPages, void* mmutableaddr);


/*! Sets a range of pages as belonging to the specified process
* \note
*	No checks are made if the pages already belong to another process.
*	So, use the other functions to first check for free pages
*/
void mmu_setPages(int firstPage, int numPages,
	uint8_t pid, bool read, bool write, bool execute);



/*!
 * Tries to find adjacent free pages.
 * \return
 *		>=0 : First page of the reserved range
 *		-1  : Not enough adjacent pages available.
 */ 
int mmu_findFreePages(int numPages);

/*! Find and sets free adjacent pages in one call.
* \return
*	First page, or -1 if it failed to find enough adjacent free pages
*/
int mmu_findFreeAndSet(int numPages, uint8_t pid, bool read, bool write,
	bool execute);

/*!
 * Sets all pages of the specific process ID as not used anymore
 */
void mmu_freePages(uint8_t pid);

/*! Sets a range of pages as not used anymore
 */
void mmu_freePagesRange(int firstPage, int numPages);


/*!
 * Counts how many pages belong to the specified process
 */
int mmu_countPages(uint8_t pid);

#define MMU_KEY(readKey,writeKey,executeKey) \
	((readKey)<<16 | (writeKey)<<8 | (executeKey))
	
#define ADDR_TO_PAGE(addr) ((uint32_t)(addr) / MMU_PAGE_SIZE)
#define PAGE_TO_ADDR(page) ((uint8_t*)((uint32_t)(page) * MMU_PAGE_SIZE))
#define SIZE_TO_PAGES(size) \
	(((size) / MMU_PAGE_SIZE) + (((size)%MMU_PAGE_SIZE) ? 1 : 0))
#define PAGES_TO_SIZE(pages) \
	((pages)*MMU_PAGE_SIZE)

// Special values for the page permissions keys
#define MMU_ANY 0
#define MMU_NONE 255

#endif

