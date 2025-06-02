#include <stdlib.h>
#include <appsdk/app_process.h>
#include <stdc_init.h>

#include "hwcrt0.h"

int helloworld_main(void *)
{
	LOG_LOG("Hello World!");
	
	static int count = 0;
	while(true)
	{
		//LOG_LOG("Helloworld: %u!", count++);
		app_sleep(1000);
	}
	
	return EXIT_SUCCESS;
}
