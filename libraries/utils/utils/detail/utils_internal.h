/*!
 * This file must be included in every C file of the library.
 * It should NOT be reachable from any public headers
 *
 */

#ifndef _utils_internal_h_
#define _utils_internal_h_

#ifndef _UTILS_SHARED_DATA
	#define _UTILS_SHARED_DATA 0
#endif

// Enable .data_shared/.bss_shared if required
#if _UTILS_SHARED_DATA
_Pragma("shareddata-on")
#endif

#endif
