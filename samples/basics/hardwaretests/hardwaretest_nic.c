#include "hardwaretest_nic.h"
#include "hwnic.h"
#include "hwclock.h"
#include "hwkeyboard.h"
#include "common.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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

int tryConnect(int* startServer, int* startID)
{
	// Loop through a couple of addresses trying to connect to a peer
	for(int server=*startServer; server<10; server++)
	{
		for(int id=*startID; id<5; id++)
		{
			int testID = server<<24 | id;
			scr_printf("Trying to connect with peer %s...",
				nic_getIDStr(testID));
			nic_sendString(testID, "connect");
			double endTime = clk_getRunningTimeSeconds() + 0.25; 
			char rcv[32];
			while(clk_getRunningTimeSeconds()<endTime)
			{
				int srcID=0;
				int rcvSize = nic_receive(rcv, sizeof(rcv), &srcID);
				// If no data received, or we received data but not from the
				// expected addressm then it's not a valid peer address
				if (rcvSize==0 || srcID!=testID)
				{
					continue;
				}
				
				if (strcmp(rcv,"ok")==0)
				{
					scr_printf("SUCCESS\n");
					*startServer = server;
					*startID = id+1;
					return testID;
				}
			}
			scr_printf("FAILED\n");
		}
		*startID = 0;
	}
	
	scr_printf("Failed to find a peer.\n");
	return 0;
}

#define NUM_PEERS 2
static int peers[NUM_PEERS];

int getPeerIndex(int id)
{
	for(int i=0; i<NUM_PEERS; i++) {
		if (srcID==peers[i])
			return i;
	}
	return -1;
}

static void hardwareTest_nic(void)
{	
	int peers[NUM_PEERS];
	scr_printf("NIC (Network Interface Card) Tests\n");
	scr_printf("Start 2 other VM instances with the project \"nicpeer\" before continuing\n");
	doPause();
	
	int startServer = 0;
	int startID = 0;
	for(int i=0; i<NUM_PEERS; i++) {
		peers[i] = tryConnect(&startServer, &startID);
		if (peers[i]==0)
			loopForever();
	}
	
	scr_printf("Found the required %d peers\n", NUM_PEERS);

	#define NUM_MESSAGES 10

	for(int num=0; num<NUM_MESSAGES; num++) {
		for(int i=0; i<NUM_PEERS; i++) {
			char buf[32];
			sprintf(buf, "N:%d", num + (i*NUM_MESSAGES));
			nic_sendString(peers[i], buf);
		}
	}
	
	int rcvCount[NUM_PEERS];
	int totalReceived = 0;
	memset(rcvCount, 0, sizeof(rcvCount));
	
	while(1)
	{
		char rcv[32];
		int srcID;
		int rcvSize = nic_receive(rcv, sizeof(rcv), &srcID);
		if (rcvSize==0)
			continue;
		
		scr_printf("Received from %s: %s\n", nic_getIDStr(srcID), rcv);
		
		// Check if the source ID is from any expected peer
		int peerIndex = getPeerIndex(srcID);
		checkf(peerIndex!=-1, "Message from unexpected peer\n");
		checkf(rcvSize>=4 && rcv[0]=='N' && rcv[1]==':', "Unexpected message format\n");
		
		int expectedNum = -(rcvCount[peerIndex] + (peerIndex*NUM_MESSAGES));
		int rcvNum = strtol(&rcv[2], NULL, 10);
		checkf(expectedNum==rcvNum, "Expected number %d, got %d\n", expectedNum, rcvNum);
		
		rcvCount[peerIndex]++;
		totalReceived++;
		
		if (totalReceived==NUM_MESSAGES*NUM_PEERS)
			break;
	}

	scr_printf("Test passed\n");
}
