#ifndef _hwcrt0_h_
#define _hwcrt0_h_

#include "hwcrt0_stdc.h"

////////////////////////////////////////////////////////////////////////////////
//                        Some generic bit manipulation macros
////////////////////////////////////////////////////////////////////////////////

/*!
 * Makes a bit mask for a range of bits (H-L, inclusive)
 */
#define HWCRT0_MAKEMASK(H,L) (((uint32_t) -1 >> ((sizeof(uint32_t)*8-1) - (H))) & ~((1U << (L)) - 1))

/*!
 * returns val with the specified range of bits zeroed (H-L, inclusive)
 */
#define HWCRT0_ZEROBITS(val, H, L) \
	((val) & (~HWCRT0_MAKEMASK((H),(L))))

/*!
 *  Returns "val" with a range of bits (H-L, inclusive) set to "bits".
 */
#define HWCRT0_SETBITS(val, H, L, bits) \
	( HWCRT0_ZEROBITS((val),(H),(L)) | ((bits) << (L)) )
	
////////////////////////////////////////////////////////////////////////////////
//                             Architecture Related
////////////////////////////////////////////////////////////////////////////////

//
// To make it clear what function parameters are physical addresses and avoid
// bugs by simply passing e.g `void*` to some function, those parameters
// are of type `phsy_addr`, which is implemented with a struct. 
// This means that any function calling another function that takes a physical
// address needs to be explicit about casts, therefore increasing type safety.
struct phys_addr_t
{
	int dummy;
};

// Note that `phys_addr` is intentionally typedef to `phys_addr_t*` (a pointer),
// because the compiler is not smart enough to realize a struct parameter is
// actually only 1 word and can be passed in a register.
// As-in, if it was typedef to a struct, the compiler would pass those on the
// stack and generate silly memcpy instructions and such, instead of just
// putting it in a register.
typedef struct phys_addr_t* phys_addr;

/*!
 * Process info that gets embedded in the ROM image
 * This lets an OS know some basic info about the code and data
 */
typedef struct ROMInfo
{
	// .text start address and size in bytes
	uint32_t textAddr;
	uint32_t textSize;
	
	// .rodata start address and size in bytes
	uint32_t rodataAddr;
	uint32_t rodataSize;
	
	// (.data+.bss) start address and size in bytes
	uint32_t dataAddr;
	uint32_t dataSize;
	
	// (.data_shared+.bss_shared) start address and size in bytes
	uint32_t dataSharedAddr;
	uint32_t dataSharedSize;
} ROMInfo;

/*! Any executable using the library, needs to have this symbol defined */
extern ROMInfo gROMInfo;

/*!
 * Struct used to make hwf calls when only r0...r3 are needed
 */
typedef struct
{
	int regs[4]; // r0...r3
} HwfSmallData;

/*!
 * Struct used to make hwf calls when all the registers (r0...r3 and f0...f3)
 * are needed
 */
typedef struct
{
	int regs[4]; // r0...r3
	double fregs[4]; // f0...f3
} HwfFullData;

//! Maximum number of devices that can be attached to the computer
#define HWBUS_COUNT 32
#define HWBUS_MASK (HWBUS_COUNT-1)

//
// The default devices are fixed at specific bus numbers
//
#define HWBUS_CPU 0
#define HWBUS_CLK 1
#define HWBUS_SCR 2
#define HWBUS_KYB 3
#define HWBUS_NIC 4
#define HWBUS_DKC 5
#define HWBUS_CPU 0
#define HWBUS_SCR 2
#define HWBUS_NIC 4

/*!
 * Generic hwd instruction that uses r0...r3 for both input and output.
 * Since most hardware functions don't need this many inputs or outputs, you
 * should use the versions that only use the required inputs/outputs.
 */
int hw_hwfsmall(int bus, int funcNum, HwfSmallData* data);

/*!
 * Optimized hwf calls with a specific number of inputs and outputs.
 * The first number is inputs, the second outputs.
 *
 * For performance reasons, it only reads (or sets) the required data. E.g,
 * if a function only uses 1 register for output, only `data->regs[0]` will be
 * set. The rest will stay as-is.
 */
int hw_hwf_0_1(int bus, int funcNum, HwfSmallData* data);
int hw_hwf_0_2(int bus, int funcNum, HwfSmallData* data);
int hw_hwf_0_3(int bus, int funcNum, HwfSmallData* data);
int hw_hwf_0_4(int bus, int funcNum, HwfSmallData* data);
int hw_hwf_1_0(int bus, int funcNum, HwfSmallData* data);
int hw_hwf_2_0(int bus, int funcNum, HwfSmallData* data);
int hw_hwf_3_0(int bus, int funcNum, HwfSmallData* data);


/*! Executes a hwf instruction that needs r0...r3, and f0..f3 */
int hw_hwffull(int bus, int funcNum, HwfFullData* data);

// 
// hwf generic return codes
// These apply to all devices
//

//! Operation succeeded
#define HWERR_SUCCESS 0
//! There is no device present at the specified bus
#define HWERR_NODEVICE -1
//! The device doesn't support the specified function
#define HWERR_INVALIDFUNCTION -2
//! An invalid parameter was passed to the function.
#define HWERR_INVALIDPARAMETER -3
//! The function tried to access invalid memory. This happens when some memory
// address is passed to a function and using it would cause the device to access
// invalid memory.
#define HWERR_INVALIDMEMORYADDRESS -4

//
// hwf generic functions
//

/*!
 * A no-op function that only serves to clear the device's IRQ pin.
 * Also useful to detect if a device exists at the specified bus. 
 * Input:
 *		None
 * Output:
 *		None
 */
#define HWFUNC_NULL 0

/*!
 * Get the device information (Device ID, Version, and Manufacturer's ID).
 * Input:
 *		None
 * Output:
 *		r0 - Device ID 
 *		r1 - Device Version
 *		r2 - Manufacturer's ID
 */
#define HWFUNC_ID -1

/*!
 * Get the human friendly device description
 * Input:
 *		None
 * Output:
 *		r0,r1,r2,r3 - String with a friendly name
 */
#define HWFUNC_DESCRIPTION -2

/*!
 * Get the device's 128-bits UUID
 * Input:
 *		None
 * Output:
 *		r0,r1,r2,r3 - 128 bit uuid
 */
#define HWFUNC_UUID -3

////////////////////////////////////////////////////////////////////////////////
//                                MMU Related
////////////////////////////////////////////////////////////////////////////////

/*! How many bits to define a page */
#define MMU_PAGE_OFFSET_BITS 12

/*! Page size in bytes */
#define MMU_PAGE_SIZE (1<<MMU_PAGE_OFFSET_BITS)

/*! Bit mask for the page offset of an address */
#define MMU_PAGE_OFFSET_MASK (MMU_PAGE_SIZE-1)

/* ! Maximum number of PTEs in a page table. */
#define MMU_MAX_PTES 256

/*! Calculate the page of the given address */
#define MMU_ADDR_TO_PAGE(addr) ((addr) >> MMU_PAGE_OFFSET_BITS)

/*! Calculates the page offset of the given address */
#define MMU_ADDR_PAGEOFFSET(addr) ((addr) & MMU_PAGE_OFFSET_MASK))

/*! Gives the starting address of a page */
#define MMU_PAGE_TO_ADDR(page) ((page)*MMU_PAGE_SIZE)

/*! Calculates how many pages are necessary for the give memory size */
uint32_t MMU_SIZE_TO_PAGES(uint32_t size);

/*! Builds an address, given a page and page offset */
#define MMU_ADDR(page, offset) (((page)<<MMU_PAGE_OFFSET_BITS) | (offset))

/*!
 * Bit mask for a PTE's access keys (7 bits). 
 */
#define MMU_PTE_KEYS_MASK 0x7f

/*! Read access permissions bit of a PTE */
#define MMU_PTE_R (1<<11)

/*! Write access permissions bit of a PTE */
#define MMU_PTE_W (1<<10)

/*! Execute access permissions bit of a PTE */
#define MMU_PTE_X (1<<9)

/*! Accessed bit: It's set to 1 by the MMU whenever the page is accessed */
#define MMU_PTE_A (1<<8)

/*! Dirty bit: It's set to 1 by the MMU whenever a write operation happens */
#define MMU_PTE_D (1<<7)

/*! Read and Write */
#define MMU_PTE_RW (MMU_PTE_R | MMU_PTE_W)

/*! All permission bits (Read, Write, Execute) of a PTE */
#define MMU_PTE_RWX (MMU_PTE_R | MMU_PTE_W | MMU_PTE_X)

////////////////////////////////////////////////////////////////////////////////
//                                CPU Related
////////////////////////////////////////////////////////////////////////////////

/*! Number of general purpose registers */
#define CPU_NUM_GREGS 16

/*! Number of floating point registers */
#define CPU_NUM_FREGS 16

/*!
 * Number of context dependent control registers.
 * There are actually 16 control registers, but since this macro is only used
 * for creating an array of N elements to save registers, we only care about
 * those 8 context dependent control registers.
 */
#define CPU_NUM_CRREGS 8

/*! Shared data base register */
#define CPU_REG_DS 11
/*! Stack pointer register */
#define CPU_REG_SP 13
/*! Frame pointer register */
#define CPU_REG_FP 14
/*! Instruction pointer register */
#define CPU_REG_PC 15

//
// Named control registers
//

/*! flags : Flags register */
#define CPU_CRREG_FLAGS 0

/*!
 * crirqmsk: IRQ mask register. It specifies what devices can generate an
 * interrupt.
 * The architecture supports 32 connected devices, and each one gets 1 bit in
 * this register.
 */
#define CPU_CRREG_IRQMSK 1

/*!
 * crtsk: Physical address where the current context is saved when an interrupt
 * happens
 */
#define CPU_CRREG_TSK 2

/*!
 * crpt: Page table's physical address.
 */
#define CPU_CRREG_PT 3

/*! crirqs: What devices need attention. */
#define CPU_CRREG_IRQS 8

/*! crirqtsk: What context to load when an interrupt happens */
#define CPU_CRREG_IRQTSK 9

//
// Individual N,Z,C,O,S bits in the flags register
//

/*! Negative flag */
#define CPU_CRREG_FLAGS_N (1<<31)
/*! Zero flag */
#define CPU_CRREG_FLAGS_Z (1<<30)
/*! Carry flag */
#define CPU_CRREG_FLAGS_C (1<<29)
/*! Overflow flag */
#define CPU_CRREG_FLAGS_V (1<<28)
/*! Supervisor flag (Tells if the CPU is running in supervisor mode) */
#define CPU_CRREG_FLAGS_S (1<<27)

/*!
 * Mask for what bits can be set in the flags register when the CPU is in user
 * mode
 */
#define CPU_CRREG_FLAGS_USERMODE_MASK \
	(CPU_CRREG_FLAGS_N|CPU_CRREG_FLAGS_Z|CPU_CRREG_FLAGS_C|CPU_CRREG_FLAGS_V)

typedef enum CpuException {
	kCpuException_Abort,
	kCpuException_DivideByZero,
	kCpuException_UndefinedInstruction,
	kCpuException_IllegalInstruction,
	kCpuException_SWI,
	kCpuException_DebugBreak
} CpuException;

/*!
 * Given the reason value received in an interrupt handler, it gets the cpu
 * exception type.
 * It should only be used when the bus specified in reason is `0` (cpu)
 */
inline CpuException hwcpu_getExceptionType(u32 reason)
{
	return (reason >> 5) & 0xF;
}

/*!
 * User mode cpu context.
 * This is typically used with a ctxswitch instruction.
 */
typedef struct UserCpuCtx {
	uint32_t gregs[CPU_NUM_GREGS];
	double fregs[CPU_NUM_FREGS];
} UserCpuCtx;

/*!
 * Full cpu context.
 * This is typically used with a fullctxswitch instruction, which requires the
 * cpu to be in Supervisor mode
 */
typedef struct FullCpuCtx {
	uint32_t gregs[CPU_NUM_GREGS];
	double fregs[CPU_NUM_FREGS];
	uint32_t crregs[CPU_NUM_CRREGS];
} FullCpuCtx;



/*!
 * Gets the ds register (the base shared data register)
 */
static u32 hwcpu_get_ds(void)
INLINEASM("\t\
mov r0, r11");

/*!
 * Sets the ds register (the base shared data register)
 */
void hwcpu_set_ds(uint32_t val)
INLINEASM("\t\
mov r11, r0");

/*! Get the value of the sp register */
void* hwcpu_get_sp(void)
INLINEASM("\t\
mov r0, sp");

/*! Get the value of the fp register */
void* hwcpu_get_fp(void)
INLINEASM("\t\
mov r0, fp");

/*! Returns the value of the crflags register */
uint32_t hwcpu_get_crflags(void)
INLINEASM("\t\
getcr r0, crflags");

/*!
 * Sets the value of the crflags register.
 * Note that some bits cannot be changed while in user mode. If in user mode,
 * the instruction will only set the bits that are alllowed to be set.
 */
void hwcpu_set_crflags(uint32_t val)
INLINEASM("\t\
setcr crflags, r0");

/*! Sets the crirqmsk register */
void hwcpu_set_crirqmsk(uint32_t crirqmsk)
INLINEASM("\t\
setcr crirqmsk, r0");

/*! Sets the crirqtsk register */
void hwcpu_set_crirqtsk(void* crirqtsk)
INLINEASM("\t\
setcr crirqtsk, r0");

/*! Sets the crtsk register */
void hwcpu_set_crtsk(void* crtsk)
INLINEASM("\t\
setcr crtsk, r0");

/*! Returns the value of the crpt register */
uint32_t hwcpu_get_crpt(void)
INLINEASM("\t\
getcr r0, crpt");

/*! Sets the crpt register */
void hwcpu_set_crpt(uint32_t val)
INLINEASM("\t\
setcr crpt, r0");

/*! Emits a nop instruction */
void hwcpu_nop(void)
INLINEASM("\t\
nop");

/*! Emits a dbgbrk instruction */
void hwcpu_dbgbrk(void)
INLINEASM("\t\
dbgbrk 0");

/*! Emits a hlt instruction */
void hwcpu_hlt(void)
INLINEASM("\t\
hlt");

/*!
 * Clears a device's IRQ pin
 *
 * This can also be used to check if a device exists at the specified bus, if
 * you don't care that it will clear the IRQ pin for that device.
 *
 * \param bus Device to use
 * \return HWERR_SUCCESS or HWERR_NODEVICE if no device found at the specified
 *	bus
 *
 */
int hw_touch(int bus)
INLINEASM("\
\thwf r0\n\
\tmov r0, ip\n\
");

/*! Clears all IRQs
 */
void hw_touchAll(void);

/*!
 * Emits a fullctxswitch instruction.
 *
 * A ctxswitch or fullctxswitch instruction does a CPU context switch, which
 * means this function might not return. An OS can use this to switch between
 * processes or threads.
 *
 * \param new
 * Context to load
 *
 * \param curr
 * Where to save the current context
 *
 * \param ret (out parameter)
 * When the current context is resumed (and thus this function returns), this
 * will contain the values of registers r0, r1, r2 and r3.
 *
 * \return
 * When the current context is resumed, it returns r0. This allows contexts to
 * pass values to each other, by setting the r0 register of the context to
 * resume.
 */
uint32_t hwcpu_fullctxswitch(const FullCpuCtx* new, FullCpuCtx* curr,
							 uint32_t* ret) INLINEASM("\
\tmov ip, r2\n\
\tfullctxswitch [r0], [r1]\n\
\tstr [ip + 0], r0\n\
\tstr [ip + 4], r1\n\
\tstr [ip + 8], r2\n\
\tstr [ip + 12], r3\n\
");

/*!
 * Emits a ctxswitch instruction.
 *
 * \param new
 * Context to load
 *
 * \param curr
 * Where to save the current context
 */
uint32_t hwcpu_ctxswitch(const UserCpuCtx* new, UserCpuCtx* curr)
INLINEASM("\t\
ctxswitch [r0], [r1]");

/*! Returns the lowest 32 bits of the cpu tick counter */
uint32_t hwcpu_getCycles(void)
INLINEASM("\t\
rdtsc r0:r0");

///

/*! Gets the ammount of ram present in the system (in bytes) */
uint32_t hwcpu_getTotalRam(void);

/*!
 * Sets the MMU keys for current context.
 * This replaces all the existing keys with the ones specified.
 */
void hwcpu_setMMUKeys(uint32_t keys)
INLINEASM("\
\tgetcr r1, crflags\n\
\tand r1, r1, " STRINGIFY(~MMU_PTE_KEYS_MASK) "\n\
\tor r1, r1, r0\n\
\tsetcr crflags, r1\
");

/*!
 * Adds the specified mmu keys to the current context
 */
void hwcpu_addMMUKeys(uint32_t keys)
INLINEASM("\
\tgetcr r1, crflags\n\
\tor r1, r1, r0\n\
\tsetcr crflags, r1\
");

/*!
 * Removes the specified mmu keys from the current context
 */
void hwcpu_removeMMUKeys(uint32_t keys)
INLINEASM("\
\tgetcr r1, crflags\n\
\tnot r0, r0\n\
\tand r1, r1, r0\n\
\tsetcr crflags, r1\
");


////////////////////////////////////////////////////////////////////////////////
//                                CLK Related
////////////////////////////////////////////////////////////////////////////////

#define HWCLK_FUNC_GET_TIMESINCEBOOT 1

/*! Gets the number of seconds the system has been running since boot */
double hwclk_getSecsSinceBoot(void)
INLINEASM("\
\tmov ip, (" STRINGIFY(HWCLK_FUNC_GET_TIMESINCEBOOT) " << 8) | " STRINGIFY(HWBUS_CLK) "\n\
\thwf ip\n\
");

/*!
 * Spins the CPU for the specified duration
 * \param ms Duration in milliseconds
 * \note This function busy-waits. It does not put the cpu in idle.
 */
void hwclk_spinMs(int ms);

////////////////////////////////////////////////////////////////////////////////
//                                SCR Related
////////////////////////////////////////////////////////////////////////////////

//

////////////////////////////////////////////////////////////////////////////////
//                                NIC Related
////////////////////////////////////////////////////////////////////////////////

/*!
 * Sends a strings to the debug output
 * \param str Physical address of the string to send.
 */
int hwnic_sendDebug(phys_addr str);

#endif
