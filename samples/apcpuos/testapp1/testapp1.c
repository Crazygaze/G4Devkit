/*!
 *	As its name suggest, testapp1 is used as a sample and test
 *	Built on top of defaultOS, it runs a loop that does nothing but increments a
 *	variable
 *
 *	It also displays informations about the stack and memory usage
 *	Along with some tests on threads.
 *
 *	@file: testapp1.c
 */

#include <stdlib.h>
#include <string.h>

#include "app_process.h" // Used mainly to get process stats

#include "app_txtui.h" // Display to the screen

/*!
 *	Used for derp purpose (derp count: 1)
 *	This variable will increments itself in the loop
 *
 *	@property derp
 */
static int derp;

// TODO : REMOVE THIS

/*!
 *	Overall size of the stack
 *	Used for log purpose
 *
 *	@property stackSize
 */
static u32 stackSize;

/*!
 *	Used for log purpose
 *
 *	@property usedStackSize
 */
static u32 usedStackSize;

/*!
 *	Used for log purpose
 *
 *	@property freeStackSize
 */
static u32 freeStackSize;

/*!
 *	Loop to log the stack infos
 *
 *	@method updateStackInfo
 */
void updateStackInfo(void)
{
	stackSize = app_getStackSize();
	usedStackSize = app_getUsedStackSize();
	freeStackSize = stackSize - usedStackSize;
}

/*!
 *	Log memory and stack usage
 *
 *	@method logmem
 */
void logmem(void)
{
	size_t
		/*!
		 *	Used memoru
		 *
		 *	@property usedmem
		 */
		usedmem,

		/*!
		 *	Memory available
		 *
		 *	@property freemem
		 *
		 */
		freemem,

		/*!
		 *	Max allocation of the memory
		 *
		 *	@property maxalloc
		 */
		maxalloc
	;

	/*
	 *	Get statistics of memory and stack, for log purpose
	 */
	_getmemstats(&usedmem,&freemem,&maxalloc);
	LOG("Mem: Used=%u, Free=%u, MaxAlloc=%u", usedmem, freemem, maxalloc);
	LOG("Stack size=%d, used=%d, free=%d", stackSize, usedStackSize,
		stackSize - usedStackSize);
}

#define ARRAY_SIZE 10 // can this go to 11?

/*!
 *	Used for thread managment.
 *	Test the thread criticalSection.
 *
 *	@beta
 *	@property cs
 */
CriticalSection cs;

/*!
 *	Index of the array, used in loop that test the critical section
 *
 *	@property array_index
 */
int array_index;

/*!
 *	Array that contains thread id and its value
 *
 *	@property array
 */
int array[ARRAY_SIZE];


/*!
 * 	Main application
 *	Critical section test.
 *
 *	@method myThread
 *	@param data
 */
static void myThread(void* data)
{
	/*
	 * Log current thread handle
	 */
	LOG( "Secondary Thread %u: %s\n", app_getThreadHandle(), (const char*)data);
	/*!
	 *	derpyderp! (derp count:2)
	 *
	 *	@property derp
	 */
	int derp=0;

	/*!
	 *	Check when no more keys in the array
	 *
	 *	@property res
	 */
	bool res;
	do
	{
		criticalSectionEnter(&cs); // launch thread compression
		res = array_index<ARRAY_SIZE; // are we at end of the array?

		/*
		 *	Log some infos about the thread and increments
		 */
		if (res)
		{
			LOG("VAL %u - Thread %u", array[array_index], app_getThreadHandle());
			array_index++;
		}
		criticalSectionLeave(&cs); // stop thread compression
		app_yield();
		for(int i=0; i<256; i++)
			derp += i; // background loop, for derp purpose (derp count:3)
	} while (res);

	//app_sleep(10000);
}

/*!
 *	Test if a message can be sent.
 *
 *	@method myThreadMsgTest
 *	@param data - thread info
 */
static void myThreadMsgTest(void* data)
{
	/*
	 * Log current thread handler
	 */
	LOG( "Secondary Thread %u: %s\n", app_getThreadHandle(), (const char*)data);

	for(int i=0;i<2048; i++) {
		derp += i; // And then, during 2048 i, derps were thrown (derp count: 4)
	}
	/*!
	 *	Create a new thread for message
	 *
	 *	@property msg
	 */
	ThreadMsg msg;

	/*
	 *	While there is message
	 */
	while(app_getMessage(&msg))
	{
		LOG("%s: Received msg %d:%d:%d", (const char*)data, msg.id, msg.param1,
			msg.param2);
		app_sleep(2000); // sleep for 2000 ms

		/*
		 * Create a send handler
		 */
		HANDLE sender = (HANDLE)msg.param2;

		/*
		 * Send a message through the send handler
		 */
		app_postMessage(
			sender,
			msg.id,
			msg.param1,
			(uint32_t)app_getThreadHandle()
		);

		app_sleep(10000); // sleep for 10000 ms

		derp++; // ninja derp spotted (derp count: 5)
	}

	LOG("Exiting thread %s", (const char*)data);
}

/*!
 * 	Runs some loops, display an hello world
 *
 *	@method testapp1
 * 	@param int - counter
 * 	@return int - EXIT_SUCCESS
 */
int testapp1(int p1)
{
	LOG("testapp1 main thread: Handle %u", app_getThreadHandle());

	/*
	 *	Get information about Stack :
	 *		+ overall stack size
	 *		+ stack size used
	 *		+ stack size free
	 */
	updateStackInfo();

	/*
	 *	Print a hello world to the screen
	 */
	txtui_printfAtXY(&rootCanvas, 0,0, "Helllo world from app %d!! :)", p1);

	//
	// Test thread message queue
	//
	{
		/*
		 *	Create message thread handler
		 */
		HANDLE hthread = app_createThread(myThreadMsgTest, 800, "MsgQueueTest");
		LOG("Secondary thread: %u", hthread);
		/*
		 *	Send message to thread
		 */
		app_postMessage(hthread, 80, 256, (uint32_t)app_getThreadHandle());
		app_postMessage(hthread, 90, 256, (uint32_t)app_getThreadHandle());
		app_postMessage(hthread, 100, 256, (uint32_t)app_getThreadHandle());
		app_postMessage(hthread, 110, 256, (uint32_t)app_getThreadHandle());
		app_postMessage(hthread, MSG_QUIT,0,(uint32_t)app_getThreadHandle());

		/*
		 *	Does messages has been get?
		 */
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

		app_sleep(10000); // sleep for 10000 ms

		LOG("Trying to delete thread");
		bool res = app_closeHandle(hthread);
		LOG("Done... %d", res);
	}
	app_sleep(5000); // sleep for  5000 ms

	/*
	 *	Comment this (at your own risk) if you want more derps
	 */
	return EXIT_SUCCESS;
	/*criticalSectionInit(&cs);*/

	/*
	 * This can go to 11
	 *
	 * However it will go to 10
	 */
	for(int i=0; i<ARRAY_SIZE; i++) {
		array[i] = i;
	}

	/*
	 *	Test if memory can be logged
	 */
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

	//	Create some threads
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
	if (hthread==INVALID_HANDLE) {
		LOG("Could not create thread");
	} else {
		LOG("Created thread with handle %u", hthread);
	}
	app_sleep(8000);

	//app_closeHandle(hthread);

	/*
	 *	Comment this (at your own risk) if you want more derps
	 */
	//return EXIT_SUCCESS;

	//app_sleep(1000);
	int counter=0;

	/*!
	 *	Loop for infinite derp
	 */
	while(TRUE) {
		p1++;
		for(int i=0; i<128; i++) {
			derp += i; // The last derp always the best (derp count: 6)
		}
		app_sleep(250);
		txtui_printfAtXY(&rootCanvas, 0,5, "Counter in app %d: %d", p1,
			counter++);
		//LOG("Hello from main thread %s : %d", "Rui", derp);
	}

	// No more derps! do not comment this!
	return EXIT_SUCCESS;
}
