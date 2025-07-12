#ifndef _hwdkc_h_
#define _hwdkc_h_

#include "hwcommon.h"

/*! Used internally */
hw_Drv* hwdkc_ctor(uint8_t bus);
/*! Used internally */
void hwdkc_dtor(hw_Drv* drv);

#endif
