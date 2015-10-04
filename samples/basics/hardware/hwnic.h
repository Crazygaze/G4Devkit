#ifndef _hwnic_h_
#define _hwnic_h_

#include "hwcommon.h"

//
// Interrupt reasons for keyboard
//
#define HWNIC_INTERRUPT_GENERIC 0

/**! Returns an IP style string of the specified network card address
\param id
	Id to convert
\return
	Pointer to a static buffer with the string.
	Note that you should not keep this pointer, as every call thi this function
	will overwrite it.
*/
const char* nic_getIDStr(u32 id);

u32 nic_getID();
void nic_send(u32 dstID, const char* buf, int size);
void nic_sendString(int dstID, const char* str);
int nic_receive(char* buf, int bufsize, u32* srcID);

typedef struct NicState
{
	// outgoing buffer
	u32 outCapacity;
	u32 outUsedSize;
	// incoming buffer
	u32 inCapacity;
	u32 inUsedSize;
	
	// global stats
	u32 totalSent; // total bytes sent
	u32 totalReceived; // total bytes received
	u32 outPacketsDropped; // number of outgoing packets dropped
	u32 inPacketsDropped; // number of incoming packets dropped
} NicState;

void nic_state(NicState* state);

#endif
