#include "appsdkconfig.h"
#include "txtui_shared.h"
#include "app_syscalls.h"
#include "app_process.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define DEFAULT_SCREEN_WIDTH 80
#define DEFAULT_SCREEN_HEIGHT 25

#define GET_XY_PTR(cv, x, y) ((cv)->data + (y)*(cv)->stride + (x))

TxtCanvas rootCanvas;

bool rectIntersect(Rect* dst, const Rect* a, const Rect* b)
{
	bool intersects =
		(a->x1 < b->x2) &&
		(a->x2 > b->x1) &&
		(a->y1 < b->y2) &&
		(a->y2 > b->y1);

	if (intersects) {
		dst->x1 = max(a->x1, b->x1);
		dst->y1 = max(a->y1, b->y1);
		dst->x2 = min(a->x2, b->x2);
		dst->y2 = min(a->y2, b->y2);
		return TRUE;
	} else {
		return FALSE;
	}
}

bool txtui_pointIntersect(const TxtCanvas* cv, int x, int y)
{
	return x>=0 && x<cv->width && y>=0 && y<cv->height;
}

bool txtui_rectIntersect(const TxtCanvas* cv, Rect* res, const Rect* a)
{
	bool intersects =
		(a->x1 < cv->width) &&
		(a->x2 > 0) &&
		(a->y1 < cv->height) &&
		(a->y2 > 0);

	if (intersects) {
		res->x1 = max(a->x1, 0);
		res->y1 = max(a->y1, 0);
		res->x2 = min(a->x2, cv->width);
		res->y2 = min(a->y2, cv->height);
		return TRUE;
	} else {
		return FALSE;
	}
}

void txtui_init(const TxtCanvas* existing)
{
	if (existing) {
		memcpy(&rootCanvas, existing, sizeof(rootCanvas));
		rootCanvas.currColour = kTXTCLR_WHITE<<8;		
		txtui_clear(&rootCanvas);		
	} else {
		//
		// Initialize main canvas
		rootCanvas.width = app_syscall0(kSysCall_GetScreenXRes);
		rootCanvas.height = app_syscall0(kSysCall_GetScreenYRes);
		rootCanvas.stride = rootCanvas.width;
		int size = rootCanvas.width*rootCanvas.height*2;

		// reserve the last line as a status bar by default
		if (app_info->prcInfo.flags & APPFLAG_WANTSSTATUSBAR) {
			rootCanvas.flags |= TXTCANVAS_STATUSBAR;
			rootCanvas.height--;
		}

		rootCanvas.data = malloc(size);
		always_assert(rootCanvas.data);

		// Clear the canvas before telling the OS where it is, otherwise it can
		// end up display garbage for a few frames
		txtui_setForegroundColour(&rootCanvas, kTXTCLR_WHITE);
		txtui_setBackgroundColour(&rootCanvas, kTXTCLR_BLACK);
		txtui_clear(&rootCanvas);		

		// Tell the OS we have a screen canvas
		bool res = app_syscall2(kSysCall_SetCanvas, (u32)rootCanvas.data, size );
		always_assert(res);
	
	}
	
}

bool txtui_createCanvas(TxtCanvas* cv, int width, int height)
{
	assert(cv);
	memset(cv, 0, sizeof(*cv));
	if (!(cv->data = calloc( width*height*2)))
		return FALSE;
	cv->width = width;
	cv->height = height;
	cv->stride = cv->width;
	cv->currColour = kTXTCLR_WHITE<<8;
	cv->flags = TXTCANVAS_OWNSMEM;
	txtui_clear(cv);
	return TRUE;
}

bool txtui_createSubCanvas(TxtCanvas* cv, const TxtCanvas* src, int x, int y,
	int width, int height)
{
	assert(cv);
	memset(cv, 0, sizeof(*cv));
	MAKERECT(tmp,x,y,x+width,y+height);
	Rect area;
	if (!txtui_rectIntersect(src, &area, &tmp))
		return FALSE;
	
	cv->data = GET_XY_PTR(src,area.x1,area.y1);
	cv->width = rectWidth(area);
	cv->height = rectHeight(area);
	cv->stride = src->stride;
	cv->currColour = kTXTCLR_WHITE<<8;
	cv->flags = 0;
	txtui_clear(cv);
	return TRUE;
}

void txtui_destroyCanvas(TxtCanvas* cv)
{
	assert(cv);
	if (cv->flags & TXTCANVAS_OWNSMEM) {
		free(cv->data);
	}
	memset(cv, 0, sizeof(*cv));
}

int txtui_getBuferSize(TxtCanvas* cv)
{
	assert(cv);
	return cv->width*cv->height*2;
}

static int clamp(int val, int left, int right)
{
	if (val<left)
		val = left;
	else if (val>right)
		val = right;
	return val;
}

static void txtui_copyCanvas(
	const TxtCanvas* from, const Rect* area,
	TxtCanvas* to, int toX, int toY)
{
	Rect src;
	// Validate source area
	if (!txtui_rectIntersect(from, &src, area))
		return;
	
	// Validate destination area
	if (!txtui_pointIntersect(to, toX, toY))
		return;
	
	int copyWidthBytes = rectWidth(src);
	copyWidthBytes = min(copyWidthBytes, to->width-toX);
	copyWidthBytes *= 2; // convert to bytes
	int copyHeight = rectHeight(src);
	copyHeight = min(copyHeight, to->height-toY);
	
	u16* srcPtr = GET_XY_PTR(from, src.x1, src.y1);
	u16* dstPtr = GET_XY_PTR(to, toX, toY);
	while(copyHeight--) {
		memcpy(dstPtr, srcPtr, copyWidthBytes);
		srcPtr += from->stride;
		dstPtr += to->stride;
	}
}

void txtui_blitCanvas(const TxtCanvas* cv, int toX, int toY)
{
	MAKERECT(src,0,0,cv->width,cv->height);
	txtui_copyCanvas(cv, &src, &rootCanvas, toX, toY);
}

void txtui_setForegroundColour(TxtCanvas* cv, TxtColour colour)
{
	assert(cv);
	cv->currColour = (cv->currColour&0xF000) | (colour<<8);
}

void txtui_setBackgroundColour(TxtCanvas* cv, TxtColour colour)
{
	assert(cv);
	cv->currColour = (cv->currColour&0x8F00) | (colour<<(8+4));
}

void txtui_setColour(TxtCanvas* cv, TxtColour bkgColour, TxtColour txtColour)
{
	assert(cv);
	cv->currColour = (bkgColour<<(8+4)) | (txtColour<<8);
}

TxtColour txtui_getForegroundColour(TxtCanvas* cv)
{
	assert(cv);
	return (cv->currColour>>8) & 0x0F;
}

TxtColour txt_getBackgroundColour(TxtCanvas* cv)
{
	assert(cv);
	return (cv->currColour>>(8+4)) & 0x7;
}

void txtui_clear(TxtCanvas* cv)
{
	assert(cv);
	txtui_clearWithColour(cv, (cv->currColour>>(8+4)) & 0x7);
}

void txtui_clearWithColour(TxtCanvas* cv, TxtColour bkgColour)
{
	assert(cv);
	// We print a space character (32) to clear the canvas
	u16 c = (bkgColour<<(8+4)) | 32;
	
	u16* ptr = cv->data;
	int todo = cv->height;
	int skipChars = cv->stride - cv->width;
	while(todo--) {
		int w= cv->width;
		while(w--)
			*ptr++ = c;
		ptr += skipChars;
	}
}

void txtui_fillArea(TxtCanvas* cv, int x, int y, int width, int height, char ch)
{
	assert(cv);
	Rect r;
	MAKERECT(area, x, y, x+width, y+height);
	if (!txtui_rectIntersect(cv, &r, &area))
		return;
	
	short finalch = cv->currColour | ch;
	int todoWidth = rectWidth(r);
	short *ptr = GET_XY_PTR(cv, r.x1, r.y1);
	int skipchars = cv->stride - todoWidth;
	int todo = rectHeight(r);
	while (todo--)
	{
		int w = todoWidth;
		while(w--)
			*ptr++ = finalch;
		ptr += skipchars;
	}
}

void txtui_fastPrintAtXY(TxtCanvas* cv, int x, int y, const char *str)
{
	assert(cv);
	if (!txtui_pointIntersect(cv,x,y))
		return;

	short* ptr = GET_XY_PTR(cv, x, y);
	int maxTodo = cv->width - x;
	while (*str && maxTodo--) {
		*ptr++ = (short)(cv->currColour | *str);
		str++;
	}
}

// TODO : Refactor this so that I don't need to use hardcoded width/height here
// This probably requires the kernel to have access to the TxtCanvas struct,
// or move this code to the kernel or screen driver, since the driver knows the
// resolution
void _txtui_setStatusBar(u16* canvasBufferStart, TxtColour txtClr,
	TxtColour bkgClr, const char* str)
{
	u16 colour = (bkgClr << (8+4)) | (txtClr<<8);
	u16* ptr = canvasBufferStart +
		(DEFAULT_SCREEN_HEIGHT-1)*DEFAULT_SCREEN_WIDTH;

	int maxTodo=DEFAULT_SCREEN_WIDTH;
	while(*str && maxTodo--) {
		*ptr++ = (short)(colour | *str++);
	}
	
	while(maxTodo--) {
		*ptr++ = (short)(colour | ' ');
	}
}

int txtui_printAtXY(TxtCanvas* cv, int x, int y, const char* str)
{
	int count=0;
	short* ptr = GET_XY_PTR(cv,x,y);
	while (*str) {
		if (*str>=32) {
			*ptr++ = (short)(cv->currColour | *str);
			count++;
		}

		str++;
	}
	return count;
}

void txtui_printfAtXY(TxtCanvas* cv, int x, int y, const char *fmt, ...)
{
	assert(cv);
	char buf[80];
	va_list ap;
	va_start(ap,fmt);
	vsprintf(buf, fmt, ap);
	txtui_fastPrintAtXY(cv, x,y,buf);
}

void txtui_printNumberAtXY(TxtCanvas* cv, int x, int y, int number)
{
	assert(cv);
	char buf[16];
	itoa(number, buf, 10);
	txtui_fastPrintAtXY(cv,x,y,buf);
}

void txtui_printFloatNumberAtXY(TxtCanvas* cv, int x, int y, float number)
{
	assert(cv);
	const char *s;
	int status=0;
	s = ftoa(number, &status);
	if (status==_FTOA_TOO_LARGE) {
		s = "1.#INF0000";
	} else if (status==_FTOA_TOO_SMALL) {
		s = "0.#DEN";
	}
	txtui_fastPrintAtXY(cv,x,y,s);
}

void txtui_printCharAtXY(TxtCanvas* cv, int x, int y, char ch)
{
	assert(cv);
	if (!txtui_pointIntersect(cv,x,y))
		return;
	short* ptr = GET_XY_PTR(cv,x,y);
	*ptr = (short)(cv->currColour | ch);
}
