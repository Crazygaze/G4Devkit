/*******************************************************************************
* NIC (Network Interface Card) driver
*******************************************************************************/
#ifndef _APCPU_HWNIC_H_
#define _APCPU_HWNIC_H_

#include "hwcommon.h"

#define HW_NIC_FUNC_SEND 1

hw_Drv* hw_nic_ctor(hw_BusId busid);
void hw_nic_dtor(hw_Drv* drv);

HWERROR hw_nic_sendDebug(const char* str);
HWERROR hw_nic_sendDebugV(const char* fmt, ...);
HWERROR hw_nic_sendDebugVA(const char* fmt, int* valist);

#endif
