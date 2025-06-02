#include "hwmmu.h"
#include "hwcommon.h"
#include "hwscreen.h"
#include "hwclock.h"
#include "hwcpu.h"
#include <stdlib.h>
#include <string.h>

int line = 1;
u32* mmuTable;
u32 mmuTableEntries;

void doWork(void)
{
	for (int x = 0; x < (SCR_XRES - 30); x++)
		for (int y = 0; y < SCR_YRES; y++) {
			scr_printfAtXY(x, y, " Testing %06d, %02d:%02d %3.3f ", x * y, x, y,
						   (float)x / (y + 1));
		}
}

// Calculate the MMU table size in bytes
// Called from the assembly boot code
u32 calcMMUTableSize(void)
{
	u32 ramAmount = hwcpu_getRamAmount();
	mmuTableEntries = ramAmount / MMU_PAGE_SIZE;
	return mmuTableEntries * sizeof(u32);
}

void initMMUTable(void)
{
	u32 key = 1;
	hwcpu_setMMUAccessKey(key);
	for (u32 i = 0; i < mmuTableEntries; i++) {
		mmuTable[i] = (i * MMU_PAGE_SIZE) | MMU_TABLE_RWX_BITS | key;
	}
}

void appMain(void)
{
	// initMMUTable();
	// cpu_setMMUTableAddress(mmuTable, mmuTableEntries);

	int x, y;
	scr_init();
	float f1;
	float f2;

	double startTime = clk_getRunningTimeSec();

	while (1) {
		doWork();
	}
#if 0
	double endTime = clk_getRunningTimeSeconds();
	uint32_t cycles = cpu_getCycles32();
	
	scr_clear();
	scr_printfAtXY(0,0, "Duration: %3.4f", endTime-startTime);
	scr_printfAtXY(0,1, "Cycles: %u", cycles);
	scr_printfAtXY(0,2, "Mhgz  : %4.4f", ((double)cycles/(endTime-startTime))/(1000*1000));

#endif

	// loopForever();
}
