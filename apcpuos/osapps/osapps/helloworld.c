#include <stdlib.h>
#include <appsdk/app_process.h>
#include <stdc_init.h>
#include <string.h>

#include "hwcrt0.h"

typedef struct Foo {
	int a;
	int b;
	int c;
	Mutex mtx;
} Foo;

Foo gFoo;

void* anotherThreadStack;
volatile int anotherThreadCounter = 0;
void anotherThread(void* cookie)
{
	LOG_LOG("Hello from another %s!", (const char*)cookie);
	int count = 0;
	while(true)
	{
		LOG_LOG("Another world: %u!", count++);
		app_sleep(1000);
		anotherThreadCounter++;
		
		app_lockMutex(&gFoo.mtx);
		gFoo.c = 1;
		gFoo.a++;
		gFoo.b++;
		app_unlockMutex(&gFoo.mtx);
	}
}

int helloworld_main(void *)
{
	LOG_LOG("Hello World!");
	
	defineZeroed(CreateThreadParams, params);
	
	params.entryFunc = anotherThread;
	params.stackSize = 1024;
	params.cookie = "world";
	
	app_createMutex(&gFoo.mtx);
	
	defineZeroed(ThreadInfo, tinfo);
	tinfo.thread = app_createThread(&params, &anotherThreadStack);
	bool res = app_getThreadInfo(&tinfo);
	
	static int count = 0;
	while(true)
	{
		LOG_LOG("Helloworld: %u!", count++);
		app_sleep(1000);
		if (anotherThreadCounter >= 1) {
		}
		
		app_lockMutex(&gFoo.mtx);
		gFoo.c = 2;
		gFoo.a++;
		gFoo.b++;
		app_sleep(100);
		app_unlockMutex(&gFoo.mtx);
	}
	
	app_destroyMutex(&gFoo.mtx);
	app_closeHandle(tinfo.thread);
	free(anotherThreadStack);

	return EXIT_SUCCESS;
}
