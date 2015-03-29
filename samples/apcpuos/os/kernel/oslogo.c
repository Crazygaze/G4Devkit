#include "kernel/kernel.h"
#include "appsdk/kernel_shared/txtui_shared.h"
#include "oslogo.h"


/*
Gray background
		txtui_setColour(&rootCanvas, kTXTCLR_BLACK, kTXTCLR_GRAY);
		char 176
		
Yellow shadow
		txtui_setColour(&rootCanvas, kTXTCLR_BLACK, kTXTCLR_BRIGHT_YELLOW);
		char 176

Full yellow
		txtui_setColour(&rootCanvas, kTXTCLR_BLACK, kTXTCLR_BRIGHT_YELLOW);
		char 219

*/

#define G 0x0800 // Gray
#define Y 0x0E00 // Yellow

#define ii G|176
#define aa Y|176
#define C0 Y|219
#define C3 Y|220
#define C4 Y|223
#define C1 Y|178
#define C2 Y|177
#define U0 Y|'_'

const unsigned short oslogo[OSLOGO_WIDTH*OSLOGO_HEIGHT] =
{
//     A                   |   P                   |   C                   |  P                    |  U                                   O                       S
ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,
ii,ii,C0,C0,C0,C0,C0,C0,C0,ii,C0,C0,C0,C0,C0,C0,C0,ii,C0,C0,C0,C0,C0,C0,C0,ii,C0,C0,C0,C0,C0,C0,C0,aa,C0,C0,C0,ii,C0,C0,C0,ii,ii,ii,ii,ii,C0,C0,C0,C0,C0,C0,C0,ii,C0,C0,C0,C0,C0,C0,C0,ii,ii,ii,ii,
ii,ii,C1,C0,C0,ii,C0,C0,C0,aa,C1,C0,C0,ii,C0,C0,C0,aa,C1,C0,C0,aa,C0,C0,C0,aa,C1,C0,C0,aa,C0,C0,C0,aa,C1,C0,C0,aa,C0,C0,C0,aa,ii,ii,ii,ii,C1,C0,C0,aa,C0,C0,C0,aa,C1,C0,C0,aa,aa,aa,aa,aa,ii,ii,ii,
ii,ii,C1,C0,C0,C0,C0,C0,C0,aa,C1,C0,C0,C0,C0,C0,C0,aa,C1,C0,C0,aa,aa,aa,aa,aa,C1,C0,C0,C0,C0,C0,C0,aa,C1,C0,C0,aa,C0,C0,C0,aa,ii,ii,ii,ii,C1,C0,C0,aa,C0,C0,C0,aa,C2,C1,C0,C0,C0,C0,C0,aa,ii,ii,ii,
ii,ii,C2,C0,C0,aa,C0,C0,C0,aa,C2,C0,C0,aa,aa,aa,aa,aa,C2,C0,C0,aa,C0,C0,C0,aa,C2,C0,C0,aa,aa,aa,aa,aa,C2,C0,C0,aa,C0,C0,C0,aa,ii,ii,ii,ii,C2,C0,C0,aa,C0,C0,C0,aa,ii,aa,aa,aa,C0,C0,C0,aa,ii,ii,ii,
ii,ii,C2,C2,C1,aa,C2,C1,C0,aa,C2,C2,C1,aa,ii,ii,ii,ii,C2,C2,C1,C1,C0,C1,C0,aa,C2,C2,C1,aa,ii,ii,ii,ii,C2,C2,C1,C1,C0,C1,C0,aa,ii,ii,ii,ii,C2,C2,C1,C1,C0,C1,C0,aa,C2,C1,C0,C0,C0,C0,C0,aa,ii,ii,ii,
ii,ii,ii,aa,aa,aa,aa,aa,aa,aa,ii,aa,aa,aa,ii,ii,ii,ii,ii,aa,aa,aa,aa,aa,aa,aa,aa,aa,aa,aa,ii,ii,ii,ii,ii,aa,aa,aa,aa,aa,aa,aa,ii,ii,ii,ii,ii,aa,aa,aa,aa,aa,aa,aa,ii,aa,aa,aa,aa,aa,aa,aa,ii,ii,ii,
ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii,ii
};


void displayOSLogo(void)
{
	u16* ptr = rootCanvas.data;
	const u16* src = &oslogo[0];
	int offset = (rootCanvas.stride - OSLOGO_WIDTH)/2;
	for(int i=0;i<OSLOGO_WIDTH*OSLOGO_HEIGHT;i++) {
		ptr[(i/OSLOGO_WIDTH)*rootCanvas.stride + (i%OSLOGO_WIDTH) + offset] = *src++;
	}
	krn_spin(2000);
}