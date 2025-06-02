#include "detail/stdc_internal.h"
#include "stdc_init.h"
#include <string.h>
#include "detail/memdetails.h"
#include <stdarg.h>
#include <stdio.h>

static LibCDebugLogFunc stdc_debugLogFunc;

void stdc_setLogFunc(LibCDebugLogFunc logFunc)
{
	assert(stdc_debugLogFunc == NULL);
	stdc_debugLogFunc = logFunc;
}

int _stdc_log(const char* fmt, ...)
{
	if (stdc_debugLogFunc) {
		va_list ap;
		char buf[256];
		va_start(ap, fmt);
		vsnprintf(buf, 256, fmt, ap);
		(*stdc_debugLogFunc)(buf);
	}

	return 0;
}

void stdc_init(void* heapStart, unsigned heapSize)
{
	if (heapSize) {
		memset(heapStart, 0, heapSize);
		_mem_init(heapStart, heapSize);
		_mem_debug();
	}
}
