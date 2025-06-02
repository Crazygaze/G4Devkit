#ifndef _hwcommon_h_
#define _hwcommon_h_

#include "hwcrt0.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdc_init.h>
#include "os_config.h"


/*!
 * Signature for a Driver's irq handler.
 */
typedef void (*hw_IrqHandlerFunc)(void);


/*!
 * Base struct for hardware devices drivers.
 * Each specialized driver then has this as the first field in their actual
 * struct.
 */
typedef struct hw_Drv
{
	// The device's bus
	uint8_t bus;
	//! Called to destroy the driver
	void (*dtor)(struct hw_Drv*);

	// How many times we've handling irqs from the device
	u32 irqCount;
	
	//! IRQ handler
	hw_IrqHandlerFunc handler;
} hw_Drv;

/*!
 * Function signature for creating and destroying a driver.
 * Each hardware device implementaton must have the two required functions with
 * these signatures.
 */
typedef hw_Drv* (*hw_DrvCtorFunc)(uint8_t bus);
typedef void (*hw_DrvDtorFunc)(hw_Drv* drv);

/*!
 * Initializes all connected devices
 */
void hw_initAll(void);

/*!
 * Handles an IRQ that happened for the specified bus
 */
void hw_handleIRQ(unsigned bus);


#endif
