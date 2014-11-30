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
	int l = 0;
	while(is_printable_char(str[l++])){}
	
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

void replace_cursor_line_on_screen(TxtCanvas * canvas, GraphTextEdit * edit, int cursor_move)
{
	txtui_setColour(canvas, edit->color_back, edit->color_text);
	
	// fill line before redraw 
	txtui_fillArea(canvas, edit->x, edit->y + edit->cp_y, edit->width, 1, ' ');
	
	txtui_printAtXY(canvas, edit->x, edit->y + edit->cp_y, edit->text[edit->cp_y]);
	
	textEdit_move_cursor(canvas, edit, edit->cp_x + cursor_move, edit->cp_y);
}

void textEdit_put_char(TxtCanvas * canvas, GraphTextEdit * edit, char ch)
{
	char * line = edit->text[edit->cp_y];
	
	int temp_lenght = strlen_printable(line);
	
	// save tail
	char * temp_str = malloc(temp_lenght - edit->cp_x);
	memcpy(temp_str, line + edit->cp_x, temp_lenght - edit->cp_x - 1);
	temp_str[temp_lenght - edit->cp_x - 1] = 0; 
	
	// change line lenght 
	line = realloc(edit->text[edit->cp_y], temp_lenght + 2);
	
	// put tail
	memcpy((line + edit->cp_x) + 1, temp_str, temp_lenght - edit->cp_x);
	line[temp_lenght + 1] = 0;
	free (temp_str);
	
	// put new char at cursor pos
	line[edit->cp_x] = ch;
	
	// if realloc give new pointer
	edit->text[edit->cp_y] = line;
	
	// replace line on screen
	replace_cursor_line_on_screen(canvas, edit, 1);
}

void textEdit_del_char(TxtCanvas * canvas, GraphTextEdit * edit)
{
	char * line = edit->text[edit->cp_y];
	
	int temp_lenght = strlen_printable(line);
	
	if (edit->cp_x > temp_lenght)
		return;
	
	// get next line as tail
	if (edit->cp_x == temp_lenght - 1){
		// get more space
		int temp_lenght_next = strlen_printable(edit->text[edit->cp_y + 1]);
		line = realloc(edit->text[edit->cp_y], temp_lenght + temp_lenght_next);
		
		// add tail
		memcpy(&line[temp_lenght-1], edit->text[edit->cp_y + 1], temp_lenght_next);
		line[temp_lenght + temp_lenght_next - 1] = 0; 
		edit->text[edit->cp_y] = line;
		
		// delete line
		for (int i = edit->cp_y + 1; i <= edit->line_num; i++){
			edit->text[i] = edit->text[i+1];
		}
		
		edit->line_num--;
		
		// redraw lines
		txtui_setColour(canvas, edit->color_back, edit->color_text);
		txtui_fillArea(canvas, edit->x, edit->y + edit->cp_y, edit->width, edit->height - edit->cp_y + 1, ' ');
		
		int lines_to_screen = (edit->line_num > edit->height) ? edit->height : edit->line_num;
		for (int i = edit->cp_y; i <= lines_to_screen; i++){
			txtui_printAtXY(canvas, edit->x, edit->y + i, edit->text[i]);
		}
		
		replace_cursor_line_on_screen(canvas, edit, 0);
		
	} else { // just del symbol
		memcpy(&line[edit->cp_x], &line[edit->cp_x] + 1, temp_lenght - edit->cp_x);
		line[temp_lenght] = 0;
		
		// replace line on screen
		txtui_setColour(canvas, edit->color_back, edit->color_text);
		txtui_printAtXY(canvas, edit->x, edit->y + edit->cp_y, edit->text[edit->cp_y]);
				
		// replace line on screen
		replace_cursor_line_on_screen(canvas, edit, 0);
	}
}

void textEdit_backspace_char(TxtCanvas * canvas, GraphTextEdit * edit)
{
	char * line = edit->text[edit->cp_y];
	
	int temp_lenght = strlen_printable(line);
	
	if (edit->cp_x > temp_lenght)
		return;
		
	if (edit->cp_x == 0 && edit->cp_y == 0)
		return;
	
	// backspace from start -> to next line as tail
	if (edit->cp_x == 0 && edit->cp_y > 0){
		// just up to prev line, and press 'del'
		textEdit_move_cursor(canvas, edit, strlen_printable(edit->text[edit->cp_y - 1]), edit->cp_y - 1);	
				
		textEdit_del_char(canvas, edit);
	} else { // just backspace symbol
		memcpy(&line[edit->cp_x - 1], &line[edit->cp_x], temp_lenght - edit->cp_x);
		line[temp_lenght] = 0;
		
		// replace line on screen
		txtui_setColour(canvas, edit->color_back, edit->color_text);
		txtui_printAtXY(canvas, edit->x, edit->y + edit->cp_y, edit->text[edit->cp_y]);
				
		// replace line on screen
		replace_cursor_line_on_screen(canvas, edit, -1);
	}
}

void textEdit_new_line(TxtCanvas * canvas, GraphTextEdit * edit)
{
	char * line = edit->text[edit->cp_y];
	
	int temp_lenght = strlen_printable(line);
	
	// save tail
	char * temp_str = malloc(temp_lenght - edit->cp_x);
	memcpy(temp_str, line + edit->cp_x, temp_lenght - edit->cp_x - 1);
	temp_str[temp_lenght - edit->cp_x - 1] = 0; 
	
	// cut tail
	memset(&line[edit->cp_x], 0, temp_lenght - edit->cp_x - 1);
		
	// shift all following lines
	for (int i = edit->line_num; i > edit->cp_y; i--){
		edit->text[i] = edit->text[i-1];
	}
	
	// put tail
	edit->text[edit->cp_y + 1] = temp_str;
	
	edit->line_num++;
	
	// redraw lines
	txtui_setColour(canvas, edit->color_back, edit->color_text);
	txtui_fillArea(canvas, edit->x, edit->y + edit->cp_y, edit->width,
		edit->height - edit->cp_y, ' ');
	
	int lines_to_screen = (edit->line_num > edit->height) ?
		edit->height : edit->line_num;
		
	for (int i = edit->cp_y; i <= lines_to_screen; i++){
		txtui_printAtXY(canvas, edit->x, edit->y + i, edit->text[i]);
	}
	
	// move cursor
	textEdit_move_cursor(canvas, edit, 0, edit->cp_y + 1);	
}

int textEdit_getLineNum(GraphTextEdit * edit)
{
	return edit->line_num;
}

char * textEdit_getLine(GraphTextEdit * edit, int line)
{
	if (line > edit->line_num)
		return NULL;
		
	char * res = malloc(strlen_printable(edit->text[line]) + 2);
	memcpy(res, edit->text[line], strlen_printable(edit->text[line]));
	res[strlen_printable(edit->text[line]) ] = '\n';
	res[strlen_printable(edit->text[line]) + 1] = 0;
	
	return res;
}