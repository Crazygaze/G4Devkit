#ifndef _hardwaretests_common_h_
#define _hardwaretests_common_h_

#include <hwscreen.h>

typedef void (*DeviceTestFunction)(void);

typedef struct DeviceTest
{
	Driver* driver;
	DeviceTestFunction testFunc;
} DeviceTest;

extern int 
/*!
*/
#define checkf(condition, fmt, ...) \
	if (!(condition)) scr_printf(fmt, __VA_ARGS__)

/*!
*/
#define check(condition) \
	if (!(condition)) \
		scr_printf("	FAILED: Line %d: \"%s\"\n", __LINE__, #condition)

/*!
*/
#define check_nl(condition) \
	if (condition) \
		scr_printf(" OK\n"); \
	else \
		scr_printf(" FAILED: Line %d : \"%s\"\n", __LINE__, #condition)

/*!
*/
void doPause(void);

#endif
