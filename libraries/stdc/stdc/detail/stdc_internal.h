#ifndef _stdc_internal_h
#define _stdc_internal_h

#ifndef __STDC_HOSTED__
	#define __STDC_HOSTED__ 0
#endif

#ifndef _STDC_SHARED_DATA
	#define _STDC_SHARED_DATA 0
#endif


// Enable .data_shared/.bss_shared if required
#if _STDC_SHARED_DATA
	_Pragma("shareddata-on")
#endif

#endif
