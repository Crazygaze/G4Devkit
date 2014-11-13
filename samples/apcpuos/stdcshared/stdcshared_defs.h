#ifndef _stdcshared_defs_h_
#define _stdcshared_defs_h_

// NOTE: the return value is unused at the moment. It's
typedef int(*DebugLogFunc)(const char* fmt, ...);
extern DebugLogFunc debugLogFunc;

void stdcshared_init(DebugLogFunc logfunc,void* heapStart, unsigned heapSize);

#ifdef DEBUG
	#define LOG(...) (*debugLogFunc)(__VA_ARGS__)
#else
	#define LOG(...) (void)(0)
#endif

#endif

