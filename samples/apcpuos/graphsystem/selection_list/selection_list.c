#include "selection_list.h"

#include <stdlib.h>
#include <string.h>

GraphSelectionList * create_selectionList(int x, int y, int width, int height,
						TxtColour color_back, TxtColour color_selected, 
						TxtColour color_text, TxtColour color_text_selected)
{
	GraphSelectionList * list = malloc(sizeof(GraphSelectionList));
	memset(list, 0, sizeof(GraphSelectionList));
	list->x = x;
	list->y = y;
	
	list->width = width;
	list->height = height;
	
	list->first = NULL;
	list->last = NULL;
	
	list->color_back = color_back;
	list->color_selected = color_selected;
	list->color_text = color_text;
	list->color_text_selected = color_text_selected;
	
	return list;
}

void clear_selectionList(GraphSelectionList * list)
{
	SelectionListElement * item = list->first;

	while (item){
		SelectionListElement * next = item->next;
		free(item);
		
		item = next;
	}
	
	list->first = list->last = NULL;
	list->size = 0;
}

void release_selectionList(GraphSelectionList * list)
{
	SelectionListElement * item = list->first;

	clear_selectionList(list);
	
	free(list);
}

void add_to_selectionList(GraphSelectionList * list, const char * path)
{
	if (list->first == NULL){
		list->first = malloc(sizeof(SelectionListElement));
		memset(list->first, 0, sizeof(SelectionListElement));
		
		list->first->next = NULL;
		list->first->prev = NULL;
		
		strcpy(list->first->path, path);
		list->last = list->first;
		
		list->size = 1;
	} else {
		SelectionListElement * item = malloc(sizeof(SelectionListElement));
		memset(item, 0, sizeof(SelectionListElement));
		
		item->next = NULL;
		item->prev = list->last;
		
		strcpy(item->path, path);
		
		list->last->next = item;		
		list->last = item;
		
		list->size++;
	}
}

void draw_selectionList(TxtCanvas * canvas, GraphSelectionList * list, unsigned int selected_item)
{
	int ypos = 0;

	SelectionListElement * item = list->first;

	while (item){
		if (ypos != selected_item){
			txtui_setColour(canvas, list->color_back, list->color_text);
			txtui_printAtXY(canvas, list->x, list->y + ypos, item->path);
		} else {
			txtui_setColour(canvas, list->color_selected, list->color_text_selected);
			txtui_printAtXY(canvas, list->x, list->y + ypos, item->path);
		}
		
		ypos++;
		item = item->next;
	}
}