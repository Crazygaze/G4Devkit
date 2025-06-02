#ifndef _appsdk_app_process_h
#define _appsdk_app_process_h

#include "app_config.h"
#include "os_shared/process_shared.h"


/*!
 * Causes the current thread to sleep for the specifed milliseconds.
 * Sleep duration isn't necessarily accurate, as it depends on Kernel's quantum,
 * and overlal load on the system.
 * It is however guaranteed to be >= ms
 */
void app_sleep(u32 ms);

/*!
 * Sends a string to the debug output.
 *
 * \note The return type is unused. It is just to patch the printf signature.
 */
int app_outputDebugStringF(const char* fmt, ...);
int app_outputDebugString(const char* fmt);

#endif
