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

Window * create_window(const char * title, int x, int y, 
							int width, int height, 
							TxtColour color_frame, TxtColour color_back, 
							TxtColour color_text )
{
	Window * win = malloc (sizeof(Window));
	if (!win){
		LOG("MALLOC FAILED");
		return NULL;
	}
	
	memset(win, 0, sizeof(Window));	
	
	win->x = x;
	win->y = y;
	win->width = width;
	win->height = height;
	
	strcpy(win->title, title);
	
	win->color_back = color_back;
	win->color_frame = color_frame;
	win->color_text = color_text;
	
	LOG("WINDOW: %s", win->title);
	
	return win;
}

void release_window(Window * win)
{
	free (win);
}

void draw_window(TxtCanvas * canvas, Window * win )
{
	LOG("WINDOW: %s", win->title);

	// Frame
	txtui_setColour(&rootCanvas, win->color_frame, win->color_text);
	txtui_fillArea(&rootCanvas, win->x, win->y, win->width, win->height, ' ');
	
	// Title
	txtui_printAtXY(&rootCanvas, win->x + (win->width/2 - strlen(win->title)/2), win->y, win->title);
	
	// Back
	txtui_setColour(&rootCanvas, win->color_back, win->color_text);
	txtui_fillArea(&rootCanvas, win->x+1, win->y+1, win->width-2, win->height-2, ' ');
}

void draw_button(TxtCanvas * canvas, const char * label, int x, int y,
					TxtColour color_back, TxtColour color_text)
{
	int width = strlen(label) + 2;

	txtui_setColour(&rootCanvas, color_back, color_text);
	txtui_fillArea(&rootCanvas, x, y, width, 1, ' ');
	txtui_printfAtXY(&rootCanvas, x + width/2 - strlen(label)/2, y, label);
}