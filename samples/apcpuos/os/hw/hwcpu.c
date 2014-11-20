/*******************************************************************************
* CPU driver
*******************************************************************************/

#include "hwcpu.h"
#include "boot/boot.h"
#include "kernel/kerneldebug.h"
#include "kernel/kernel.h"
#include "hw/hwscreen.h"
#include "multitasking/process.h"

typedef struct hw_cpu_Drv {
	hw_Drv base;
	// Extra fields required by the device goes below...
} hw_cpu_Drv;

typedef enum CpuException {
	// High priority
	kCpuException_HighPriority_Reset,
	kCpuException_HighPriority_Abort,
	// Low priority
	kCpuException_LowPriority_SWI,
	kCpuException_LowPriority_DivideByZero,
	kCpuException_LowPriority_UndefinedInstruction,
	kCpuException_LowPriority_IllegalInstruction,
	kCpuException_MAX
} CpuException;

static const char * const cpuIntrStr[kCpuException_MAX] = {
	"Reset",
	"Access violation",	
	"Divide by zero",
	"Undefined instruction",
	"Illegal instruction"
	"SWI"
};

static hw_cpu_Drv driver;
static void hw_cpu_irqHandler(uint16_t reason, u32 data1, u32 data2);

hw_Drv* hw_cpu_ctor(hw_BusId busid)
{
	krn_bootLog("\n");
	krn_bootLog("  Detected RAM: %d kbytes\n", ramAmount/1024);	
	krn_bootLog("  Initializing MMU...");
	
	driver.base.irqHandler = hw_cpu_irqHandler;
	return (hw_Drv*)&driver;
}

void hw_cpu_dtor(hw_Drv* drv)
{
}

const char* hw_cpu_getIntrReasonMsg(uint16_t reason)
{
	return (reason<kCpuException_MAX) ? cpuIntrStr[reason] : "unknown";
}

static void hw_cpu_irqHandler(uint16_t reason, u32 data1, u32 data2)
{
	krn_panic(
		"TASK %d:%s, REASON '%s' DATA 0x%X  DATA 0x%X",
		krn.interruptedTcb->pcb->info.pid, krn.interruptedTcb->pcb->info.name,
		hw_cpu_getIntrReasonMsg(reason) , data1, data2);
}
