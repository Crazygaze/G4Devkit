#include "hardwaretest_nic.h"
#include "hwnic.h"
#include "common.h"
#include <assert.h>
#include <string.h>

#define HWNIC_INTERRUPT_MAX 1
typedef struct NicDriver
{
	Driver base;
	u32 irqCount;
} NicDriver;

static NicDriver nicDriver;
static char rcvMsg[1024];

// Interrupt handler
static void nic_handleIRQ(u32 data0, u32 data1, u32 data2, u32 data3)
{
	nicDriver.irqCount++;
}

InterruptHandler nicHandlers[HWNIC_INTERRUPT_MAX] =
{
	&nic_handleIRQ
};

static void hardwareTest_nic(void);
void hardwareTest_nic_init(DeviceTest* data)
{
	nicDriver.base.handlers = nicHandlers;
	nicDriver.base.numHanders = 
		sizeof(nicHandlers)/sizeof(InterruptHandler);
	data->driver = &nicDriver.base;
	data->testFunc = &hardwareTest_nic;
}

static void hardwareTest_nic(void)
{
	// TODO : Check what ID to send to once I fix the VM implementation
	const int peerID = 0;
	
	scr_printf("NIC (Network Interface Card) Tests\n");
	scr_printf("    Connecting with peer (ID %d)...");
	
	// Send the connect message, and wait for the reply
	while(1) {
		nic_sendString(peerID, "connect");
		pause(1000);
	}
}
