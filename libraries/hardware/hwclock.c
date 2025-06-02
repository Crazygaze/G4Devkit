#include "hwclock.h"
#include <assert.h>
#include <string.h>


void hwclk_getTime(DateTime* dateTime)
{
	HwfSmallData data;
	int res = hw_hwfsmall(HWBUS_CLK, HWCLK_FUNC_GET_TIME, &data);
	always_assert(res == HWERR_SUCCESS);
	memcpy(dateTime, &data.regs[0], sizeof(DateTime));
}
