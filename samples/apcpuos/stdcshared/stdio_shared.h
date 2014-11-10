#ifndef _stdio_shared_h_
#define _stdio_shared_h_

#include <stdarg_shared.h>

int sprintf(char *buffer, const char* format, ...);

int _apcpu_vsprintf(char* buffer, const char* format, va_list* vlist);
#define vsprintf(buffer,format,vlist) _apcpu_vsprintf(buffer, format, &vlist);

#endif

