#ifndef _WINDOWS_H_
#define _WINDOWS_H_

#include "app_txtui.h"

void print_header(TxtCanvas * canvas, const char * title, 
							TxtColour color_back, TxtColour color_text);
void draw_window(TxtCanvas * canvas, const char * title, int x, int y, 
							int width, int height, 
							TxtColour color_frame, TxtColour color_back, 
							TxtColour color_text );

#endif