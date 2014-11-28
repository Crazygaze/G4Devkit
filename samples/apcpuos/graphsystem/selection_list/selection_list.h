#ifndef _SELECTION_LIST_H_
#define _SELECTION_LIST_H_

#include "app_txtui.h"

#define MAX_ELEMENT_LENGHT 15

typedef struct SelectionListElement{
	char path[MAX_ELEMENT_LENGHT];
	
	void * next;
	void * prev;
} SelectionListElement;

typedef struct GraphSelectionList{
	int size;
	
	int x, y;
	int width, height;
	
	SelectionListElement * first;
	SelectionListElement * last;
	
	TxtColour color_back;
	TxtColour color_selected;
	TxtColour color_text;
	TxtColour color_text_selected;
} GraphSelectionList;

GraphSelectionList * create_selectionList(int x, int y, int width, int height,
						TxtColour color_back, TxtColour color_selected, 
						TxtColour color_text, TxtColour color_text_selected);

void release_selectionList(GraphSelectionList * list);

void add_to_selectionList(GraphSelectionList * list, const char * path);
void clear_selectionList(GraphSelectionList * list);
void draw_selectionList(TxtCanvas * canvas, GraphSelectionList * list, 
				unsigned int selected_item);

#endif