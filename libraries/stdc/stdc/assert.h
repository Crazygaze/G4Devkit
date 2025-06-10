#ifndef _stdc_assert_h_
#define _stdc_assert_h_


#include "detail/config.h"
#include <hwcrt0_stdc.h>

/*!
* This one is not a standard function, but it's handy for when you want to
* assert something even on Release build
*/
#define always_assert(condition) if (!(condition)) { _assert_impl(); }

#endif
