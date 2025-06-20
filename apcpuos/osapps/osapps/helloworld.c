#include <stdlib.h>
#include <appsdk/app_process.h>
#include <stdc_init.h>
#include <string.h>

#include "hwcrt0.h"

void* anotherThreadStack;
volatile int anotherThreadCounter = 0;
void anotherThread(void* cookie)
{
	LOG_LOG("Hello from another %s!", (const char*)cookie);
	int count = 0;
	while(true)
	{
		LOG_LOG("Another world: %u!", count++);
		app_sleep(5000);
		anotherThreadCounter++;
	}
}

int helloworld_main(void *)
{
	LOG_LOG("Hello World!");
	
	defineZeroed(CreateThreadParams, params);
	
	params.entryFunc = anotherThread;
	params.stackSize = 1024;
	params.cookie = "world";
	
	defineZeroed(ThreadInfo, tinfo);
	tinfo.thread = app_createThread(&params, &anotherThreadStack);
	bool res = app_getThreadInfo(&tinfo);
	
	static int count = 0;
	while(true)
	{
		LOG_LOG("Helloworld: %u!", count++);
		app_sleep(2000);
		if (anotherThreadCounter >= 1) {
			//bool res = app_closeHandle(app_getCurrentThread());
			res = app_closeHandle(tinfo.thread);
			LOG_LOG("Res: %d!", res);
			if (res) {
				free(anotherThreadStack);
			}
		}
	}
	
	return EXIT_SUCCESS;
}
