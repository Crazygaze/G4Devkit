#include "hwcommon.h"
#include "hwnic.h"
#include "hwcpu.h"
#include <string.h>
#include "hwscreen.h"
#include "hwkeyboard.h"
#include "common.h"
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

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
	return &appCtx;
}

void hardwareTestsNic(void);

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
	appCtx.gregs[CPU_REG_PC] = (u32) &hardwareTestsNic;
	appCtx.flags = CPU_FLAGSREG_SUPERVISOR;
	
	// We return this to the assembly code, which specifies what execution
	// context to switch to
	return &appCtx;
}

void hardwareTestsNic(void)
{
	scr_init();
	
	scr_printf("This is the NIC unit test companion application.\n");
	scr_printf("It is used to test communication between computers.\n");
	
	char rcv[128];
	while(1)
	{
		int srcID;
		int rcvSize = nic_receive(rcv, sizeof(rcv), &srcID);
		if(rcvSize==0)
			continue;
			
		scr_printf("Received from %s : %s\n", nic_getIDStr(srcID), rcv);

		if (strcmp(rcv, "connect")==0)
		{
			const char* str = "ok";
			scr_printf("Sending to %s: %s\n", nic_getIDStr(srcID), str); 
			nic_sendString(srcID, str);
		}
		else if (rcv[0]=='N' && rcv[1]==':')
		{
			int num = strtol(&rcv[2], NULL, 10);
			char buf[32];
			sprintf(buf, "N:%d", -num);
			nic_sendString(srcID, buf);			
		}		
	}	
	
	// We should never return from this function
	loopForever();
}