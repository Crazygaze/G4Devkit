#include "hwnic.h"
#include <assert.h>
#include <string.h>

#define HWINICERR_GENERIC 0x1

#define HWNICFUNC_SEND 0 // Send a packet
#define HWNICFUNC_RECEIVE 1 // Retrieve a packet from the incoming buffer
#define HWNICFUNC_QUERYBUFFERS 2 // Query the state of the buffers
#define HWNICFUNC_QUERYGLOBAL 3 // Query some global stats

void nic_send(u32 dstID, const char* buf, int size)
{
	HwiData data;
	data.regs[0] = dstID;
	data.regs[1] = (u32)buf;
	data.regs[2] = (u32)size;
	int res = hwiCall(HWBUS_NIC, HWNICFUNC_SEND, &data);
	always_assert(res==HWIERR_SUCCESS);
}

void nic_sendString(int dstID, const char* str)
{
	nic_send(dstID, str, strlen(str)+1);
}

int nic_receive(char* buf, int bufsize, u32* srcID)
{
	HwiData data;
	data.regs[0] = (u32)buf;
	data.regs[1] = (u32)bufsize;
	int res = hwiCall(HWBUS_NIC, HWNICFUNC_RECEIVE, &data);
	if (res==HWINICERR_GENERIC)
		return 0; // If there is no data to receive
	
	always_assert(res==HWIERR_SUCCESS);			
	*srcID = data.regs[0];
	return data.regs[1];
}

void nic_state(NicState* state)
{
	HwiData data;
	int res = hwiCall(HWBUS_NIC, HWNICFUNC_QUERYBUFFERS, &data);
	always_assert(res==HWIERR_SUCCESS);
	state->outCapacity = data.regs[0];
	state->outUsedSize = data.regs[1];
	state->inCapacity = data.regs[2];
	state->inUsedSize = data.regs[3];
	
	res = hwiCall(HWBUS_NIC, HWNICFUNC_QUERYGLOBAL, &data);
	always_assert(res==HWIERR_SUCCESS);
	state->totalSent = data.regs[0];
	state->totalReceived = data.regs[1];
	state->outPacketsDropped = data.regs[2];
	state->inPacketsDropped = data.regs[3];
}
