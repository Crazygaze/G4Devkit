#ifndef _hwnic_h_
#define _hwnic_h_

#include "hwcommon.h"

/*! Used internally */
hw_Drv* hw_nic_ctor(uint8_t bus);
/*! Used internally */
void hw_nic_dtor(hw_Drv* drv);

int hwnic_sendDebug(const char* str);

int hwnic_sendDebugV(const char* fmt, ...);
#pragma printflike hw_nic_sendDebugV

#endif
