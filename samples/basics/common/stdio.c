#include "stdio.h"
#include "stddef.h"
#include "stdarg.h"
#include "extern/printf/printf.h"

int sprintf (char *buffer, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);	
	return printf_valist(&buffer, format, (int*)ap.argptr);
}

int vsprintf_impl(char* buffer, const char* format, va_list* vlist)
{
	return printf_valist(&buffer, format, (int*)vlist->argptr);
}
