#ifndef _stdc_limits_h_
#define _stdc_limits_h_

#include "detail/config.h"

/******************************************************************************/
// limits.h
/******************************************************************************/

#define MB_LEN_MAX 1

// char
#define CHAR_BIT 8
#define CHAR_MIN (-0x7f - 1)
#define CHAR_MAX 0x7f
#define SCHAR_MIN (-0x7f - 1)
#define SCHAR_MAX 0x7f
#define UCHAR_MAX 0xffU

// short
#define SHRT_MIN (-0x7fff - 1)
#define SHRT_MAX 0x7fff
#define USHRT_MAX 0xffffU

// int
#define INT_MIN (-0x7fffffff - 1)
#define INT_MAX 0x7fffffff
#define UINT_MAX 0xffffffffU

// long (same as int)
#define LONG_MIN INT_MIN
#define LONG_MAX INT_MAX
#define ULONG_MAX UINT_MAX

#endif
