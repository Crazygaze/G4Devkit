#include <stdlib.h>
#include <string.h>
#include "app_process.h"
#include "app_txtui.h"

static int derp;
// TODO : REMOVE THIS

static u32 stackSize;
static u32 usedStackSize;
static u32 freeStackSize;

void updateStackInfo(void)
{
	stackSize = app_getStackSize();
	usedStackSize = app_getUsedStackSize();
	freeStackSize = stackSize - usedStackSize;
}

void logmem(void)
{
	size_t usedmem, freemem, maxalloc;
	_getmemstats(&usedmem,&freemem,&maxalloc);
	LOG("Mem: Used=%u, Free=%u, MaxAlloc=%u", usedmem, freemem, maxalloc);
	LOG("Stack size=%d, used=%d, free=%d", stackSize, usedStackSize,
		stackSize - usedStackSize);
}

#define ARRAY_SIZE 10
CriticalSection cs;
int array_index;
int array[ARRAY_SIZE];

static void myThread(void* data)
{
	LOG( "Secondary Thread %u: %s\n", app_getThreadHandle(), (const char*)data);
	int derp=0;
	bool res;
	do
	{
		criticalSectionEnter(&cs);
		res = array_index<ARRAY_SIZE;
		if (res)
		{
			LOG("VAL %u - Thread %u", array[array_index], app_getThreadHandle());
			array_index++;
		}
		criticalSectionLeave(&cs);
		app_yield();
		for(int i=0; i<256; i++)
			derp += i;				
	} while (res);

	
	//app_sleep(10000);
}

static void myThreadMsgTest(void* data)
{
	LOG( "Secondary Thread %u: %s\n", app_getThreadHandle(), (const char*)data);
	
	for(int i=0;i<2048; i++) {
		derp += i;
	}
	ThreadMsg msg;
	while(app_getMessage(&msg))
	{
		LOG("%s: Received msg %d:%d:%d", (const char*)data, msg.id, msg.param1,
			msg.param2);
		app_sleep(2000);
		HANDLE sender = (HANDLE)msg.param2;
		app_postMessage(sender, msg.id, msg.param1,
			(uint32_t)app_getThreadHandle());
		app_sleep(10000);
		derp++;
	}
	
	LOG("Exiting thread %s", (const char*)data);
}


int testapp1(int p1)
{	
	LOG("testapp1 main thread: Handle %u", app_getThreadHandle());
	updateStackInfo();
	
	txtui_printfAtXY(&rootCanvas, 0,0, "Helllo world from app %d!! :)", p1);
	//
	// Test thread message queue
	//
	{
		HANDLE hthread = app_createThread(myThreadMsgTest, 800, "MsgQueueTest");
		LOG("Secondary thread: %u", hthread);
		app_postMessage(hthread, 80, 256, (uint32_t)app_getThreadHandle());
		app_postMessage(hthread, 90, 256, (uint32_t)app_getThreadHandle());
		app_postMessage(hthread, 100, 256, (uint32_t)app_getThreadHandle());
		app_postMessage(hthread, 110, 256, (uint32_t)app_getThreadHandle());
		app_postMessage(hthread, MSG_QUIT,0,(uint32_t)app_getThreadHandle());
		ThreadMsg msg;
		while(app_getMessage(&msg)) {
			LOG("MainThread: Received msg %d:%d:%d", msg.id, msg.param1,
				msg.param2);
			if (msg.id==110) {
				LOG("Breaking out...");
				break;
			}
			app_closeHandle(hthread);
		}
		app_sleep(10000);
		LOG("Trying to delete thread");
		bool res = app_closeHandle(hthread);
		LOG("Done... %d", res);		
	}
	app_sleep(5000);
	return EXIT_SUCCESS;
	criticalSectionInit(&cs);
	
	for(int i=0; i<ARRAY_SIZE; i++)
		array[i] = i;

	//test();

	LOG("Testing");
	logmem();
	void *ptr = malloc(4);
	logmem();
	void *ptr2 = malloc(1300);
	logmem();
	free(ptr);
	logmem();
	free(ptr2);
	logmem();
	
	//
	// \param func
	// \param stack size
	HANDLE hthread = app_createThread(myThread, 800, "Thread 1");
	app_yield();
	HANDLE hthread2 = app_createThread(myThread, 800, "Thread 2");
	app_yield();
	HANDLE hthread3= app_createThread(myThread, 800, "Thread 3");
	app_yield();
	HANDLE hthread4= app_createThread(myThread, 800, "Thread 4");
	app_yield();
	HANDLE hthread5= app_createThread(myThread, 800, "Thread 5");

	app_yield();
	if (hthread==INVALID_HANDLE)
		LOG("Could not create thread");
	else
		LOG("Created thread with handle %u", hthread);
	app_sleep(8000);

	//app_closeHandle(hthread);
	
	//return EXIT_SUCCESS;

	//app_sleep(1000);
	int counter=0;
	while(TRUE) {
		p1++;
		for(int i=0; i<128; i++)
			derp += i;				
		app_sleep(250);
		txtui_printfAtXY(&rootCanvas, 0,5, "Counter in app %d: %d", p1,
			counter++);		
		//LOG("Hello from main thread %s : %d", "Rui", derp);
	}
	
	return EXIT_SUCCESS;
}
