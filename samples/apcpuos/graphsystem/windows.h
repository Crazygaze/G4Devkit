#ifndef _WINDOWS_H_
#define _WINDOWS_H_

#include "app_txtui.h"

#define MAX_TITLE_LENGHT 128

typedef struct GraphWindow{
	int x, y;
	int width, height;
	
	char title[MAX_TITLE_LENGHT];
	
	TxtColour color_frame;
	TxtColour color_back;
	TxtColour color_text;
} GraphWindow;

void print_header(TxtCanvas * canvas, const char * title, 
							TxtColour color_back, TxtColour color_text);
							
GraphWindow * window_create(const char * title, int x, int y, 
							int width, int height, 
							TxtColour color_frame, TxtColour color_back, 
							TxtColour color_text );

void window_release(GraphWindow * win);							
							
void window_draw(TxtCanvas * canvas, GraphWindow * win);

void window_clean(TxtCanvas * canvas, GraphWindow * win);

#endif