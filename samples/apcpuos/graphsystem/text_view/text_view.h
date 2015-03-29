#ifndef _TEXT_VIEW_H_
#define _TEXT_VIEW_H_

#include "app_txtui.h"

typedef struct GraphTextView{
	int x, y;
	int width, height;
	
	char * text;
} GraphTextView;

GraphTextView * textView_create(int x, int y, int widht, int height);
void textView_release(GraphTextView * view);

void textView_set_string(GraphTextView * view, const char * text);
void textView_draw(TxtCanvas * canvas, GraphTextView * view);

#endif