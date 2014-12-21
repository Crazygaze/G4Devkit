#include "hwclock.h"

void clk_pauseMS(int ms)
{
	double end = clk_getRunningTimeSeconds() + (double)ms/1000.0f;
	// Loop until enough time as passed
	while(end > clk_getRunningTimeSeconds()){
	}
}
