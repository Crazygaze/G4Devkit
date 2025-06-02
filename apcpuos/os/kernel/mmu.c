#include "mmu.h"
#include "kernel.h"
#include "hwcrt0.h"
#include "utils/bitops.h"
#include <stdlib.h>

/*!
 * Physical Page information.
 * This tracks the state a single physical page.
 */
typedef struct PPInfo
{
	PageTable* owner;
} PPInfo;

typedef struct MMU
{
	// How much ram the system has in pages
	u32 totalPages;
	
	u32* table;
	u32 pteCount;
	
	// Page fault handler context
	FullCpuCtx pfctx;
	// Page fault handler's stack
	u8 pfstack[PFHANDLER_STACK_SIZE];
	
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

	// Point the IO allocation cursor to the right location
	mmu.currIOEnd = mmu.krn.ioBegin;
	
	return mmu.krn.stackEnd;
}

void mmu_ppBeginTransaction(PageTable* pt)
{
	krnassert(mmu.ptTransaction == NULL);
	mmu.ptTransaction = pt;
}

static inline PageTable* createTransactionTag(void)
{
	// Create the dummy pointer used to tag what pages are in the transation
	uint8_t* dummy = NULL;
	PageTable* tag = (PageTable*)(dummy - 1);
	return tag;
}

/*!
 * Allocates one physical page for the specified page table.
 * \returns the page number (>0) on success, or 0 on failure
 */
u32 mmu_ppAlloc(void)
{
	krnassert(mmu.ptTransaction);
	
	PageTable* tag = createTransactionTag();
	u32 firstPage = mmu_krnPTECount(&mmu.krn);
	
	for (int i = firstPage; i < mmu.totalPages; i++) {
		if (mmu.ppinfo[i].owner == NULL) {
			mmu.ppinfo[i].owner = tag;
			return i;
		}
	}
}
void mmu_ppFinishTransaction(bool commit)
{
	krnassert(mmu.ptTransaction);
	
	PageTable* tag = createTransactionTag();
	u32 firstPage = mmu_krnPTECount(&mmu.krn);
	
	for (int i = firstPage; i < mmu.totalPages; i++) {
		if (mmu.ppinfo[i].owner == tag) {
			mmu.ppinfo[i].owner = commit ? mmu.ptTransaction : NULL;
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
			count++;
		}
	}
	
	return count;
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
void mmu_setKrnRange(u32* pt, u32 begin, u32 end, u32 firstpage, u32 bits)
{
	// A range can be empty
	if (end == begin)
		return;
	
	u32 p = firstpage;
	pt++; // Skip the PT header
	for (u32 i = MMU_ADDR_TO_PAGE(begin); i <= MMU_ADDR_TO_PAGE(end - 1); i++) {
		pt[i] = (p << MMU_PAGE_OFFSET_BITS) | bits;
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
		u32 ppage = 0;
		if (allocPages) {
			ppage = mmu_ppAlloc();
			if (ppage == 0)
				return false;
		}
		
		data[i] = (ppage << MMU_PAGE_OFFSET_BITS) | rwxbits | keys;
	}
}

/*!
 * Setups the kernel space range of the given page table
 */
void mmu_setupKernelSpace(PageTable* pt)
{
	const PTKrnLayout* krn = &mmu.krn;
	krnassert(krn);
	
	u32* data= pt->data;
	
	mmu_setKrnRange(data, krn->textBegin, krn->textEnd,
		MMU_ADDR_TO_PAGE(krn->textBegin),
		MMU_PTE_X | MMU_PTE_KEY_USR | MMU_PTE_KEY_KRN);
	mmu_setKrnRange(data, krn->rodataBegin, krn->rodataEnd,
		MMU_ADDR_TO_PAGE(krn->rodataBegin),
		MMU_PTE_R | MMU_PTE_KEY_USR | MMU_PTE_KEY_KRN);
	mmu_setKrnRange(data, krn->dataBegin, krn->dataEnd,
		MMU_ADDR_TO_PAGE(krn->dataBegin),
		MMU_PTE_RW | MMU_PTE_KEY_KRN);
	// The original shared data is set with keys `0`, and we only set the keys
	// when we want to access it duplicate it for another process
	mmu_setKrnRange(data, krn->oriSharedDataBegin, krn->oriSharedDataEnd,
		MMU_ADDR_TO_PAGE(krn->oriSharedDataBegin), MMU_PTE_R | 0);
	mmu_setKrnRange(data, krn->stackBegin, krn->stackEnd,
		MMU_ADDR_TO_PAGE(krn->stackBegin),
		MMU_PTE_RW | MMU_PTE_KEY_KRN);
	mmu_setKrnRange(data, krn->sharedDataBegin, krn->sharedDataEnd,
		MMU_ADDR_TO_PAGE(krn->sharedDataBegin),
		MMU_PTE_RW | MMU_PTE_KEY_KRN);
	mmu_setKrnRange(data, krn->heapBegin, krn->heapEnd,
		MMU_ADDR_TO_PAGE(krn->heapBegin),
		MMU_PTE_RW | MMU_PTE_KEY_KRN);
	mmu_setKrnRange(data, krn->ioBegin, krn->ioEnd,
		MMU_ADDR_TO_PAGE(krn->ioBegin),
		MMU_PTE_RW | MMU_PTE_KEY_KRN);
}

static PageTable* mmu_createKrnPT(void)
{
	const PTKrnLayout* krn = &mmu.krn;
	krnassert(krn);
	
	u32 npages = mmu_krnPTECount(&mmu.krn);
	
	// Allocate all in one chunk.
	// The extra word is for the header
	PageTable* pt = calloc(sizeof(PageTable) + sizeof(u32) + (npages * sizeof(u32)));
	if (!pt) {
		OS_ERR("Out of memory creating page table (%u pages)", npages);
		goto out1;
	}
	
	pt->data = (u32*)&pt[1];
	pt->data[0] = MMU_PAGE_TO_ADDR(npages);
	mmu_setupKernelSpace(pt);
	
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
		
	stdc_init((void*)mmu.krn.heapBegin, kernelHeapSize);
	
	// Since the kernel space is fixed, we don't need to track those pages, so 
	// the tracking array doesn't need to be as big.
	u32 krnNPages = mmu_krnPTECount(&mmu.krn);
	mmu.ppinfo = calloc(sizeof(PPInfo) * (mmu.totalPages - krnNPages));
	
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
PageTable* mmu_createUsrPT(u32 stackSize, u32 heapSize)
{
	krnassert(stackSize);
	
	PTUsrLayout usr = { 0 };
	
	u32 npages = mmu_calcPTLayout(stackSize, heapSize, &usr);
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
	pt->usrSP = usr.stackEnd;
	
	//
	// Setup kernel space
	//
	mmu_setupKernelSpace(pt);
		
	//
	// Setup user space
	//
	mmu_ppBeginTransaction(pt);
	
	// Heap
	// The heap doesn't get any allocated pages until the process uses the heap
	krnverify(mmu_setUsrRange(pt, usr.heapBegin, usr.heapEnd, false, MMU_PTE_RW));
	
	// Stack
	// For the stack we allocate 1 page to begin with, and then as the stack
	// grows, we allocate more.
	//
	// Setup the stack range as not having have physical pages
	krnverify(mmu_setUsrRange(pt, usr.stackBegin, usr.sharedDataEnd, false,
		MMU_PTE_RW));
		
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
