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

uint32_t cpu_getCycles32(void)
INLINEASM("\t\
rdtsc r0:r0");

//! Struct used for manually retrieving IRQs from the IRQ queue.
typedef struct IRQData
{
	u32 busid; // Device bus that caused the IRQ
	u32 reason; // Reason for the IRQ.
	u32 regs[4]; // r0..r3
} IRQData;

/*! Explicitly retrieves the next IRQ from the IRQ queue.
This can be used you prefer to always have interrupts disabled, and treat IRQs
explicitly
\param dst
	Where you will get the IRQ information
\param busFilter
	Allows retrieving IRQs only for the specific bus. To retrieve the next IRQ
	without any filtering, set this to 255
\return
	True if an IRQ was retrieved, false if there was no IRQ available
*/
bool cpu_getNextIRQ(IRQData* dst, u8 busFilter);

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

//
// Interrupt reasons for the CPU
//
#define HWCPU_INTERRUPT_ABORT 0
#define HWCPU_INTERRUPT_DIVIDEBYZERO 1
#define HWCPU_INTERRUPT_UNDEFINEINSTRUCTION 2
#define HWCPU_INTERRUPT_ILLEGALINSTRUCTION 3
#define HWCPU_INTERRUPT_SWI 4
#define HWCPU_INTERRUPT_MAX 5


#endif

