#ifndef _WINDOWS_H_
#define _WINDOWS_H_

#include "app_txtui.h"

#define MAX_TITLE_LENGHT 128

typedef struct Window{
	int x, y;
	int width, height;
	
	char title[MAX_TITLE_LENGHT];
	
	TxtColour color_frame;
	TxtColour color_back;
	TxtColour color_text;
} Window;

void print_header(TxtCanvas * canvas, const char * title, 
							TxtColour color_back, TxtColour color_text);
							
Window * create_window(const char * title, int x, int y, 
							int width, int height, 
							TxtColour color_frame, TxtColour color_back, 
							TxtColour color_text );

void release_window(Window * win);							
							
void draw_window(TxtCanvas * canvas, Window * win);

void draw_button(TxtCanvas * canvas, const char * label, int x, int y,
					TxtColour color_back, TxtColour color_text);

#endif