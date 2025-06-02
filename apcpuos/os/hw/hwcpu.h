#ifndef _hwcpu_h_
#define _hwcpu_h_

#include "hwcommon.h"

/*! Used internally */
hw_Drv* hwcpu_ctor(uint8_t bus);
/*! Used internally */
void hwcpu_dtor(hw_Drv* drv);

#endif
