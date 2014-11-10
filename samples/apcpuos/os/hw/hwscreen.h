/*******************************************************************************
 * Screen hardware device driver
 ******************************************************************************/

#ifndef _APCPU_HWSCREEN_H_
#define _APCPU_HWSCREEN_H_

#include "hwcommon.h"

#define HW_SCR_FUNC_GETINFO 0
#define HW_SCR_FUNC_MAP 1

hw_Drv* hw_scr_ctor(hw_BusId bus);
void hw_scr_dtor(hw_Drv* drv);

uint32_t hw_scr_getBufferSize(void);
void* hw_scr_getScreenBuffer(void);
int hw_scr_getScreenXRes(void);
int hw_scr_getScreenYRes(void);

void hw_scr_mapBuffer(void* buffer);

#endif
