#include "hwcommon.h"
#include "hwcpu.h"
#include "hwclock.h"
#include "hwkeyboard.h"
#include <string.h>
#include <hwscreen.h>
#include "common.h"
#include <stdarg.h>
#include <stdio.h>

static Ctx appCtx;

void doPause(void)
{
	scr_printf("Press any key to continue...\n");
	kyb_pause();
}

void interruptHandler(u32 d0, u32 d1, u32 d2, u32 d3)
{
	
}

void hardwareTests(void);

/*
Initializes the application execution context
*/
Ctx* setupAppCtx(void)
{
	// We use this small memory block as a stack for the application context
	#define APPSTACKSIZE 1024*10
	static char appStack[APPSTACKSIZE];
	memset(&appCtx, 0, sizeof(appCtx));
	appCtx.gregs[CPU_REG_SP] = (u32) &appStack[APPSTACKSIZE];
	appCtx.gregs[CPU_REG_PC] = (u32) &hardwareTests;
	appCtx.flags = CPU_FLAGSREG_SUPERVISOR;
	
	// We return this to the assembly code, which specifies what execution
	// context to switch to
	return &appCtx;
}


void hardwareTest_cpu(void)
{
	scr_printf("CPU Tests\n");
	cpu_enableIRQ();
	cpu_disableIRQ();
	cpu_enableIRQ();
	u32 ram = cpu_getRamAmount();
	scr_printf("	RAM = %d bytes (%d Kbytes)\n", ram, ram/1024);
	scr_printf("	IRQ Queue Size = %d\n", cpu_getIRQQueueSize());
}

void hardwareTests(void)
{
	scr_init();
	
	hardwareTest_cpu();
	doPause();
	
	// We should never return from this function
	loopForever();
}


