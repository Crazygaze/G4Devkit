#include "hwcommon.h"
#include "hwcpu.h"
#include "hwclock.h"
#include "hwkeyboard.h"
#include <string.h>
#include <hwscreen.h>
#include "common.h"
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include "hardwaretest_common.h"
#include "hardwaretest_cpu.h"
#include "hardwaretest_clock.h"

static Ctx appCtx;

// The assembly interrupt handler sets this whenever an interrupt happens
Ctx* interruptedCtx;
// The assembly interrupt handler sets this whenever an IRQ interrupt happens
u32 interruptBus;
u32 interruptReason;

#define NUM_DRIVERS 2

// Put all the the drivers together
DeviceTest deviceTests[NUM_DRIVERS];

Ctx* interruptHandler(u32 data0, u32 data1, u32 data2, u32 data3)
{
	always_assert(interruptBus<NUM_DRIVERS);
	Driver* driver = deviceTests[interruptBus].driver;
	always_assert(interruptReason < driver->numHanders);	
	// We just forward the handling to the specific driver
	driver->handlers[interruptReason](data0, data1, data2, data3);	
	return &appCtx;
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

void hardwareTests(void)
{
	scr_init();

	// Initialize drivers 
	hardwareTest_cpu_init(&deviceTests[HWBUS_CPU]);
	hardwareTest_clock_init(&deviceTests[HWBUS_CLK]);
	
	for(int i=0; i<NUM_DRIVERS; i++) {
		deviceTests[i].testFunc();
		doPause();
	}
	
	// We should never return from this function
	loopForever();
}

