#ifndef _SELECTION_LIST_H_
#define _SELECTION_LIST_H_

#include "app_txtui.h"

#define MAX_ELEMENT_LENGHT 20

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

GraphSelectionList * selectionList_create(int x, int y, int width, int height,
						TxtColour color_back, TxtColour color_selected, 
						TxtColour color_text, TxtColour color_text_selected);

void selectionList_release(GraphSelectionList * list);

void selectionList_add(GraphSelectionList * list, const char * path);
void selectionList_clear(GraphSelectionList * list);
void selectionList_draw(TxtCanvas * canvas, GraphSelectionList * list, 
				unsigned int selected_item);

#endif