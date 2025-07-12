#include <stdlib.h>
#include <appsdk/app_process.h>
#include <appsdk/app_stdio.h>
#include <stdc_init.h>
#include <string.h>
#include <assert.h>

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

const char* bigString = "\
Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor1.\
Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor2.\
Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor3.\
Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor4.\
Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor5.\
Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor6.\
Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor7.";

void testFileWrite(void)
{
	FILE* f = fopen("12345678.txt", "w");
	LOG_LOG("fopen = %p", f);
	
	int done = fwrite(bigString, 1, strlen(bigString)+1, f);
	
	int res = fclose(f);
	LOG_LOG("fclose=%d", res);
}

void testFileRead(void)
{
	FILE* f = fopen("12345678.txt", "r+");
	LOG_LOG("fopen = %p", f);
	
	char buf[512*2];
	memset(buf, 0, sizeof(buf));
	
	int done = fread(buf, 1, strlen(bigString)+1, f);
	assert(strcmp(bigString, buf) == 0);
	
	int res = fclose(f);
	LOG_LOG("fclose=%d", res);
	app_sleep(1000000);
}

int helloworld_main(void *)
{
	LOG_LOG("Hello World!");

	testFileWrite();
	testFileRead();
	app_sleep(1000000);
	
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
