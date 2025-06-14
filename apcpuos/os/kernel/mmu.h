#ifndef _os_mmu_h
#define _os_mmu_h

#include "../os_config.h"

//
// PTE keys
//
// Kernel mode should have access
#define MMU_PTE_KEY_KRN (1 << 0)
// User mode should have access
#define MMU_PTE_KEY_USR (1 << 1)

// Keys to the original .data_shared/.bss_shared sections
// Those sections are unacessible by default to help catch bugs.
// Whenever the kernel needs access to those, it needs to have this key
#define MMU_PTE_KEY_ORIGINAL_SHARED (1 << 2)


typedef struct PageTable
{
	struct PCB* owner;
	
	// Each process has 1 page table, which is used when in user mode and kernel
	// mode.
	// There is no need to have a separate page table for kernel mode, because
	// we can lock access to the kernel region by setting the right PTE keys.
	
	// Actual page table data
	u32* data;
	
	// What value to set the `ds` register to.
	u32 usrDS;
	
	u32 stackEnd; // What value to set the main thread's `sp` register to
	u32 stackBegin;

	// #TODO : See if this is being used. If not, remove it.
	u32 heapBegin;
	u32 heapEnd;
	
	// Program break
	// See https://en.wikipedia.org/wiki/Sbrk 
	// This can go up to `heapEnd`
	u32 brk;
} PageTable;

/*!
 * Information about a page table's user space
 * Begin/End are addresses. Begin is inclusive, End is exclusive.
 * The fields are ordered to match the layout (e.g kernel at the bottom)
 */
typedef struct PTUsrLayout {
	//
	// User space
	//
	u32 sharedDataEnd;
	u32 sharedDataBegin;
	
	u32 stackEnd; // Stack top (full slot)
	u32 stackBegin; // Stack bottom
	
	u32 stackGuardEnd;
	u32 stackGuardBegin;
	
	u32 heapEnd;
	u32 heapBegin;
} PTUsrLayout;


/*!
 * Information about a page table's kernel space
 * Begin/End are addresses. Begin is inclusive, End is exclusive.
 * The fields are ordered to match the layout (e.g kernel at the bottom)
 */
typedef struct PTKrnLayout {
	u32 ioEnd;
	u32 ioBegin;

	u32 heapEnd;
	u32 heapBegin;
	
	u32 sharedDataEnd;
	u32 sharedDataBegin;

	u32 stackEnd; // Stack top (full slot)
	u32 stackBegin; // Stack bottom

	u32 oriSharedDataEnd;
	u32 oriSharedDataBegin;
	
	u32 dataEnd;
	u32 dataBegin;

	u32 rodataEnd;
	u32 rodataBegin;
	
	u32 textEnd;
	u32 textBegin;
} PTKrnLayout;


/*!
 * Returns the number of PTE entries for just kernel space
 */
inline u32 mmu_krnPTECount(const PTKrnLayout* krn)
{
	return MMU_ADDR_TO_PAGE(krn->ioEnd);
}

/*!
 * Returns the number of PTE entries for user space.
 * Note that since kernel space is always below a user space, the value returned
 * is the number of PTE for a full page table (kernel + user spaces)
 */
inline u32 mmu_usrPTECount(const PTUsrLayout* usr)
{
	return MMU_ADDR_TO_PAGE(usr->sharedDataEnd);
}

/*!
 * Returns the number of PTE entries in the specified page table
 */
inline u32 mmu_getPTECount(const PageTable* pt)
{
	return MMU_ADDR_TO_PAGE(pt->data[0]);
}

/*!
 * Returns the specified PTE's information
 */
inline u32 mmu_getPTE(const PageTable* pt, u32 index)
{
	return pt->data[index+1]; // +1 because we want to skip the header
}


/*!
 * Initializes the mmu.
 *
 * This includes initializing the memory layout.
 *
 */
void mmu_init(void);

/*!
 * Allocates IO memory.
 * This can be used when configuring a device that needs memory mapping
 *
 * \return The pointer to use for the mapping, or 0 if allocation failed.
 */
void* mmu_allocIO(u32 size);


/*!
 * Return the page table that is used for kernel only processes
 */
PageTable* mmu_getKrnOnlyPT(void);

PTKrnLayout* mmu_getKrnPTLayout(void);

/*!
 * Creates a new user space page table.
 *
 * \param stackSize stack size required by the process. This is an hint, and the
 * actual stack size might be bigger.
 *
 * \param heapNPages Desired heap size. This is an hint, and the actual heap
 * size might end up being bigger.
 *
 * \return
 * The page table on success, NULL on failure.
 * mmu_destroyPT should be used to destroy the page table.
 */
PageTable* mmu_createUsrPT(u32 stackSize, u32 heapNPages);

/*!
 * Destroy the specified page table.
 * It releases any necessary resources and clears the struct.
 */
void mmu_destroyPT(PageTable* pt);

/*!
 * Sets a range of virtual pages for the user space of the specified page table
 *
 * \param pt The page table to set
 *
 * \param vbegin, vend Virtual addresses to set. vend is exclusive.
 *
 * \param allocPages if true, it will allocate physical pages. If false, it will
 * not allocate physica pages and therefore trying to access the address will
 * cause an Abort.
 *
 * \param rwxbits Values for the RWX bits. Note that the keys bits are set
 * depending on the value of `allocPages`
 *
 * \return true on success, false on failure. If the operation, the caller
 * should make use of the 
 * 
 */
bool mmu_setUsrRange(PageTable* pt, u32 vbegin, u32 vend, bool allocPages, u32 rwxbits);

void mmu_ppBeginTransaction(PageTable* pt);
void mmu_ppFinishTransaction(bool commit);

/*!
 * Releases all the pages owned by the specified page table
 * \return The total number of pages released
 */
u32 mmu_freePages(PageTable* pt);

/*!
 * Frees the pages in the range [vbegin, vend)
 */
void mmu_freePageRange(PageTable* pt, u32 vbegin, u32 vend);

/*!
 * Logs the mmu state tracking
 */
void mmu_debugdumpState(void);

/*!
 * Logs the contents of the specified page table
 */
void mmu_debugdumpPT(const PageTable* pt);

/*!
 * Returns the kernel stack size in bytes
 */
u32 mmu_getKrnStackSize(void);

/*!
 * Calculate how much of the stack the kernel used so far.
 * This is not guaranteed to be correct, because the stack usage is calculated
 * by initializating the stack stack with magic number, which this function then
 * checks to see what portion of the stack the kernel has used so far.
 */
u32 mmu_calcKrnUsedStack(void);

#endif
