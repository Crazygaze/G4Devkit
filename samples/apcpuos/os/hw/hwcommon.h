/*******************************************************************************
* Common code for hardware device drivers
*******************************************************************************/

#ifndef _APCPU_HWCOMMON_H_
#define _APCPU_HWCOMMON_H_

#include "kernel/kerneldefs.h"
#include "appsdk/kernel_shared/syscalls_shared.h"
#include <stddef_shared.h>

/*
 * = 0 Success
 * <0 Generic error.
 * >0 Error specific to the device.
 */
typedef int32_t HWERROR;

typedef uint16_t hw_BusId;
#define HWBUS_MAX 0xFF

/*
 * Fixed devices bus numbers
 */
#define HWBUS_CPU 0
#define HWBUS_CLK 1
#define HWBUS_SCR 2
#define HWBUS_KYB 3
#define HWBUS_NIC 4
#define HWBUS_DKC 5
#define HWBUS_DEFAULTDEVICES_MAX 6

#define HWERR_SUCCESS 0x0
#define HWERR_DEFAULT_DEVICENOTFOUND 0x80000001
#define HWERR_DEFAULT_INVALIDFUNCTION 0x80000002
#define HWERR_DEFAULT_INVALIDMEMORYADDRESS 0x80000003

#define HWDEFAULT_FUNC_QUERYINFO 0x80000000
#define HWDEFAULT_FUNC_QUERYDESC 0x80000001

/*
 * Struct used to make hwi calls
 */
typedef struct hw_HwiData {
	// r0 to r10 can be used
	unsigned int regs[11];
} hw_HwiData;

typedef void (*hw_IrqHandler)(uint16_t reason, uint32_t data1, uint32_t data2);

typedef struct hw_Drv
{
	hw_BusId bus;

	// Destructor
	void (*dtor)(struct hw_Drv*);
	// This should be set by the driver constructor function	
	hw_IrqHandler irqHandler; 

	const char* (*getErrorMsg)(uint32_t);
} hw_Drv;


typedef hw_Drv* (*hw_CreateDriver)(hw_BusId);
typedef void (*hw_Destroydriver)(hw_Drv*);

/*
 * Performs a generic hwi call, using all the registers.
 * If the required hwi call is known to only use a few registers, consider
 * using the simpler calls bellow.
 * \param
 *	data Register contents as requested by the device and function desired.
 *	Will also contain the register values after the call to hwi.
 */
HWERROR hw_hwiFull(hw_HwiData* data);


/*
 * Performs a simple hwi call, where all parameters and required return values
 * don't change anything else other than the registers r0-r3.
 * \note
 *	Doesn't check for any errors, since the return value is the register r1,
 *	which is normally where a device returns any data.
 * \param bus Device bus
 * \param func Device function number
 */
uint32_t hw_hwiSimple0(
	__reg("r0") hw_BusId bus,
	__reg("r1") uint32_t func)
INLINEASM("\t\
hwi \n\t\
mov r0,r1");

// Same as the hw_hwiSimple0 above, but were we expect a double as a return (f0)
double hw_hwiSimple0_Double(
	__reg("r0") hw_BusId bus,
	__reg("r1") uint32_t func)
INLINEASM("\t\
hwi");

uint32_t hw_hwiSimple1(
	__reg("r0") hw_BusId bus,
	__reg("r1") uint32_t func,
	__reg("r2") uint32_t p1)
INLINEASM("\t\
hwi \n\t\
mov r0,r1");

uint32_t hw_hwiSimple2(
	__reg("r0") hw_BusId bus,
	__reg("r1") uint32_t func,
	__reg("r2") uint32_t p1,
	__reg("r3") uint32_t p2)
INLINEASM("\t\
hwi \n\t\
mov r0,r1");

extern hw_Drv* hw_drivers[HWBUS_DEFAULTDEVICES_MAX];
void hw_initAll(void);


#endif
