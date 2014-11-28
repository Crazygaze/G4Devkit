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
							
GraphWindow * create_window(const char * title, int x, int y, 
							int width, int height, 
							TxtColour color_frame, TxtColour color_back, 
							TxtColour color_text );

void release_window(GraphWindow * win);							
							
void draw_window(TxtCanvas * canvas, GraphWindow * win);

void clean_window(TxtCanvas * canvas, GraphWindow * win);

#endif