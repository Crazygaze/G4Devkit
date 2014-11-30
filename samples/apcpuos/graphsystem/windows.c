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

GraphWindow * window_create(const char * title, int x, int y, 
							int width, int height, 
							TxtColour color_frame, TxtColour color_back, 
							TxtColour color_text )
{
	GraphWindow * win = malloc (sizeof(GraphWindow));
	if (!win){
		return NULL;
	}
	
	memset(win, 0, sizeof(GraphWindow));	
	
	win->x = x;
	win->y = y;
	win->width = width;
	win->height = height;
	
	strcpy(win->title, title);
	
	win->color_back = color_back;
	win->color_frame = color_frame;
	win->color_text = color_text;

	return win;
}

void window_release(GraphWindow * win)
{
	free (win);
}

void window_draw(TxtCanvas * canvas, GraphWindow * win )
{
	// Frame
	txtui_setColour(&rootCanvas, win->color_frame, win->color_text);
	txtui_fillArea(&rootCanvas, win->x, win->y, win->width, win->height, ' ');
	
	// Title
	txtui_printAtXY(&rootCanvas, win->x + (win->width/2 - strlen(win->title)/2), win->y, win->title);
	
	// Back
	txtui_setColour(&rootCanvas, win->color_back, win->color_text);
	txtui_fillArea(&rootCanvas, win->x+1, win->y+1, win->width-2, win->height-2, ' ');
}

void window_clean(TxtCanvas * canvas, GraphWindow * win)
{
	txtui_setColour(&rootCanvas, win->color_back, win->color_text);
	txtui_fillArea(&rootCanvas, win->x+1, win->y+1, win->width-2, win->height-2, ' ');
}