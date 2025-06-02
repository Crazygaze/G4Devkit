#include "hwcrt0.h"

////////////////////////////////////////////////////////////////////////////////
//                                MMU Related
////////////////////////////////////////////////////////////////////////////////

// Making this one a function instead of a macro because 'size' is used twice
// and doing that in a macro is asking for trouble.
uint32_t MMU_SIZE_TO_PAGES(uint32_t size)
{
	return (size / MMU_PAGE_SIZE) + ((size % MMU_PAGE_SIZE) ? 1 : 0);
}

////////////////////////////////////////////////////////////////////////////////
//                                CPU Related
////////////////////////////////////////////////////////////////////////////////

#define HWCPU_FUNC_GET_RAMAMOUNT 1

void hw_touchAll()
{
	// Touch all devices, to clear the pending IRQs
	for (int bus = 0; bus < HWBUS_COUNT; bus++) {
		hw_touch(bus);
	}
}

uint32_t hwcpu_getTotalRam(void)
{
	HwfSmallData data;
	hw_hwf_0_1(HWBUS_CPU, HWCPU_FUNC_GET_RAMAMOUNT, &data);
	return data.regs[0];
}

////////////////////////////////////////////////////////////////////////////////
//                                CLK Related
////////////////////////////////////////////////////////////////////////////////

void hwclk_spinMs(int ms)
{
	double end = hwclk_getSecsSinceBoot() + (double)ms / 1000.0f;
	// Loop until enough time has passed
	while (end > hwclk_getSecsSinceBoot()) {
	}
}

////////////////////////////////////////////////////////////////////////////////
//                                NIC Related
////////////////////////////////////////////////////////////////////////////////

#define HWNICFUNC_SND 2

int hwnic_sendDebug(const char* str)
{
	HwfSmallData hwf;
	hwf.regs[0] = 0; // Destination id (0 is the debug destination)
	hwf.regs[1] = (int)str;
	hwf.regs[2] = strlen(str) + 1;
	return hw_hwf_3_0(HWBUS_NIC, HWNICFUNC_SND, &hwf);
}
