#ifndef _stdc_init_h_
#define _stdc_init_h_

#include "detail/config.h"

/*!
 * Sets the maximum verbosity.
 * It controls how much verbosity is compiled into the code.
 * 0 - None
 * 1 - Fatal
 * 2 - Error
 * 3 - Warning
 * 4 - Log
 * 5 - Verbose
 */
#ifndef _STDC_LOG_VERBOSITY
	#ifdef _DEBUG
		#define _STDC_LOG_VERBOSITY 5
	#else
		#define _STDC_LOG_VERBOSITY 4
	#endif
#endif

/*! Logs a Fatal error */
#if _STDC_LOG_VERBOSITY>=1
	#define LOG_FTL(...) _stdc_log("FTL:" __VA_ARGS__)
#else
	#define LOG_FTL(...)
#endif


/*! Logs an Error */
#if _STDC_LOG_VERBOSITY>=2
	#define LOG_ERR(...) _stdc_log("ERR:" __VA_ARGS__)
#else
	#define LOG_ERR(...)
#endif

/*! Logs a warning */
#if _STDC_LOG_VERBOSITY>=3
	#define LOG_WRN(...) _stdc_log("WRN:" __VA_ARGS__)
#else
	#define LOG_WRN(...)
#endif

/*! Logs a normal verbosity log */
#if _STDC_LOG_VERBOSITY>=4
	#define LOG_LOG(...) _stdc_log("LOG:" __VA_ARGS__)
#else
	#define LOG_LOG(...)
#endif

/*! Logs a verbose log */
#if _STDC_LOG_VERBOSITY>=5
	#define LOG_VER(...) _stdc_log("VER:" __VA_ARGS__)
#else
	#define LOG_VER(...)
#endif

/*!
 * To allow printing logs from inside the library itself, the library expects
 * the user to specify a logging function.
 * NOTE: The return value is unused. It's just to match the printf signature.
 */
typedef int(*LibCDebugLogFunc)(const char* fmt);

/*!
 * Used internally to call the real logging function.
 *  We wrap it in this one so we can set the #pragma below to catch formatting
 * errors
 */
int _stdc_log(const char* fmt, ...);
#pragma printflike _stdc_Log

#include <stddef.h>

/*!
 * Sets a log function that gets called by _stdc_Log
 * This doesn't have any dependencies and can be called at any time.
 * It's a good idea to call right at boot so the boot code can use the LOG_XXX
 * macros before calling stdc_init
 */
void stdc_setLogFunc(LibCDebugLogFunc logFunc);

/*!
 * This needs to be called before using anything else in the library, other than
 * stdc_setLogFunc
 */
void stdc_init(void* heapStart, unsigned heapSize);

#endif
