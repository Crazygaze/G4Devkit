#ifndef _hwcommon_h_
#define _hwcommon_h_

#include <stddef.h>

/*!
This struct matches a register set.
Having a struct like this makes it easier to use from C, instead of just a
simple pointer/array
*/
typedef struct Ctx
{
	// Fields that must match the architecture register set
	int gregs[16];
	int flags;
	double fregs[16];
} Ctx;


/*!
 * Struct used to make hwi calls. It's used for both input and output
 */
typedef struct HwiData {
	unsigned int regs[4];
} HwiData;

typedef void (*InterruptHandler)(u32 data0, u32 data1, u32 data2, u32 data3);

int hwiCall(int bus, int funcNum, HwiData* data);


//
// The default devices are fixed a specific bus numbers
//
#define HWBUS_CPU 0
#define HWBUS_CLK 1
#define HWBUS_SCR 2
#define HWBUS_KYB 3
#define HWBUS_NIC 4
#define HWBUS_DKC 5

#define HWIERR_SUCCESS 0
#define HWIERR_NODEVICE 0x80000001
#define HWIERR_INVALIDFUNCTION 0x80000002
#define HWIERR_INVALIDMEMORYADDRESS 0x80000003

#define HWIFUNC_ID 0x80000000
#define HWIFUNC_DESCRIPTION 0x80000001
#define HWIFUNC_UUID 0x80000002

#endif
