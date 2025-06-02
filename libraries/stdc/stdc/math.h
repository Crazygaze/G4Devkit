// According to https://en.cppreference.com/w/c/header/math

#ifndef _stdc_math_h_
#define _stdc_math_h_

#include <stddef.h>

// Internal detail
#define _HUGE_ENUF 1e+300

#define INFINITY ((float)(_HUGE_ENUF * _HUGE_ENUF))
#define HUGE_VAL ((double)INFINITY)
#define HUGE_VALF ((float)INFINITY)

#define NAN ((float)(INFINITY * 0.0f))

typedef float float_t;
typedef double double_t;

/*!
 * Checks if the given number is infinite
 */
#define isinf(arg) _isinfImpl(arg)
int _isinfImpl(double arg);

/*!
 * Checks if the given number is NaN
 */
#define isnan(arg) _isnanImpl(arg)
int _isnanImpl(double arg);

/*!
 * Computes absolute value of a floating-point value.
 */
double fabs(double arg)
INLINEASM("\t\
fabs f0, f0");

/*!
 * Computes absolute value of a floating-point value.
 */
float fabsf(float arg)
INLINEASM("\t\
fabs f0, f0");

/*!
 * Computes natural (base e) logarithm.
 */
float logf(float arg)
INLINEASM("\t\
flgn f0, f0");

/*!
 * Computes natural (base e) logarithm.
 */
double log(double arg)
INLINEASM("\t\
flgn f0, f0");

/*!
 * Computes base-2 logarithm.
 */
float log2f(float arg);

/*!
 * Computes base-2 logarithm.
 */
double log2(double arg);

#endif
