#include "osapps.h"

int krn_idleTask(void* cookie);
int helloworld_main(void* cookie);

const KernelAppInfo krnApps[kKernelAppID_MAX] = 
{
	{"idle", krn_idleTask, true, 800, 0, 0},
	{"helloworld", helloworld_main, false, 800, 4096, 0},
};
