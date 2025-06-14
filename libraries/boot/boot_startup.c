
#include <stdc_init.h>
#include "boot_startup.h"

int main(void);

uint32_t initialSP;

// +1 because of the page table header
uint32_t pageTable[1+MMU_MAX_PTES];

void boot_setupMMU(uint32_t totalRam)
{
	uint32_t numPages = MMU_SIZE_TO_PAGES(totalRam);

	if (numPages > MMU_MAX_PTES) {
		LOG_LOG("WARNING: MMU_ENABLED is set, but total ram is too high.");
		return;
	}

	uint32_t mmuKeys = 1;

	LOG_LOG("Setting up MMU with %u pages", numPages);
	// Setup the header
	pageTable[0] = MMU_ADDR(numPages, 0);
	//Setup the PTEs
	for (uint32_t p = 0; p < numPages; p++) {
		pageTable[p+1] =
			(p << MMU_PAGE_OFFSET_BITS) | // Virtual page to Physical page
			MMU_PTE_RWX                 | // Enable all permissions (no restrictions)
			mmuKeys;                      // Set keys
	}

	hwcpu_setMMUKeys(mmuKeys);
	hwcpu_set_crpt((u32)&pageTable[0]);
}

void boot_startup(uint32_t stackTop)
{
	// First thing we need to do once enter the FIRST C function is to setup
	// the shared data base register, otherwise if we end up accessing any 
	// global data put into `.data_shared` or `.bss_shared`, it will use the
	// wrong address, since the `ds` register has a value of `0` at boot.
	hwcpu_set_ds(gROMInfo.dataSharedAddr);
	
	stdc_setLogFunc((LibCDebugLogFunc)hwnic_sendDebug);

	uint32_t imageSize = gROMInfo.dataSharedAddr + gROMInfo.dataSharedSize;
	uint32_t imagePages = MMU_SIZE_TO_PAGES(imageSize);

	LOG_LOG("ROM Info:");
	LOG_LOG("  .text                   : Addr=%8u(%08xh), Size=%8u",
			gROMInfo.textAddr, gROMInfo.textAddr, gROMInfo.textSize);
	if (gROMInfo.textAddr != 0) {
		LOG_LOG("  WARNING: .text address other than 0 is not tested");
	}
	LOG_LOG("  .rodata                 : Addr=%8u(%08xh), Size=%8u",
			gROMInfo.rodataAddr, gROMInfo.rodataAddr, gROMInfo.rodataSize);
	LOG_LOG("  .data+.bss              : Addr=%8u(%08xh), Size=%8u",
			gROMInfo.dataAddr, gROMInfo.dataAddr, gROMInfo.dataSize);
	LOG_LOG("  .data_shared+.bss_shared: Addr=%8u(%08xh), Size=%8u",
			gROMInfo.dataSharedAddr, gROMInfo.dataSharedAddr,
			gROMInfo.dataSharedSize);
	LOG_LOG("  total size: %u bytes, %u pages", imageSize, imagePages);

	uint32_t totalRam = hwcpu_getTotalRam();
	uint32_t deviceReserved = totalRam - stackTop;
	LOG_LOG("Top memory used for devices = %u bytes", deviceReserved);
	LOG_LOG("SP=%08xh", stackTop);

	uint32_t freeMem = stackTop - imageSize;
	uint32_t stackSize = BOOT_STACK_SIZE;
	uint32_t heapSize = freeMem - stackSize;
	LOG_LOG("Free memory:");
	LOG_LOG("  Heap : %8u bytes", heapSize);
	LOG_LOG("  Stack: %8u bytes", stackSize);

	if (MMU_ENABLED) {
		boot_setupMMU(totalRam);
	}

	initialSP = stackTop;
	stdc_init((void*)imageSize, heapSize, NULL);

	main();
}
