#include "appslist.h"
#include "kernel/kerneldebug.h"


#define NUM_APPS 3

int testapp1(int p1);
int testapp2(int p1);
int krn_idleTask(int p1);
int sysstats_main(int p1);

int file_manager(int p1);

static AppInfo apps[NUM_APPS] =
{
	{ "idle", krn_idleTask, TRUE, 800, 0, APPFLAG_NOFOCUS, 0},
	{ "sysstats", sysstats_main, FALSE, 800, 1024+4000*2, APPFLAG_WANTSKEYS|APPFLAG_WANTSCANVAS|APPFLAG_WANTSSTATUSBAR, 0},
	/*{ "testapp1", testapp1, FALSE, 800, 2048+4000, APPFLAG_WANTSCANVAS|APPFLAG_WANTSSTATUSBAR, 1},
	{ "testapp2", testapp2, FALSE, 800, 1024+4000, APPFLAG_WANTSCANVAS|APPFLAG_WANTSSTATUSBAR, 2},
	{ "testapp3", testapp2, FALSE, 800, 1024+4000, APPFLAG_WANTSCANVAS|APPFLAG_WANTSSTATUSBAR, 3},
	{ "testapp4", testapp2, FALSE, 800, 1024+4000, APPFLAG_WANTSCANVAS|APPFLAG_WANTSSTATUSBAR, 4},
	{ "testapp5", testapp2, FALSE, 800, 1024+4000, APPFLAG_WANTSCANVAS|APPFLAG_WANTSSTATUSBAR, 5},*/
	{ "file_manager", file_manager, FALSE, 11800, 1024+4000*2, APPFLAG_WANTSCANVAS|APPFLAG_WANTSSTATUSBAR, 6}
};

int os_getNumApps(void)
{
	return NUM_APPS;
}

AppInfo* os_getAppInfo(uint32_t appnumber)
{
	kernel_assert(appnumber<NUM_APPS);
	return &apps[appnumber];
}
