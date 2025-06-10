#include <stdlib.h>
#include <appsdk/app_process.h>
#include <stdc_init.h>

#include "hwcrt0.h"

void anotherThread(void* cookie)
{
	LOG_LOG("Hello from another %s!", (const char*)cookie);
	static int count = 0;
	while(true)
	{
		LOG_LOG("Another world: %u!", count++);
		app_sleep(5000);
	}
}

int helloworld_main(void *)
{
	int a[2] = {1, 2};
	LOG_LOG("Hello World!");
	
	CreateThreadParams params = { 0 };
	params.entryFunc = anotherThread;
	params.stackSize = 512;
	params.cookie = "world";
	HANDLE h = app_createThread(&params);

	static int count = 0;
	while(true)
	{
		LOG_LOG("Helloworld: %u!", count++);
		app_sleep(3000);
	}
	
	return EXIT_SUCCESS;
}
