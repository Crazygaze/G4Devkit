#include "stdlib_shared.h"
#include "string_shared.h"

DebugLogFunc debugLogFunc;

void stdcshared_init( DebugLogFunc logfunc, void* heapStart, size_t heapSize )
{
	debugLogFunc = logfunc;
	
	if (heapSize)
	{
		memset(heapStart,0,heapSize);
		_mem_init(heapStart, heapSize);
	}
}
