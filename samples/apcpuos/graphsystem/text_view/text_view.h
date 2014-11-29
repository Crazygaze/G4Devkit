#ifndef _TEXT_VIEW_H_
#define _TEXT_VIEW_H_

#include "app_txtui.h"

typedef struct GraphTextView{
	int x, y;
	int width, height;
	
	char * text;
} GraphTextView;

GraphTextView * create_textView(int x, int y, int widht, int height);
void release_textView(GraphTextView * view);

void setString_textView(GraphTextView * view, const char * text);
void draw_textView(TxtCanvas * canvas, GraphTextView * view);

#endif