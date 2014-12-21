#ifndef _hwcpu_h_
#define _hwcpu_h_

#include "hwcommon.h"

void cpu_halt(void);
void cpu_dbgbrk(void);
u32 cpu_getRamAmount(void);
int cpu_getIRQQueueSize(void);
void cpu_enableIRQ(void);
void cpu_disableIRQ(void);
void cpu_setMMUTableAddress(void* tbl, int size);

#define MMU_PAGE_SIZE (1024*1)
#define ADDR_TO_PAGE(addr) ((uint32_t)(addr) / MMU_PAGE_SIZE)
#define PAGE_TO_ADDR(page) ((uint8_t*)((uint32_t)(page) * MMU_PAGE_SIZE))
#define SIZE_TO_PAGES(size) \
	(((size) / MMU_PAGE_SIZE) + (((size)%MMU_PAGE_SIZE) ? 1 : 0))
#define PAGES_TO_SIZE(pages) \
	((pages)*MMU_PAGE_SIZE)


#define CPU_NUM_GREGS 16
#define CPU_NUM_FREGS 16

#define CPU_REG_DS 11
#define CPU_REG_SP 13
#define CPU_REG_PC 15

#define INTRCTX_ADDR 0x8

#define CPU_FLAGSREG_SUPERVISOR (1<<26)
#define CPU_FLAGSREG_IRQDISABLED (1<<27)

#endif

