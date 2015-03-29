#include "hwcommon.h"
#include "hwnic.h"
#include "hwcpu.h"
#include <string.h>
#include "hwscreen.h"
#include "common.h"
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>

static Ctx appCtx;

// The assembly interrupt handler sets this whenever an interrupt happens
Ctx* interruptedCtx;
// The assembly interrupt handler sets this whenever an IRQ interrupt happens
u32 interruptBus;
u32 interruptReason;

Ctx* interruptHandler(u32 data0, u32 data1, u32 data2, u32 data3)
{
	// In this sample, we should be only getting IRQs for the network card
	always_assert(interruptBus==HWBUS_NIC);
	always_assert(interruptReason==HWNIC_INTERRUPT_GENERIC);
	
	char buf[1024];
	int srcID;
	int receivedSize = nic_receive(buf, sizeof(buf), &srcID);
	always_assert(receivedSize<=sizeof(buf));
	
	scr_printf("RCV:%d: %s", srcID, buf);
	if (strcmp(buf, "connect")==0)
		nic_sendString(srcID,"ok");
	else
		nic_sendString(srcID, buf);

	return &appCtx;
}

void nicPeerMain(void);

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
	appCtx.gregs[CPU_REG_PC] = (u32) &nicPeerMain;
	appCtx.flags = CPU_FLAGSREG_SUPERVISOR;
	
	// We return this to the assembly code, which specifies what execution
	// context to switch to
	return &appCtx;
}

void nicPeerMain(void)
{
	scr_init();

	scr_printf("This is the NicPeer sample\nAnother line");
	
	// We should never return from this function
	loopForever();
}

