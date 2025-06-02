#ifndef _hwnic_h_
#define _hwnic_h_

#include "hwcommon.h"

/*! Used internally */
hw_Drv* hwnic_ctor(uint8_t bus);
/*! Used internally */
void hwnic_dtor(hw_Drv* drv);

/*!
 * Sets the irq modes
 *
 * \param irqOnSend If true, the device will generate an IRQ when the outgoing
 * buffer is empty
 * \param irqOnReceive If true, the device will generate an IRQ when new data is
 * received
 */
void hwnic_setIRQMode(bool irqOnSend, bool irqOnReceive);

#endif

