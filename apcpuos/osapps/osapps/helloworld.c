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
	
	app_setTimer(10000, true, cookie);
	
	while(app_getMsg(&msg))
	{
		if (msg.id == MSG_TIMER)  {
			LOG_LOG("Other thread: %u. msgId=%u, param1=%s", 
				count, msg.id, (const char*)msg.param1);
		} else {
			LOG_LOG("Other thread: %u. msgId=%u, param1=%u, param2=%u", count,
				msg.id, msg.param1, msg.param2);
		}
		count++;
	}
	
	LOG_LOG("Quitting other thread");
}

int helloworld_main(void *)
{
	u32 stack = app_calcUsedStack(app_getCurrentThread());
	LOG_LOG("Hello World!. Stack = %u", stack);
	
	defineZeroed(CreateThreadParams, params);
	
	params.entryFunc = otherThread;
	params.stackSize = 1024;
	params.cookie = "world";
	
	void* th1Stack;
	HANDLE th1 = app_createThread(&params, &th1Stack);
	
	int count = 0;
	ThreadMsg msg;
	while(app_getMsg(&msg))
	{
		LOG_LOG("Main thread: %d", count++);
		
		if (msg.id >= MSG_KEY_PRESSED && msg.id <= MSG_KEY_TYPED) {
			LOG_LOG("msgId=%u, key=%u('%c'), mods=%u",
				msg.id,
				msg.param1,
				msg.param1 >= KEY_ASCII_FIRST ? (char)msg.param1 : ' ',
				msg.param2);
		}
		
		app_postMsg(th1, MSG_FIRST_CUSTOM, count, count+1);
		app_postMsg(th1, MSG_QUIT, count, count+1);
		break;
	}
	
	app_closeHandle(th1);
	free(th1Stack);

	return EXIT_SUCCESS;
}



