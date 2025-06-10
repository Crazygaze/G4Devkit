#include <stdlib.h>
#include <stdc_init.h>
#include "hwcrt0.h"

int krn_idleTask(void* cookie)
{
	LOG_LOG("Idle entry");
	int tick = 0;
	
	while(true) {
		tick++;
		hwcpu_hlt();
	}
	
	return EXIT_SUCCESS;
}
