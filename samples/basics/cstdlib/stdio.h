#ifndef _stdio_h_
#define _stdio_h_

#include <stddef.h>
#include <stdarg.h>

int sprintf (char *buffer, const char *format, ...);
int vsprintf_impl(char* buffer, const char* format, va_list* vlist);
#define vsprintf(buffer,format,vlist) vsprintf_impl(buffer, format, &vlist);

#endif
