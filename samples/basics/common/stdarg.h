#ifndef _stdarg_h_
#define _stdarg_h_

#include "stddef.h"

typedef struct va_list
{
	char* argptr;
} va_list;

// This is not a real function. The compiler recognizes this function by name
// and generates the required code
void _apcpu_va_start(va_list*);

#define _INTSIZEOF(n)   ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )
#define _ADDRESSOF(v) (&(v))

#define va_start(ap, v) _apcpu_va_start(&ap)

#define va_arg(ap,t)    ( *(t *)((ap.argptr += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define va_end(ap)      ( ap.argptr = 0 )

#endif

