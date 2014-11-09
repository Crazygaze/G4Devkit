#ifndef _stdlib_shared_h_
#define _stdlib_shared_h_

#include <stddef_shared.h>

void itoa(int value, char *str, int base);

/* ftoa function. NOT THREAD SAFE */
#define _FTOA_TOO_LARGE	-2	/* |input| > 2147483520 */
#define _FTOA_TOO_SMALL	-1	/* |input| < 0.0000001 */
/* ftoa returns static buffer of ~15 chars. If the input is out of
 * range, *status is set to either of the above #define, and 0 is
 * returned. Otherwise, *status is set to 0 and the char buffer is
 * returned.
 * This version of the ftoa is fast but cannot handle values outside
 * of the range listed. Please contact us if you need a (much) larger
 * version that handles greater ranges.
 * Note that the prototype differs from the earlier version of this
 * function. Example:
 *
 * int stat;
 * char *s = ftoa(123.45, &stat);
 * if (stat == 0)	// all OK!
 * 
 */
char *ftoa(float f, int *status);

#define FLT_MAX 3.402823466e+38F
#define FLT_MIN 1.175494351e-38F

#define min(a,b) ((a)<=(b) ? (a) : (b))
#define max(a,b) ((a) >(b) ? (a) : (b))

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#include <details/memdetails.h>

#endif
