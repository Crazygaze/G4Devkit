#include "text_edit.h"

#include <string.h>
#include <stdlib.h>

GraphTextEdit * textEdit_create(int x, int y, int widht, int height,
				TxtColour color_back,
				TxtColour color_text,
				TxtColour color_cursor_back,
				TxtColour color_cursor_text)
{
	GraphTextEdit * edit = malloc(sizeof(GraphTextEdit));
	
	edit->x = x;
	edit->y = y;
	edit->width = widht;
	edit->height = height;
	
	edit->cp_x = 0;
	edit->cp_y = 0;
	
	edit->color_back = color_back;
	edit->color_text = color_text;
	edit->color_cursor_back = color_cursor_back;
    edit->color_cursor_text = color_cursor_text; 
	
	return edit;
}

void textEdit_release(GraphTextEdit * edit)
{
	for (int i = 0; i < edit->line_num; i++)
		free(edit->text[i]);
		
	free(edit);
}

void textEdit_set_string(GraphTextEdit * edit, const char * text)
{		
	int column = 0;
	int line = 0;

	int cur_pos = 0;
	int nl_pos = find(text, '\n', cur_pos);

	if (nl_pos != -1){
		while (nl_pos != -1){
			edit->text[line] = malloc(nl_pos - cur_pos);
			memcpy(edit->text[line], &text[cur_pos], nl_pos - cur_pos);
			
			cur_pos = nl_pos + 1;
			nl_pos = find(text, '\n', cur_pos);
			
			line++;
			
			if (line > edit->height)
				break;
			
			column = edit->x;
			
			if (nl_pos == -1){
				edit->text[line] = malloc(strlen(text) - cur_pos + 1);
				memcpy(edit->text[line], &text[cur_pos], strlen(text) - cur_pos + 1);
			} 
		}
	} else {
		edit->text[line] = malloc(strlen(text) + 1);
		memcpy(edit->text[line], text, strlen(text));
		
		line++;
	}
	
	edit->line_num = line;
}

void textEdit_draw(TxtCanvas * canvas, GraphTextEdit * edit)
{
	txtui_setColour(canvas, edit->color_back, edit->color_text);
	for (int i = 0; i < edit->line_num; i++){
		txtui_printAtXY(canvas, edit->x, edit->y + i, edit->text[i]);
	}
	
	textEdit_move_cursor(canvas, edit, edit->cp_x, edit->cp_y);
}

bool is_printable_char(char ch)
{
	if (ch > 31 && ch < 127)	
		return TRUE;

	return FALSE;
}

int strlen_printable(char * str)
{
	int l = 0
	while(is_printable_char(str[l++])){};
	
	return l;
}

void textEdit_move_cursor(TxtCanvas * canvas, GraphTextEdit * edit, int x, int y)
{
	// checking input
	if (x < 0 || y < 0)
		return;
		
	if (y > TEXT_EDIT_MAX_LINES)
		return;
	
	if (edit->text[y] == NULL){
		x = 0;
	} else {
		if (strlen_printable(edit->text[y]) - 1 < x)
			x = strlen_printable(edit->text[y]) - 1;
	}

	// replace old symbol
	txtui_setColour(canvas, edit->color_back, edit->color_text);
	char old_ch = *(edit->text[edit->cp_y] + edit->cp_x);
	
	if (! is_printable_char(old_ch))
		old_ch = ' ';
	
	txtui_printCharAtXY(canvas, edit->x + edit->cp_x, edit->y + edit->cp_y, old_ch);
	
	// draw new cursor position
	txtui_setColour(canvas, edit->color_cursor_back, edit->color_cursor_text);
	char new_ch = *(edit->text[y] + x);
	
	if (! is_printable_char(new_ch))
		new_ch = ' ';
	
	txtui_printCharAtXY(canvas, edit->x + x, edit->y + y, new_ch);
		
	// update cursor position
	edit->cp_x = x;
	edit->cp_y = y;
}

void textEdit_put_char(TxtCanvas * canvas, GraphTextEdit * edit, char ch)
{
	char * line = edit->text[edit->cp_y];
	LOG("LINE_: %s", line);
	
	int temp_lenght = strlen_printable(line);
	
	char * temp_str = malloc(temp_lenght - edit->cp_x);
	memcpy(temp_str, line + edit->cp_x, temp_lenght - edit->cp_x - 1);
	temp_str[temp_lenght - edit->cp_x - 1] = 0; 
	LOG("TEMP_: %s", temp_str);
	
	line = realloc(edit->text[edit->cp_y], temp_lenght + 2);
	memcpy((line + edit->cp_x) + 1, temp_str, temp_lenght - edit->cp_x);
	line[temp_lenght + 1] = 0;
	free (temp_str);
	
	LOG("LINE2_: %s", line);
	
	line[edit->cp_x] = ch;
	
	LOG("LINE3_: %s", line);
	edit->text[edit->cp_y] = line;
	LOG("LINE4_: %s", edit->text[edit->cp_y]);
	LOG(" ");
	
	
	// replace line on screen
	txtui_setColour(canvas, edit->color_back, edit->color_text);
	txtui_printAtXY(canvas, edit->x, edit->y + edit->cp_y, edit->text[edit->cp_y]);
	
	textEdit_move_cursor(canvas, edit, edit->cp_x + 1, edit->cp_y);
}