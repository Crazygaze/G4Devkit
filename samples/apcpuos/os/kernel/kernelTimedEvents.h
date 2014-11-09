/*******************************************************************************
 * Misc code to support timed events
 ******************************************************************************/

#ifndef _APCPU_KERNELTIMEDEVENTS_H_
#define _APCPU_KERNELTIMEDEVENTS_H_

#include "kerneldefs.h"
#include <stddef_shared.h>

typedef void (*KrnTimedEventFunc)(void* data1, void* data2, void* data3);
typedef struct KrnTimedEvent
{
	// Time at which this event needs to execute
	KrnTimedEventFunc func;
	double time;
	void* data1;
	void* data2;
	void* data3;
} KrnTimedEvent;

/*! Initializes the timed events system
*/
void krn_initTimedEvents(void);

/*! Adds a timed event
*/
bool krn_addTimedEvent(double execTime, KrnTimedEventFunc func, void* data1,
	void* data2, void* data3);
	
/*! Checks the timed events queue for any overdue events, and executes them
*/
void krn_checkTimedEvents(void);

#endif
