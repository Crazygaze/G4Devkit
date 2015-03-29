#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "hwclock.h"
#include "hwcpu.h"

void pause(int ms)
{
	double now = clk_getRunningTimeSeconds();
	double finalTime = now + (double)ms/1000;
	
	do {
		now = clk_getRunningTimeSeconds();
	} while(now<finalTime);
}

void loopForever(void)
{
	while(1) {
		cpu_halt();
	}
}