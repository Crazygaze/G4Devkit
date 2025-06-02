#ifndef _hwclock_h_
#define _hwclock_h_

#include "hwcommon.h"

#define HWCLK_FUNC_GET_TIME 5

/*! Used internally */
hw_Drv* hw_clk_ctor(uint8_t bus);
/*! Used internally */
void hw_clk_dtor(hw_Drv* drv);

typedef struct {
	int16_t year; //
	int16_t month; // 1-31
	int16_t dayOfWeek; // 0(Monday)-6
	int16_t day; // 1-31
	int16_t hour; // 0-23
	int16_t minute; // 0-59
	int16_t second; // 0-59
	int16_t millisecond; // 0-999
} DateTime;

/*! Gets the current utc time */
void hwclk_getTime(DateTime* dateTime);

#endif
