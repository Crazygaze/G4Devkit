#include "hwkeyboard.h"

void kyb_pause(void)
{
	kyb_clearBuffer();
	kyb_getKey();
	kyb_clearBuffer();
}
