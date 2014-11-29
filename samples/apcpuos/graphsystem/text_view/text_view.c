#include "text_view.h"

#include <string.h>
#include <stdlib.h>

GraphTextView * create_textView(int x, int y, int widht, int height)
{
	GraphTextView * view = malloc(sizeof(GraphTextView));
	
	view->x = x;
	view->y = y;
	view->width = widht;
	view->height = height;
	
	return view;
}

void release_textView(GraphTextView * view)
{
	free(view->text);
	free(view);
}

void setString_textView(GraphTextView * view, const char * text)
{
	view->text = malloc(strlen(text));
	
	memcpy(view->text, text, strlen(text));
}

void draw_textView(TxtCanvas * canvas, GraphTextView * view)
{
	int column = view->x;
	int line = view->y;

	int cur_pos = 0;
	int nl_pos = find(view->text, '\n', cur_pos);

	if (nl_pos != -1){
		while (nl_pos != -1){
			char buf_line[256];
			memset(buf_line, 0, 256);
			memcpy(buf_line, &view->text[cur_pos], nl_pos - cur_pos);
		
			txtui_printAtXY(&rootCanvas, column, line, buf_line);
			
			cur_pos = nl_pos + 1;
			nl_pos = find(view->text, '\n', cur_pos);
			
			line++;
			if (line > view->height)
				break;
			
			column = view->x;
			
			if (nl_pos == -1){
				memset(buf_line, 0, 256);
				memcpy(buf_line, &view->text[cur_pos], strlen(view->text) - cur_pos + 1);
				txtui_printAtXY(&rootCanvas, column, line, buf_line);
				
				column += strlen(view->text) - cur_pos;
			} 
		}
	} else {
		txtui_printAtXY(&rootCanvas, column, line, view->text);
		column += strlen(view->text);
	}
}