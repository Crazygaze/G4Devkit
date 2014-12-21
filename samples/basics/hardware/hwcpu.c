#include "hwcpu.h"
#include <assert.h>

#define HWCPUFUNC_GET_RAM_AMOUNT 0
#define HWCPUFUNC_GET_IRQ_QUEUE_SIZE 1
#define HWCPUFUNC_SET_MMU_TABLE 2

u32 cpu_getRamAmount(void)
{
	HwiData data;
	int res = hwiCall(HWBUS_CPU, HWCPUFUNC_GET_RAM_AMOUNT, &data);
	always_assert(res==HWIERR_SUCCESS);
	return data.regs[0];
}

int cpu_getIRQQueueSize(void)
{
	HwiData data;
	int res = hwiCall(HWBUS_CPU, HWCPUFUNC_GET_IRQ_QUEUE_SIZE, &data);
	always_assert(res==HWIERR_SUCCESS);
	return data.regs[0];
}

void cpu_setMMUTableAddress(void* tbl, int size)
{
	HwiData data;
	data.regs[0] = (u32)tbl;
	u32 ram = cpu_getRamAmount();
	u32 numPages = ram / MMU_PAGE_SIZE;
	always_assert(numPages==(size/4));
	int res = hwiCall(HWBUS_CPU, HWCPUFUNC_SET_MMU_TABLE, &data);
}
