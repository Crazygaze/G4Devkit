#include <stdlib.h>
#include <appsdk/app_process.h>
#include <stdc_init.h>

#include "hwcrt0.h"

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
	}
}

int helloworld_main(void *)
{
	LOG_LOG("Hello World!");
	
	CreateThreadParams params = { 0 };
	params.entryFunc = anotherThread;
	params.stackSize = 1024;
	params.cookie = "world";
	HANDLE h = app_createThread(&params);
	
	static int count = 0;
	while(true)
	{
		LOG_LOG("Helloworld: %u!", count++);
		app_sleep(1000);
		if (anotherThreadCounter >= 1) {
			//bool res = app_closeHandle(app_getCurrentThread());
			bool res = app_closeHandle(h);
			LOG_LOG("Res: %d!", res);
		}
	}
	
	return EXIT_SUCCESS;
}
