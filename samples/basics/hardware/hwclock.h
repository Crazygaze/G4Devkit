#ifndef _hwclock_h_
#define _hwclock_h_

/*! Gets the number of seconds the system as been running since boot
*/
double clk_getRunningTimeSeconds(void);

/*! Pauses the program for the specified milliseconds
*/
void clk_pauseMS(int ms);

#endif
