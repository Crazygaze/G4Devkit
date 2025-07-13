#include <stdlib.h>
#include <appsdk/app_process.h>
#include <appsdk/app_stdio.h>
#include <stdc_init.h>
#include <string.h>
#include <assert.h>

#include "hwcrt0.h"

void otherThread(void* cookie)
{
	LOG_LOG("Hello from other thread: cookie = %s!", (const char*)cookie);
	int count = 0;
	ThreadMsg msg;
	while(app_getMsg(&msg))
	{
		LOG_LOG("Other thread: %u. msgId=%u, param1=%u, param2=%u", count++,
			msg.id, msg.param1, msg.param2);
		break;
	}
	
	LOG_LOG("Quitting other thread");
}

int helloworld_main(void *)
{
	LOG_LOG("Hello World!");

	defineZeroed(CreateThreadParams, params);
	
	params.entryFunc = otherThread;
	params.stackSize = 1024;
	params.cookie = "world";
	
	void* th1Stack;
	HANDLE th1 = app_createThread(&params, &th1Stack);
	
	int count = 0;
	while(true)
	{
		LOG_LOG("Main thread: %d", count++);
		app_sleep(4000);
		
		app_postMsg(th1, MSG_FIRST_CUSTOM, count, count+1);
		app_sleep(4000);
		break;
	}
	
	app_closeHandle(th1);
	free(th1Stack);

	return EXIT_SUCCESS;
}
