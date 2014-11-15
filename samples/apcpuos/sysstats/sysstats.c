#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "app_process.h"
#include "app_sysinfo.h"
#include "app_txtui.h"

typedef enum SysStatsMode {
	kSysStats_None,
	kSysStats_CpuOnly,
	kSysStats_CpuDetailed,
	kSysStats_ProcessPages,
	kSysStats_KernelMem,
	kSysStats_MAX
} SysStatsMode;

static char getRotatingChar(void)
{
	static char busy[4] = {'-' , '\\' , '|' , '/' };
	static int busycounter=0;
	return busy[(busycounter++)&0x3];
}

#define STATS_WIDTH 17
#define STATS_HEIGHT 25


typedef struct SysStatsState
{
	// Process that had focus before
	uint8_t previousFocusPid;
	int prcInfoArraySize;
	ProcessInfo* prcInfo;
	int prcCount;
	OSInfo osInfo;
	SysStatsMode mode;
	TxtCanvas composite;
	TxtCanvas statsCv;
	bool hasFocus;
} SysStatsState;

static void sysstats_startFrame(SysStatsState* st)
{
	if (st->hasFocus) {
		int bytes = app_processScreenshot(st->previousFocusPid,
			st->composite.data, txtui_getBuferSize(&st->composite));	
		if (bytes==0)
			txtui_clearWithColour(&st->composite, kTXTCLR_GREEN);
	}
}

static void sysstats_endFrame(SysStatsState* st)
{
	if (st->hasFocus) {
		txtui_blitCanvas(&st->composite,0,0);
	}
}

static void sysstats_update(SysStatsState* st, uint32_t flags)
{
	// Update data
	//
	st->prcCount = sysinfo_getProcessCount();
	if (st->prcCount > st->prcInfoArraySize) {
		st->prcInfo = realloc(st->prcInfo, sizeof(ProcessInfo)*st->prcCount);
		assert(st->prcInfo);
		st->prcInfoArraySize = st->prcCount;
	}

	st->prcCount = sysinfo_getOSInfo(&st->osInfo, st->prcInfo,
		st->prcInfoArraySize, flags);
}

/*
display some system stats
*/
static void sysstats_display(SysStatsState* st)
{	
	static char* modes[kSysStats_MAX] =
		{"","","Cpu usage","Process Pages", "Kernel Mem"};
	static int extralines[kSysStats_MAX] = {0,1,3,3};

	TxtCanvas* cv = &st->statsCv;

	// If we are not required to show anything, then no point in collecting
	// stats, so clear the canvas and exit
	if (st->mode==kSysStats_None) {
		txtui_clearWithColour(cv, kTXTCLR_BLACK);
		return;
	}	

	if (st->mode==kSysStats_CpuOnly) {
		txtui_printfAtXY(cv, cv->width-4, 0, "%3u%%", st->osInfo.cpu_usage);	
		return;
	}

	int y=0;	
	txtui_fillArea(cv, 0, 0, cv->width,	st->prcCount + extralines[st->mode],
		' ');
	txtui_printfAtXY(cv, 0, y++, "%c %s", getRotatingChar(), modes[st->mode]);
	
	if (st->mode==kSysStats_CpuDetailed)
	{
		txtui_printfAtXY(cv, cv->width-4, 0, "%3u%%", st->osInfo.cpu_usage);
		for(int i=0; i<st->prcCount; i++) {
			ProcessInfo* it = &st->prcInfo[i];
			txtui_printfAtXY(cv,0,y++, "%8s:%3u%%(%2u)", it->name,
				it->cpu, it->cpuswi);
		}
	} else if (st->mode==kSysStats_ProcessPages) {

		for(int i=0; i<st->prcCount; i++) {
			ProcessInfo* it = &st->prcInfo[i];
			txtui_printfAtXY(cv,0,y++, "%8s:%4uk", it->name,
				it->memory/1024);
		}

		txtui_printfAtXY(cv,0,y++, "%8s:%4uk", "Free",
			st->osInfo.mem_available/1024);
	} else if (st->mode==kSysStats_KernelMem) {
		txtui_printfAtXY(cv,0,y++, "   Used :%3.1fk",
			(float)st->osInfo.krn_mem_used/1024);
		txtui_printfAtXY(cv,0,y++, "   Free :%3.1fk",
			(float)st->osInfo.krn_mem_free/1024);
		txtui_printfAtXY(cv,0,y++, "maxAlloc:%3.1fk",
			(float)st->osInfo.krn_mem_maxAlloc/1024);
	}

}

void sysstats_updateStatusBar(SysStatsState* st)
{
	// Get the process with focus
	ProcessInfo* focusedPrc=NULL;
	for(int i=0; i<st->prcCount; i++) {
		if (st->prcInfo[i].pid==st->osInfo.focusedPid) {
			focusedPrc = &st->prcInfo[i];
			break;
		}		
	}

	if (!focusedPrc)
		return;

	char buf[81];
	sprintf(buf, "%8s:%3u%%(%2u):%3u%%", focusedPrc->name, (u32)focusedPrc->cpu,
		(u32)focusedPrc->cpuswi, (u32)st->osInfo.cpu_usage);		
	app_setStatusBar(buf, strlen(buf));
}

static void sysstats_changeProcess(SysStatsState* st, int offset)
{
	// update our stuff, so we know what processes are running at the
	// moment
	sysstats_update(st, 0);
	
	int index = 0;
	for(int i=0; i<st->prcCount; i++) {
		if (st->prcInfo[i].pid==st->osInfo.focusedPid) {
			index = i;
			break;
		}
	}
	
	do {
		index = index+offset;			
		if (index<0)
			index = st->prcCount-1;
		else if (index>=st->prcCount)
			index = 0;
		LOG("Trying to switch to %d",st->prcInfo[index].pid);
	} while(st->prcInfo[index].pid==app_info->prcInfo.pid ||
				!app_setFocusTo(st->prcInfo[index].pid));
				
}

static void sysstats_procesKeyRelease(SysStatsState* st, ThreadMsg* msg)
{
	bool hasCtrl = msg->param2&KEY_FLAG_CTRL;
	bool hasShift = msg->param2&KEY_FLAG_SHIFT;
	
	if (hasCtrl && hasShift) {
		//
		// Control actions
		
		switch(msg->param1)
		{
			case 'S':
			{
				if (!st->hasFocus) {
					sysstats_update(st, 0);
					// Remember the process that had focus, so we can
					// capture its screen
					st->previousFocusPid = st->osInfo.focusedPid;
					app_focus();
				}

				st->mode = (st->mode+1) % kSysStats_MAX;
				LOG("MODE CHANGED to %d", st->mode);
				sysstats_startFrame(st);
				txtui_clearWithColour(&st->statsCv, kTXTCLR_BLACK);
				if (st->hasFocus)
					sysstats_display(st);
				sysstats_endFrame(st);
			}
		break;
		
		// Set focus to next application
		case 'N':
			LOG("Next...");
			sysstats_changeProcess(st, 1);
		break;
		
		// Set focus to previous application
		case 'P':
			LOG("Previous");
			sysstats_changeProcess(st, -1);
		break;
		}
			
	} else if (!(hasCtrl || hasShift) && st->hasFocus) {
		if (msg->param1==KEY_BACKSPACE) {
			st->mode = kSysStats_None;
			app_setFocusTo(st->previousFocusPid);
		}
	}
}

static void sysstats_processTimer(SysStatsState* st, ThreadMsg* msg)
{
	uint32_t flags=0;
	if (
		st->mode==kSysStats_None ||
		st->mode==kSysStats_CpuOnly ||
		st->mode==kSysStats_CpuDetailed
		) {
		flags |= OSINFO_CPU;
	}
	
	if (st->mode==kSysStats_KernelMem) {
		flags |= OSINFO_KERNELMEM;
	}

	sysstats_update(st, flags);
	sysstats_startFrame(st);
	if (st->hasFocus)
		sysstats_display(st);
	sysstats_updateStatusBar(st);
	sysstats_endFrame(st);
}

int sysstats_main(int p1)
{
	LOG("HELLO WORLD");
	
	SysStatsState state;
	memset(&state, 0, sizeof(state));
	state.mode = kSysStats_None;
	
	bool res = txtui_createCanvas(&state.composite, rootCanvas.width,
		rootCanvas.height);
	assert(res);
	
	res = txtui_createSubCanvas(&state.statsCv, &state.composite,
		state.composite.width-STATS_WIDTH, 0, STATS_WIDTH, STATS_HEIGHT);
	assert(res);

	txtui_setColour(&state.statsCv,kTXTCLR_BLUE, kTXTCLR_BRIGHT_WHITE);
		
	app_setTimer(1, 1000, true);
	ThreadMsg msg;

	while (app_getMessage(&msg))
	{
		if (msg.id>=MSG_KEY_PRESSED && msg.id<=MSG_KEY_TYPED) {
			bool ctrl = (msg.param2&KEY_FLAG_CTRL) ? TRUE : FALSE;
			bool shift = (msg.param2&KEY_FLAG_SHIFT) ? TRUE : FALSE;
		}

		switch(msg.id) {
		case MSG_QUIT:
			goto out;
		
		case MSG_TIMER:
			sysstats_processTimer(&state, &msg);
			break;
			
		case MSG_KEY_RELEASED:
			sysstats_procesKeyRelease(&state, &msg);
			break;
		
		case MSG_FOCUSGAINED:
			LOG("Focus gained");
			state.hasFocus = true;
			break;
			
		case MSG_FOCUSLOST:
			LOG("Focus lost");
			state.hasFocus = false;
			break;
			
		default:
		
		};
		
	}

out:
	txtui_destroyCanvas(&state.statsCv);
	txtui_destroyCanvas(&state.composite);
	return EXIT_SUCCESS;
}