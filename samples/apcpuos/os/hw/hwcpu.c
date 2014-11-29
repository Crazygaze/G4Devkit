/*******************************************************************************
* CPU driver
*******************************************************************************/

#include "hwcpu.h"
#include "boot/boot.h"
#include "kernel/kerneldebug.h"
#include "kernel/kernel.h"
#include "hw/hwscreen.h"
#include "multitasking/process.h"
#include "syscalls/syscalls.h"

typedef struct hw_cpu_Drv {
	hw_Drv base;
	// Extra fields required by the device goes below...
} hw_cpu_Drv;

typedef enum CpuInterrupt {
	kCpuInterrupt_Abort,
	kCpuInterrupt_DivideByZero,
	kCpuInterrupt_UndefinedInstruction,
	kCpuInterrupt_IllegalInstruction,
	kCpuInterrupt_SWI,
	kCpuException_MAX
} CpuException;

static const char * const cpuIntrStr[kCpuException_MAX] = {
	"Access violation",	
	"Divide by zero",
	"Undefined instruction",
	"Illegal instruction"
	"SWI"
};

static hw_cpu_Drv driver;
static void hw_cpu_irqHandler(uint32_t reason, u32 data1, u32 data2);

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

const char* hw_cpu_getIntrReasonMsg(uint32_t reason)
{
	return (reason<kCpuException_MAX) ? cpuIntrStr[reason] : "unknown";
}

static void hw_cpu_irqHandler(uint32_t reason, u32 data0, u32 data1)
{
	switch(reason)
	{
		case 0: // Abort
		case 1: // Divide by zero
		case 2: // Undefined instruction
		case 3: // Illegal instruction
			// TODO : Instead of doing a kernel panic, close the offender app
			krn_panic(
				"TASK %d:%s, REASON '%s' DATA 0x%X,0x%X",
				krn.interruptedTcb->pcb->info.pid,
				krn.interruptedTcb->pcb->info.name,
				cpuIntrStr[reason], data0, data1);
		break;
		
		case 4: // SWI (System Call)
		{
			int* regs = (int*)krn.interruptedTcb->cpuctx;
			uint32_t id = regs[SYSCALL_ID_REGISTER];
			bool ok;
			if (id<kSysCall_Max) {
				ok = krn_syscalls[id]();
			} else {
				// TODO : Instead of doing a kernel panic, close the
				// offender app
				KERNEL_DEBUG(
					"TASK %d:%s, Invalid SWI ID '%u'",
					krn.interruptedTcb->pcb->info.pid,
					krn.interruptedTcb->pcb->info.name, id);
				ok = false;
			}
			
			if (!ok) {
				KERNEL_DEBUG(
					"TASK %d:%s, killed calling SWI ID '%u'",
					krn.interruptedTcb->pcb->info.pid,
					krn.interruptedTcb->pcb->info.name, id);
				prc_delete(krn.interruptedTcb->pcb);
				
				// No need to set the next thread, as prc_delete will
				// eventually trigger that already
				//krn.nextTcb = krn.idlethread;
			} else {
				krn_countSwiTime = true;
			}
		}		
		break;
		
		default:
			krn_panic("UNKNOWN CPU INTERRUPT TYPE: %d", reason);
	}
}
