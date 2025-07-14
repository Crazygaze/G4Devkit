#include "osapps.h"

int krn_idleTask(void* cookie);
int helloworld_main(void* cookie);

const KernelAppInfo krnApps[kKernelAppID_MAX] = 
{
	{"idle", krn_idleTask, true, 800, 0, 0},
	{"hellow", helloworld_main, false, 2048, 5, APPFLAG_WANTSKEYS},
};
