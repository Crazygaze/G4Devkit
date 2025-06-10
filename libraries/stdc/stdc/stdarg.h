#ifndef _stdc_stdarg_h_
#define _stdc_stdarg_h_

#include "detail/config.h"
#include <stddef.h>

typedef struct
{
	char* argptr;
} va_list;

// This is not a real function. The compiler recognizes this function by name
// and generates the required code
void _apcpu_va_start(va_list*);

#define _INTSIZEOF(n)   ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )
#define _ADDRESSOF(v) (&(v))

#define va_start(ap, v) _apcpu_va_start(&ap)

//! Access the next variadic function argument
// \param t It must be "int", or "double"
//
// Note that with variadic functions, arguments are promoted, therefore it's
// invalid to specify t as e.g "char". See:
// https://en.cppreference.com/w/cpp/language/variadic_arguments
// https://www.eskimo.com/~scs/cclass/int/sx11c.html
// https://stackoverflow.com/questions/24967283/why-float-is-promoted-to-double-when-using-template-and-stdarg-function
#define va_arg(ap,t)    ( *(t *)((ap.argptr += _INTSIZEOF(t)) - _INTSIZEOF(t)) )

#define va_end(ap)      ( ap.argptr = 0 )

#endif

