#include "windows.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_header(TxtCanvas * canvas, const char * title, 
							TxtColour color_back, TxtColour color_text)
{
	int size = (canvas->width) * sizeof(char);
	char * header_title = malloc(size);
	memset(header_title, ' ', size);
	memcpy(&header_title[canvas->width/2 - strlen(title)/2], title, strlen(title));

	txtui_setColour(&rootCanvas, color_back, color_text);
	txtui_printAtXY(&rootCanvas, 0, 0, header_title);
}

void draw_window(TxtCanvas * canvas, const char * title, int x, int y, 
					int width, int height, 
					TxtColour color_frame, TxtColour color_back, 
					TxtColour color_text )
{
	txtui_setColour(&rootCanvas, color_frame, color_text);
	txtui_fillArea(&rootCanvas, x, y, width, height, ' ');
	txtui_printfAtXY(&rootCanvas, width/2 - strlen(title)/2, y, title);
	txtui_setColour(&rootCanvas, color_back, color_text);
	txtui_fillArea(&rootCanvas, x+1, y+1, width-2, height-2, ' ');
}
