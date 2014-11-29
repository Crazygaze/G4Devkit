#ifndef _TEXT_EDIT_H_
#define _TEXT_EDIT_H_

#include "app_txtui.h"

#define TEXT_EDIT_MAX_LINES 200

typedef struct GraphTextEdit{
	int x, y;
	int width, height;
	
	int line_num;
	char * text[TEXT_EDIT_MAX_LINES];
	
	int cp_x, cp_y; // cursor position
	
	TxtColour color_back;
	TxtColour color_text;
	TxtColour color_cursor_back;
	TxtColour color_cursor_text;
} GraphTextEdit;

GraphTextEdit * create_textEdit(int x, int y, int widht, int height, 
				TxtColour color_back,
				TxtColour color_text,
				TxtColour color_cursor_back,
				TxtColour color_cursor_text);
void release_textEdit(GraphTextEdit * edit);

void setString_textEdit(GraphTextEdit * edit, const char * text);
void draw_textEdit(TxtCanvas * canvas, GraphTextEdit * edit);

void moveCursor_textEdit(TxtCanvas * canvas, GraphTextEdit * edit, int x, int y);

void putChar_textEditor(TxtCanvas * canvas, GraphTextEdit * edit, char ch);

#endif