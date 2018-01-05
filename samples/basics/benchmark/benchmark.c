
#include "common.h"
#include "hwcommon.h"
#include "hwscreen.h"
#include "hwclock.h"
#include "hwcpu.h"
#include <stdlib.h>
#include <string.h>

//
// We can have up to 128 devices, but because proper support for that in this
// sample would require some kind of screen scrolling, we are keeping things
// simple by only trying to find a limited number of devices
#define MAX_DEVICES 20

int line=1;

void doWork()
{
	for(int x=0; x<(SCR_XRES-30); x++)
		for(int y=0; y<SCR_YRES; y++)
		{
			scr_printfAtXY(x,y," Testing %06d, %02d:%02d %3.3f ", x*y, x,y, (float)x/(y+1));
		}
}

void appMain(void)
{
	int x,y;
	scr_init();
	
	float f1;
	float f2;
	
	double startTime = clk_getRunningTimeSeconds();
	while(1)
	{
		doWork();
	}
	double endTime = clk_getRunningTimeSeconds();
	uint32_t cycles = cpu_getCycles32();
	
	scr_clear();
	scr_printfAtXY(0,0, "Duration: %3.4f", endTime-startTime);
	scr_printfAtXY(0,1, "Cycles: %u", cycles);
	scr_printfAtXY(0,2, "Mhgz  : %4.4f", ((double)cycles/(endTime-startTime))/(1000*1000));
		
	
	loopForever();
}
