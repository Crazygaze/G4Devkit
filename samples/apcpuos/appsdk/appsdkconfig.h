/*
 * Miscellaneous defines for configuring the appsdk library
 */
#ifndef _appsdk_config_h_
#define _appsdk_config_h_

/*
 * Maximum string size allowed for calls to app_outputDebugString.
 * Calls to app_outputDebugString that result in strings bigger than this will
 * corrupt the application
 */
#define APPSDK_DEBUGSTRING_SIZE 256

#endif
