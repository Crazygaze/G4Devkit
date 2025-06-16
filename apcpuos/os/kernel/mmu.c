#include "mmu.h"
#include "kernel.h"
#include "hwcrt0.h"
#include "utils/bitops.h"
#include <stdlib.h>
#include <stdio.h>

/*!
 * Physical Page information.
 * This tracks the state a single physical page.
 */
typedef struct PPInfo
{
	// What page table this physical page belongs to, which also means what
	// process owns this page.
	PageTable* owner;

	// What virtual page (within the owner's page table) this physical page is
	// being used for
	u32 vpage;
} PPInfo;

typedef struct MMU
{
	// How much ram the system has in pages
	u32 totalPages;
	
	// The page table layout for the kernel address range
	PTKrnLayout krn;

	// Where the IO alloc cursor is at the moment
	u32 currIOEnd;
	
	// Array of physical PPInfo.
	PPInfo* ppinfo;
	
	PageTable* krnOnlyPT;
	
	// If we are trying to allocate multiple physical pages, this is set to
	// the page table we are dealing with.
	// It allows having a concept of page allocation transations.
	PageTable* ptTransaction;
} MMU;


MMU mmu;

/*

See https://en.wikipedia.org/wiki/Page_table

Each PTE (Page Table Entry) is 32-bits (1 word), represents a single virtual
page, and has the following information:
--------------------------------------------------------------------------------
                                 PTE Layout
--------------------------------------------------------------------------------
\3\3\2\2\2\2\2\2\ \2\2\2\2\1\1\1\1\ \1\1\1\1\1\1\ \ \ \ \ \ \ \ \ \ \ \ 
 \1\0\9\8\7\6\5\4\ \3\2\1\0\9\8\7\6\ \5\4\3\2\1\0\9\8\ \7\6\5\4\3\2\1\0\
  [            Physical Page                ] R W X A   D [  PTE Keys ]

Physical Page:
	Set by the OS. Specifies the physical page to which the virtual page is
	mapped to.
R, W, X :
	Set by the OS. Indicates what operations (Read, Write, Execute) are allowed
	on this virtual page
A :
	Accessed bit. The MMU sets this bit to 1 when the page is accessed.
D :
	Dirty bit. The MMU sets this bit to 1 upon a write operation to the page.
PTE Keys :
	Set by the OS. The MMU does a bitwise AND of this with the keys in crflags.
	If the resulting AND is !=0, access if granted.
*/


inline u32 pageAlign(u32 val)
{
	return ALIGN(val, MMU_PAGE_SIZE);
}

void mmu_calcKrnPTLayout(void)
{
	PTKrnLayout* krn = &mmu.krn;

	//
	// Validate section alignments as the OS expects
	//
	
	// .rodata must be separate from .text, so we can have .text be eXecute only
	// and .rodata Read only.
	krnassert(ISALIGNED(gROMInfo.rodataAddr, MMU_PAGE_SIZE));
	// .data+.bss must be aligned
	krnassert(ISALIGNED(gROMInfo.dataAddr, MMU_PAGE_SIZE));
	// .data_shared/.bss_shared separate from .data/.bss so we can set it as
	// unaccessible or Read-Only when we need to duplicate it for a new process
	krnassert(ISALIGNED(gROMInfo.dataSharedAddr, MMU_PAGE_SIZE));
	
	//
	// Calculate the layout for the kernel space (bottom of the page table)
	//
	
	krn->textBegin = 0;
	krn->textEnd = krn->textBegin + gROMInfo.textSize;
	
	krn->rodataBegin = gROMInfo.rodataAddr;
	krn->rodataEnd = krn->rodataBegin + gROMInfo.rodataSize;
	
	krn->dataBegin = gROMInfo.dataAddr;
	krn->dataEnd = krn->dataBegin + gROMInfo.dataSize;
	
	krn->oriSharedDataBegin = gROMInfo.dataSharedAddr;
	krn->oriSharedDataEnd = krn->oriSharedDataBegin + gROMInfo.dataSharedSize;

	krn->stackBegin = pageAlign(krn->oriSharedDataEnd);
	krn->stackEnd = krn->stackBegin + KERNEL_STACK_SIZE;
	
	krn->sharedDataBegin = krn->stackEnd;
	krn->sharedDataEnd = krn->sharedDataBegin + gROMInfo.dataSharedSize;
	
	krn->heapBegin = krn->sharedDataEnd;
	// We align the heap end to the page boundary, since that would go to waste
	// anyway.
	krn->heapEnd = pageAlign(krn->heapBegin + KERNEL_HEAP_NUMPAGES * MMU_PAGE_SIZE);

	krn->ioBegin = pageAlign(krn->heapEnd);
	krn->ioEnd = krn->ioBegin + KERNEL_IO_PAGES*MMU_PAGE_SIZE;
}

/*!
 * Calculates a page table's layout.
 *
 * \return The number of pages required for the page table or 0 on error.
 */
u32 mmu_calcPTLayout(u32 maxStackBytes, u32 maxHeapBytes, PTUsrLayout* usr)
{
	// There needs to be some stack. The process can't function without stack
	if (maxStackBytes == 0)
	{
		OS_ERR("Page table always needs stack pages");
		return 0;
	}
	
	PTKrnLayout* krn = &mmu.krn;
	
	//
	// Calculate the layout for the user space (top of the page table)
	//
	
	usr->heapBegin = krn->ioEnd;
	if (maxHeapBytes == 0) {
		// If no heap required, then we don't reserve any pages for heap
		usr->heapEnd = usr->heapBegin;
	} else {
		usr->heapEnd = pageAlign(usr->heapBegin + maxHeapBytes);
	}
	
	usr->stackGuardBegin = usr->heapEnd;
	if (usr->heapBegin == usr->heapEnd) {
		// If there is heap, then we also don't need the stack guard, because we'll
		// be able to detect stack overflows if the stack grows to try and write
		// over the kernel pages.
		usr->stackGuardEnd = usr->stackGuardBegin;
	} else {
		// 1 page of stack guard
		usr->stackGuardEnd = usr->stackGuardBegin + MMU_PAGE_SIZE;
	}
	
	usr->stackBegin = usr->stackGuardEnd;
	// The user shared data is right at the top of the page table, AND right
	// above the user stack.
	// Since their boundary doesn't need to be aligned, we adjust things so that
	// the shared data stays right at the top of the last page and we expand
	// adjust the stackEnd to also use what would be a wasted partial page if
	// we had set the shared data to start at a page boundary.
	usr->sharedDataEnd = pageAlign(usr->stackBegin + maxStackBytes + gROMInfo.dataSharedSize);
	usr->sharedDataBegin = usr->sharedDataEnd - gROMInfo.dataSharedSize;
	usr->stackEnd = usr->sharedDataBegin;
	
	return mmu_usrPTECount(usr);
}

/*!
 * Set's the `ds` register and returns the kernel's stack top.
 * This is called first thing from the boot assembly
 */
u32 mmu_preInit(void)
{
	mmu_calcKrnPTLayout();

	// First thing we need to do before accessing global data is to setup the
	// ds register, otherwise if we end up accessing any global data in
	// `.data_shared` or `.bss_shared`, it will use the wrong address because
	// the `ds` register has a value of `0` at boot.
	hwcpu_set_ds(mmu.krn.sharedDataBegin);
	
	// Copy the shared data to the kernel' own copy, since we need to keep the
	// original one untouched so it can be copied to other processes.
	memcpy((void*)mmu.krn.sharedDataBegin, (void*)gROMInfo.dataSharedAddr,
		gROMInfo.dataSharedSize);

	// set the kernel stack to a magic number we can look for to calculate how
	// much stack is being used. It's not 100% accurate, but it gives us an
	// idea.
	memset((void*)mmu.krn.stackBegin, 0xCC, mmu_getKrnStackSize());

	// Point the IO allocation cursor to the right location
	mmu.currIOEnd = mmu.krn.ioBegin;
	
	return mmu.krn.stackEnd;
}

u32 mmu_getKrnStackSize(void)
{
	return mmu.krn.stackEnd - mmu.krn.stackBegin;
}

u32 mmu_calcKrnUsedStack(void)
{
	u32* ptr = (u32*)mmu.krn.stackBegin;
	u32* end = (u32*)mmu.krn.stackEnd;
	while(*ptr == 0xCCCCCCCC && ptr < end)
		ptr++;
	
	u32 used = (u32)end - (u32)ptr;
	return used;
}

void mmu_ppBeginTransaction(PageTable* pt)
{
	krnassert(mmu.ptTransaction == NULL);
	mmu.ptTransaction = pt;
}

/*!
 * This creates a dummy "owner" we can set the PPInfo entries to to indicate
 * that they are part of the current transaction.
 */
static inline PageTable* createTransactionTag(void)
{
	// Create the dummy pointer used to tag what pages are in the transation
	uint8_t* dummy = NULL;
	PageTable* tag = (PageTable*)(dummy - 1);
	return tag;
}

/*!
 * Allocates one physical page for the specified page table.
 *
 * \param vpage What virtual page are we getting a page for.
 *	This is just to build a debug relationship between ppinfo and the page
 *	table.
 * 
 * \returns the page number (>0) on success, or 0 on failure
 */
u32 mmu_ppAlloc(u32 vpage)
{
	krnassert(mmu.ptTransaction);
	
	PageTable* tag = createTransactionTag();
	u32 firstPage = mmu_krnPTECount(&mmu.krn);
	
	for (int i = firstPage; i < mmu.totalPages; i++) {
		if (mmu.ppinfo[i].owner == NULL) {
			mmu.ppinfo[i].owner = tag;
			mmu.ppinfo[i].vpage = vpage;
			return i;
		}
	}

	return 0;
}


void mmu_ppFinishTransaction(bool commit)
{
	krnassert(mmu.ptTransaction);
	
	PageTable* tag = createTransactionTag();
	u32 firstPage = mmu_krnPTECount(&mmu.krn);
	
	for (int i = firstPage; i < mmu.totalPages; i++) {
		if (mmu.ppinfo[i].owner == tag) {
			if (commit) {
				mmu.ppinfo[i].owner = mmu.ptTransaction;
			} else {
				mmu.ppinfo[i].owner = NULL;
				mmu.ppinfo[i].vpage = 0;
			}
		}
	}
	
	mmu.ptTransaction = NULL;
}

/*!
 * Releases all the pages owned by the specified page table
 * \return The total number of pages released
 */
u32 mmu_freePages(PageTable* pt)
{
	u32 firstPage = mmu_krnPTECount(&mmu.krn);
	u32 count = 0;
	for (int i = firstPage; i < mmu.totalPages; i++) {
		if (mmu.ppinfo[i].owner == pt) {
			mmu.ppinfo[i].owner = NULL;
			mmu.ppinfo[i].vpage = 0;
			count++;
		}
	}
	
	return count;
}

void mmu_freePageRange(PageTable* pt, u32 vbegin, u32 vend)
{
	// A range can be empty
	if (vend == vbegin)
		return;
		
	u32 pteCount = mmu_getPTECount(pt);
	u32* data = pt->data;
	data++; // Skip the PT header
	
	for (u32 i = MMU_ADDR_TO_PAGE(vbegin); i <= MMU_ADDR_TO_PAGE(vend - 1); i++) {
		krnassert(i < pteCount);
		u32 ppage = data[i] >> MMU_PAGE_OFFSET_BITS;
		if (ppage) {
			// Set the page to 0, remove the keys, and keep all the other bits.
			data[i] &= (MMU_PAGE_OFFSET_MASK & ~MMU_PTE_KEYS_MASK);
			
			krnassert(mmu.ppinfo[ppage].owner = pt);
			mmu.ppinfo[ppage].owner = NULL;
			mmu.ppinfo[ppage].vpage = 0;
		}
		
	}
	
}

/*!
 * Sets the virtual page for a range of PTEs.
 * The first PTE in the range gets set `firstpage`, the second to `firstpage+1`
 * and so on.
 *
 * \param pt The page table
 * \param begin First address (not page);
 * \param end Address past the last valid address 
 * \param firstpage first physical page to use.
 * \param bits Bits to set. A bitwise OR is done with the already existing
 * bits, therefore if no changes are desired to any bits, you should pass 0
 */
void mmu_setKrnRange(PageTable* pt, u32 begin, u32 end, u32 firstpage, u32 bits)
{
	// A range can be empty
	if (end == begin)
		return;
	
	u32 p = firstpage;
	u32* data = pt->data;
	data++; // Skip the PT header
	
	for (u32 i = MMU_ADDR_TO_PAGE(begin); i <= MMU_ADDR_TO_PAGE(end - 1); i++) {
		data[i] = (p << MMU_PAGE_OFFSET_BITS) | bits;
		p++;
	}
}

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
bool mmu_setUsrRange(PageTable* pt, u32 vbegin, u32 vend, bool allocPages, u32 rwxbits)
{
	// A range can be empty
	if (vend == vbegin)
		return true;

	u32* data = pt->data;
	data++; // Skip the PT header
	
	u32 bits = rwxbits;
	u32 keys = allocPages ? MMU_PTE_KEY_USR : 0;
	
	for (u32 i = MMU_ADDR_TO_PAGE(vbegin); i <= MMU_ADDR_TO_PAGE(vend - 1); i++) {
		u32 ppage = data[i] >> MMU_PAGE_OFFSET_BITS;
		// Only try to allocate if this PTE doesn't have one yet and the caller
		// requested to alloc
		if (ppage == 0 && allocPages) {
			ppage = mmu_ppAlloc(i);
			if (ppage == 0)
				return false;
		}

		// #TODO : These should be also part of a transaction somehow. As-in,
		// if the caller is doing a bunch of calls and one of them fails, we
		// need to revert the changes done to the page table.
		data[i] = (ppage << MMU_PAGE_OFFSET_BITS) | rwxbits | keys;
	}
	
	return true;
}

/*!
 * Setups the kernel space range of the given page table
 */
void mmu_setupKernelSpace(PageTable* pt)
{
	const PTKrnLayout* krn = &mmu.krn;
	krnassert(krn);
	
	mmu_setKrnRange(pt, krn->textBegin, krn->textEnd,
		MMU_ADDR_TO_PAGE(krn->textBegin),
		MMU_PTE_X | MMU_PTE_KEY_USR | MMU_PTE_KEY_KRN);
	mmu_setKrnRange(pt, krn->rodataBegin, krn->rodataEnd,
		MMU_ADDR_TO_PAGE(krn->rodataBegin),
		MMU_PTE_R | MMU_PTE_KEY_USR | MMU_PTE_KEY_KRN);
	mmu_setKrnRange(pt, krn->dataBegin, krn->dataEnd,
		MMU_ADDR_TO_PAGE(krn->dataBegin),
		MMU_PTE_RW | MMU_PTE_KEY_KRN);
	// The original shared data is set with keys `0`, and we only set the keys
	// when we want to access it duplicate it for another process
	mmu_setKrnRange(pt, krn->oriSharedDataBegin, krn->oriSharedDataEnd,
		MMU_ADDR_TO_PAGE(krn->oriSharedDataBegin),
		MMU_PTE_R | MMU_PTE_KEY_ORIGINAL_SHARED);
	mmu_setKrnRange(pt, krn->stackBegin, krn->stackEnd,
		MMU_ADDR_TO_PAGE(krn->stackBegin),
		MMU_PTE_RW | MMU_PTE_KEY_KRN);
	mmu_setKrnRange(pt, krn->sharedDataBegin, krn->sharedDataEnd,
		MMU_ADDR_TO_PAGE(krn->sharedDataBegin),
		MMU_PTE_RW | MMU_PTE_KEY_KRN);
	mmu_setKrnRange(pt, krn->heapBegin, krn->heapEnd,
		MMU_ADDR_TO_PAGE(krn->heapBegin),
		MMU_PTE_RW | MMU_PTE_KEY_KRN);
	mmu_setKrnRange(pt, krn->ioBegin, krn->ioEnd,
		MMU_ADDR_TO_PAGE(krn->ioBegin),
		MMU_PTE_RW | MMU_PTE_KEY_KRN);
}

static PageTable* mmu_createKrnPT(void)
{
	const PTKrnLayout* krn = &mmu.krn;
	krnassert(krn);
	
	u32 npages = mmu_krnPTECount(krn);
	
	// Allocate all in one chunk:
	// PageTable struct + Page table header + PTEs
	PageTable* pt = calloc(sizeof(PageTable) + sizeof(u32) + (npages * sizeof(u32)));
	if (!pt) {
		OS_ERR("Out of memory creating page table (%u pages)", npages);
		goto out1;
	}
	
	pt->data = (u32*)&pt[1];
	// Set the Page table header
	pt->data[0] = MMU_PAGE_TO_ADDR(npages);
	mmu_setupKernelSpace(pt);
	
	// Set the kernel as the owner of all the physical pages in kernel address
	// range
	for (int i = 0; i < npages; i++) {
		mmu.ppinfo[i].owner = pt;
		mmu.ppinfo[i].vpage = i;
	}
	
	pt->usrDS = hwcpu_get_ds();
	return pt;
	
	out1:
		return NULL;
}

void mmu_init(void)
{
	OS_LOG("Initializing MMU");
	
	ROMInfo* romInfo = &gROMInfo;
	OS_LOG("ROM Info:");
	OS_LOG("  .text                   : Addr=%8u(%08xh), Size=%8u",
		romInfo->textAddr, romInfo->textAddr, romInfo->textSize);
	OS_LOG("  .rodata                 : Addr=%8u(%08xh), Size=%8u",
		romInfo->rodataAddr, romInfo->rodataAddr, romInfo->rodataSize);
	OS_LOG("  .data+.bss              : Addr=%8u(%08xh), Size=%8u",
		romInfo->dataAddr, romInfo->dataAddr, romInfo->dataSize);
	OS_LOG("  .data_shared+.bss_shared: Addr=%8u(%08xh), Size=%8u",
		gROMInfo.dataSharedAddr, gROMInfo.dataSharedAddr,
		gROMInfo.dataSharedSize);
	OS_LOG("  Image size              : %u bytes",
		gROMInfo.dataSharedAddr + gROMInfo.dataSharedSize);

	u32 totalRam = hwcpu_getTotalRam();
	mmu.totalPages = MMU_SIZE_TO_PAGES(totalRam);
	OS_LOG("Total ram installed: %u bytes (%uKB, %u pages)", totalRam,
		   totalRam / 1024, mmu.totalPages);

	OS_LOG("Kernel stack size: %u bytes",
		   mmu.krn.stackEnd - mmu.krn.stackBegin);
	u32 kernelHeapSize = mmu.krn.heapEnd- mmu.krn.heapBegin;
	OS_LOG("Kernel heap size: %u bytes", kernelHeapSize);
	u32 krnIOBytes = mmu.krn.ioEnd - mmu.krn.ioBegin;
	OS_LOG("Kernel IO: %u bytes, (%uKB, %u pages)",
		krnIOBytes, krnIOBytes/1024, MMU_SIZE_TO_PAGES(krnIOBytes));
	OS_LOG("Kernel page table space: %uKB (%u pages)",
		(mmu_krnPTECount(&mmu.krn) * MMU_PAGE_SIZE) / 1024,
		mmu_krnPTECount(&mmu.krn));
		
	stdc_init((void*)mmu.krn.heapBegin, kernelHeapSize, NULL);
	
	// Since the kernel space is fixed, we don't need to track those pages, so 
	// the tracking array doesn't need to be as big.
	u32 krnNPages = mmu_krnPTECount(&mmu.krn);
	
	mmu.ppinfo = calloc(sizeof(PPInfo) * mmu.totalPages);
	
	mmu.krnOnlyPT = mmu_createKrnPT();
	krnverify(mmu.krnOnlyPT);
	
	hwcpu_setMMUKeys(MMU_PTE_KEY_KRN);
	hwcpu_set_crpt((u32)mmu.krnOnlyPT->data);
}

PageTable* mmu_getKrnOnlyPT(void)
{
	krnassert(mmu.krnOnlyPT)
	return mmu.krnOnlyPT;
}

PTKrnLayout* mmu_getKrnPTLayout(void)
{
	return &mmu.krn;
}

PageTable* mmu_createUsrPT(u32 stackSize, u32 heapNPages)
{
	krnassert(stackSize);
	
	PTUsrLayout usr = { 0 };
	
	u32 npages = mmu_calcPTLayout(stackSize, heapNPages * MMU_PAGE_SIZE, &usr);
	if (npages == 0)
		goto out1;
	
	const PTKrnLayout* krn = &mmu.krn;
	krnassert(krn);
	
	// Allocate all in one chunk.
	// The extra word is for the header
	PageTable* pt = calloc(sizeof(PageTable) + sizeof(u32) + (npages * sizeof(u32)));
	if (!pt) {
		OS_ERR("Out of memory creating page table (%u pages)", npages);
		goto out1;
	}
	
	pt->data = (u32*)&pt[1];
	pt->data[0] = MMU_PAGE_TO_ADDR(npages);
	pt->usrDS = usr.sharedDataBegin;
	pt->stackEnd= usr.stackEnd;
	pt->stackBegin = usr.stackBegin;
	pt->heapBegin = usr.heapBegin;
	pt->heapEnd= usr.heapEnd;
	pt->brk = usr.heapBegin;
	
	//
	// Setup kernel space
	//
	mmu_setupKernelSpace(pt);
		
	//
	// Setup user space
	//
	mmu_ppBeginTransaction(pt);
	
	// Heap
	// We setup the address range to have space for the desired heap size, but
	// we only actually allocate the first heap page.
	// The allocator will then request more pages from the os
	krnverify(mmu_setUsrRange(pt, usr.heapBegin, usr.heapEnd, false, MMU_PTE_RW));
	// Allocate the first page only (if we are indeed using the heap)
	if (usr.heapEnd != usr.heapBegin) {
		krnassert(ISALIGNED(usr.heapBegin, MMU_PAGE_SIZE));
		// +1 (1 byte), since the function will detect that as 1 whole page.
		krnverify(mmu_setUsrRange(pt, usr.heapBegin, usr.heapBegin+1, true, MMU_PTE_RW));
		pt->brk = usr.heapBegin + MMU_PAGE_SIZE;
	}
	
	// Stack
	// For the stack we allocate 1 page to begin with, and then as the stack
	// grows, we allocate more.
	//
	// Setup the stack range as not having have physical pages
	krnverify(mmu_setUsrRange(pt, usr.stackBegin, usr.sharedDataEnd, false,
		MMU_PTE_RW));
		
	// #TODO : Implement dynamic stack growing. We are only allocating one page
	// at the moment, which is what we want, but to make it dynamic we now need
	// to detect stack overflows and allocate pages as needed
	
	// Allocate 1 page of stack and all the pages for data_shared pages
	if (!mmu_setUsrRange(pt, usr.stackEnd-1, usr.sharedDataEnd, true, MMU_PTE_RW)) {
		goto out2;
	}
	
	mmu_ppFinishTransaction(true);
	return pt;
	
	out2:
		mmu_ppFinishTransaction(false);
		free(pt);
	out1:
		return NULL;
}

void mmu_destroyPT(PageTable* pt)
{
	// We are not allowed to destroy the kernel's main page table
	krnassert( pt != mmu.krnOnlyPT);
	
	mmu_freePages(pt);
	free(pt);
}

void* mmu_allocIO(u32 size)
{
	// Make sure the mmu is initialize
	krnassert(mmu.currIOEnd);
	
	u32 newEnd = mmu.currIOEnd + size;
	if (newEnd <= mmu.krn.ioEnd)
	{
		void* ptr = (void*)mmu.currIOEnd;;
		mmu.currIOEnd = newEnd;
		return ptr;
	} else {
		return NULL;
	}
}

#define INRANGE(value, begin, end) ((value) >= (begin) && (value) < (end))

static u32 calcSharedBoundary(u32 begin, u32 end)
{
	if (ISALIGNED(end, MMU_PAGE_SIZE))
		return 0;
	else
		return MMU_PAGE_TO_ADDR(MMU_ADDR_TO_PAGE(end-1));
}

/*!
 * Where the user address space begins
 */
inline const u8* mmu_userSpaceBeginAddr(void)
{
	return (const u8*)mmu.krn.ioEnd;
}

// To make this check as fast as possible, we only validate that the memory
// falls in the expected range(s) of the page table, instead of checking the pte
// contents. This is because this function is executed on behalf of the user
// process and thus we only care to make sure it is not allowed read/write
// kernel only data.
// If the process passes something that is outside the kernel address space but
// still invalid, it will crash since even though system calls are in supervisor
// mode, they run on behalf of the user process with it's page table.
bool mmu_checkUserPtr(struct PCB* pcb, bool needsWrite, void* addr, u32 size)
{
	const u8* begin = (const u8*)addr;
	const u8* end = begin + size;

	if (begin >= mmu_userSpaceBeginAddr() ||
		(needsWrite == false &&
			begin >= (u8*)mmu.krn.rodataBegin && end <= (u8*)mmu.krn.rodataEnd)) {
		return true;
	} else {
		OS_ERR("checkUserPtr failed: PCB=%p, access=%s, addr=%p, size=%u", pcb,
			needsWrite ? "RW" : "R", addr, size);
		return false;
	}
}

void mmu_debugdumpPT(const PageTable* pt)
{
	OS_LOG("-- Page table for %s --", pt->owner->info.name);
	
	const u32 pteCount = mmu_getPTECount(pt);
	const u32 firstUserPage = mmu_krnPTECount(&mmu.krn);

	//
	// Make some calculations so we can correctly display the information for 
	// PTEs that are not aligned (See the README.md file).
	u32 sharedBoundary1 =
		calcSharedBoundary(mmu.krn.stackBegin, mmu.krn.stackEnd);
	u32 sharedBoundary3 =
		calcSharedBoundary(mmu.krn.sharedDataBegin, mmu.krn.sharedDataEnd);
	u32 sharedBoundary2 = 0;
	if (sharedBoundary1 == sharedBoundary3)
		sharedBoundary2 = sharedBoundary1;
		
	u32 sharedBoundary4 =
		calcSharedBoundary(pt->stackBegin, pt->stackEnd);
	
	if (pteCount != firstUserPage)
		OS_LOG("    - USER SPACE -");
	
	for (int i = pteCount - 1; i >= 0; i--) {
	
		if (i == firstUserPage - 1)
			OS_LOG("    - KERNEL SPACE -");
			
		u32 pte = *mmu_getPTE(pt, i);
		u32 vaddr = i * MMU_PAGE_SIZE;
		const char* use = "";
		if (vaddr < mmu.krn.textEnd)
			use = ".text";
		else if (vaddr < mmu.krn.rodataEnd)
			use = ".rodata";
		else if (vaddr < mmu.krn.dataEnd)
			use = ".data/.bss";
		else if (vaddr < mmu.krn.oriSharedDataEnd)
			use = ".data_shared/.bss_shared";
		else if (vaddr == sharedBoundary2)
			use = "stack/.data_shared/.bss_shared/heap";
		else if (vaddr == sharedBoundary1)
			use = "stack/.data_shared/.bss_shared";
		else if (vaddr == sharedBoundary3)
			use = ".data_shared/.bss_shared/heap";
		else if (vaddr < mmu.krn.stackEnd) 
			use = "stack";
		else if (vaddr < mmu.krn.sharedDataEnd)
			use = ".data_shared/.bss_shared";
		else if (vaddr < mmu.krn.heapEnd)
			use = "heap";
		else if (vaddr < mmu.krn.ioEnd)
			use = "IO";
			
		// User space ...
		else if (vaddr < pt->heapEnd)
			use = "heap";
		else if (vaddr < pt->stackBegin)
			use = "stack guard";
		else if (vaddr == sharedBoundary4)
			use = "stack/.data_shared/.bss_shared";
		else if (vaddr < pt->stackEnd)
			use = "stack";
		else if (vaddr >= pt->usrDS)
			use = ".data_shared/.bss_shared";
		
		OS_LOG("|%3u->%3u %c%c%c %c%c %2x| %s",
			i, MMU_ADDR_TO_PAGE(pte),
			pte & MMU_PTE_R ? 'R' : '.',
			pte & MMU_PTE_W ? 'W' : '.',
			pte & MMU_PTE_X ? 'X' : '.',
			pte & MMU_PTE_A ? 'A' : '.',
			pte & MMU_PTE_D ? 'D' : '.',
			pte & MMU_PTE_KEYS_MASK,
			use
		);
	}
}

void mmu_debugdumpState(void)
{
	int columns = 0;
	char buf[128];
	buf[0] = 0;
	
	char* ptr = buf;
	
	OS_LOG("-- mmu.ppinfo --");
	for (int i = mmu.totalPages - 1; i >= 0; i--) {
		if (mmu.ppinfo[i].owner) {
			// Get the owning PCB
			PCB* owner = mmu.ppinfo[i].owner->owner;
			
			// Figure out what this page is being used for
			char use = ' ';
			const u32 vaddr = mmu.ppinfo[i].vpage * MMU_PAGE_SIZE;
			
			if (vaddr < mmu.krn.textEnd)
				use = 'T';
			else if (vaddr < mmu.krn.rodataEnd)
				use = 'R';
			else if (vaddr < mmu.krn.dataEnd)
				use = 'D';
			else if (vaddr < mmu.krn.stackEnd)
				use = 'S';
			else if (vaddr < mmu.krn.sharedDataEnd)
				use = 'S';
			else if (vaddr < mmu.krn.heapEnd)
				use = 'H';
			else if (vaddr < mmu.krn.ioEnd)
				use = 'I';
			else if (vaddr < owner->pt->heapEnd)
				use = 'H';
			else if (vaddr < owner->pt->stackEnd)
				use = 'S';
			else
				use = 'S';
			
			sprintf(ptr, " |%c%3u:%8s", use, i, owner->info.name);
		}
		else {
			sprintf(ptr, " | %3u:        ", i);
		}
			
		ptr += strlen(ptr);
		columns++;
		if (columns == 8) {
			strcat(ptr, "|");
			columns = 0;
			ptr = buf;
			OS_LOG("%s", buf);
		}
	}
}
