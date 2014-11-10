#include "kernel/kernelTimedEvents.h"
#include "kernel/kernel.h"
#include "kernel/kerneldebug.h"
#include "hw/hwclock.h"
#include "hw/hwcpu.h"

/*!
* Used as a callback to PriorityQueue32, to sort TCB entries according to
* what sleeping threads will wake up first
*/
static int krnTimedEvent_cmp(const void* a_, const void* b_)
{
	KrnTimedEvent* a = (KrnTimedEvent*)a_;
	KrnTimedEvent* b = (KrnTimedEvent*)b_;
	if (a->time < b->time)
		return 1;
	else if (a->time > b->time)
		return -1;
	else
		return 0;
}

void krn_initTimedEvents(void)
{
	bool res = priorityQueue_create(&krn.timedEvents, sizeof(KrnTimedEvent), 4,
		krnTimedEvent_cmp);
	kernel_check(res);
}

bool krn_addTimedEvent(double execTime, KrnTimedEventFunc func, void* data1,
	void* data2, void* data3)
{
	KrnTimedEvent evt;
	evt.time = execTime;
	evt.func = func;
	evt.data1 = data1;
	evt.data2 = data2;
	evt.data3 = data3;
	bool res = priorityQueue_push(&krn.timedEvents, &evt);
	return res;
}

void krn_checkTimedEvents(void)
{
	// Wake up any sleeping threads that are due to wake up
	KrnTimedEvent* evt;
	while( priorityQueue_peek(&krn.timedEvents, &evt) &&
		  (evt->time <= hw_clk_currSecs))
	{
		priorityQueue_popAndDrop(&krn.timedEvents);		
		evt->func(evt->data1, evt->data2, evt->data3);
	}
}
