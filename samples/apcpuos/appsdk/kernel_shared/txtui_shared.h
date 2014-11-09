/*!
This file contains code to manipulate text mode UI.

*/

#ifndef _txtui_shared_h_
#define _txtui_shared_h_

#include <stdint_shared.h>
#include <stddef_shared.h>
#include <stdlib_shared.h>

typedef struct Point
{
	int x;
	int y;
} Point;

/*! Represents a rectangle
x1,x2 is the top left corner.
x2,y2 is the bottom right corner, and it's EXCLUSIVE.
For example, a rectangle to cover a 25x80 screen is defined as:
{0,0},{25,80}, and NOT {0,0},{24,79}
*/
typedef struct Rect
{
	int x1;
	int y1;
	int x2;
	int y2;
} Rect;

#define SETRECT(var, x1_, y1_, x2_, y2_) \
	var.x1 = x1_;                        \
	var.y1 = y1_;                        \
	var.x2 = x2_;                        \
	var.y2 = y2_;

#define MAKERECT(var, x1_, y1_, x2_, y2_) \
	Rect var;                             \
	SETRECT(var, x1_, y1_, x2_, y2_);

/*! Calculates a rectangle width */
#define rectWidth(rect) ((rect).x2 - (rect).x1)

/*! Calculates a rectangle height */
#define rectHeight(rect) ((rect).y2 - (rect).y1)

/*! Checks if two rectangles intersect
* \param dst
*	Rectangle resulting from the intersection
* \return
*	True if intersected. In this case, "dst" contains the resulting rectangle
*	False if no intersection
*/
bool rectIntersect(Rect* dst, const Rect* a, const Rect* b);

/*! The canvas owns the memory */
#define TXTCANVAS_OWNSMEM (1<<0)
#define TXTCANVAS_PRINTNONVISIBLE (1<<1)
#define TXTCANVAS_WRAP (1<<2)
#define TXTCANVAS_STATUSBAR (1<<3)

typedef struct TxtCanvas
{
	u16* data;
	int stride;
	int width;
	int height;
	u16 currColour;
	u8 flags;
} TxtCanvas;

extern TxtCanvas rootCanvas;

typedef enum TxtColour {
	/*
	Colours that can be used for background and foreground
	*/
	kTXTCLR_BLACK = 0,
	kTXTCLR_BLUE,
	kTXTCLR_GREEN,
	kTXTCLR_CYAN,
	kTXTCLR_RED,
	kTXTCLR_MAGENTA,
	kTXTCLR_BROWN,
	kTXTCLR_WHITE,
	
	/*
	Colours that can only be used for foreground
	*/
	kTXTCLR_GRAY,
	kTXTCLR_BRIGHT_BLUE,
	kTXTCLR_BRIGHT_GREEN,
	kTXTCLR_BRIGHT_CYAN,
	kTXTCLR_BRIGHT_RED,
	kTXTCLR_BRIGHT_MAGENTA,
	kTXTCLR_BRIGHT_YELLOW,
	kTXTCLR_BRIGHT_WHITE
} TxtColour;

// Should not be used by the user code
// It's used only at the application startup
void txtui_init(const TxtCanvas* existing);

bool txtui_createCanvas(TxtCanvas* cv, int width, int height);
bool txtui_createSubCanvas(TxtCanvas* cv, const TxtCanvas* src, int x, int y,
	int width, int height);
void txtui_destroyCanvas(TxtCanvas* cv);

/*! Returns the canvas buffer size, in bytes
Note that this doesn't take into account the stride.
It's simply the number of bytes for the canvas area
*/
int txtui_getBuferSize(TxtCanvas* cv);

void txtui_blitCanvas(const TxtCanvas* cv, int toX, int toY);
void txtui_setForegroundColour(TxtCanvas* cv, TxtColour colour);
void txtui_setBackgroundColour(TxtCanvas* cv, TxtColour colour);
void txtui_setColour(TxtCanvas* cv, TxtColour bkgColour, TxtColour txtColour);
TxtColour txtui_getForegroundColour(TxtCanvas* cv);
TxtColour txtui_getBackgroundColour(TxtCanvas* cv);


/*! Clears the canvas using the current canvas colour
*/
void txtui_clear(TxtCanvas* cv);

/*! Clears the canvas, but allows specifying the colour
* \note The specified colour is used only for clearing the canvas. The current
* canvas colour is not changed.
*/
void txtui_clearWithColour(TxtCanvas* cv, TxtColour bkgColour);

void txtui_fillArea(TxtCanvas* cv, int x, int y, int width, int height, char ch);
int  txtui_printAtXY(TxtCanvas* cv, int x, int y, const char* str);
void txtui_fastPrintAtXY(TxtCanvas* cv, int x, int y, const char *str);
void txtui_printfAtXY(TxtCanvas* cv, int x, int y, const char *fmt, ...);
void txtui_printNumberAtXY(TxtCanvas* cv, int x, int y, int number);
void txtui_printFloatNumberAtXY(TxtCanvas* cv, int x, int y, float number);
void txtui_printCharAtXY(TxtCanvas* cv, int x, int y, char ch);

/*! Used internally to update the status bar.
No need to call it directly
\param canvasBufferStart Where the canvas buffer starts.
	The OS works out where the status line is given the start of the buffer
*/
void _txtui_setStatusBar(u16* canvasBufferStart, TxtColour txtClr,
	TxtColour bkgClr, const char* str);

#endif
