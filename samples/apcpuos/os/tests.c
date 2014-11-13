//#include "appslist.h"
//#include "kernel/kerneldebug.h"

/*
static double hw_clk_currSecs;
typedef int (*KrnTimedEventFunc)(void* data1, void* data2);
typedef struct Kernel
{
	int timedEvents;
} Kernel;
Kernel krn;

typedef struct KrnTimedEvent
{
    // Time at which this event needs to execute
    float time;
    KrnTimedEventFunc func;
    void* data1;
    void* data2;
} KrnTimedEvent;

int priorityQueue_peek(void*, void** evt);
*/

/*
static int krn_tickTimedEvents()
{
    // Wake up any sleeping threads that are due to wake up
    KrnTimedEvent* evt;
	priorityQueue_peek(&krn.timedEvents, &evt);
	if (evt->time <= hw_clk_currSecs)
		return 1;
 
 //while( priorityQueue_peek(&krn.timedEvents, &evt) &&
//             (evt->time <= hw_clk_currSecs)) {
        //priorityQueue_popAndDrop(&krn.timedEvents);
        //evt->func(evt->data1, evt->data2);
    //}
}
*/