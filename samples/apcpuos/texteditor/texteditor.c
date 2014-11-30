#include "texteditor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app_txtui.h"
#include "app_process.h"

#include "windows.h"
#include "text_view/text_view.h"
#include "text_edit/text_edit.h"
#include "selection_list/selection_list.h"
#include "file_system_provider.h"

int text_editor (int cookie)
{	
	bool menu_focus = FALSE;
	int menu_selected = 0;

	if (!change_drive(0)){
		return EXIT_FAILURE;
	}

	print_header(&rootCanvas, "APCPU Text Editor", kTXTCLR_BLUE, kTXTCLR_WHITE);
	
	char title_tmp[128];
	sprintf (title_tmp, "File: %s", prcArguments);
	
	GraphWindow * win_editor = window_create(title_tmp, 0, 1, rootCanvas.width, rootCanvas.height - 3,
						kTXTCLR_BRIGHT_GREEN, kTXTCLR_BLACK, kTXTCLR_BLACK);
	
	GraphWindow * win_command = window_create("Commands:", 0, rootCanvas.height - 3, rootCanvas.width, 3,	
						kTXTCLR_BLUE, kTXTCLR_BLACK, kTXTCLR_WHITE);

	GraphTextEdit * text_edit = textEdit_create(1, 2, rootCanvas.width - 2, rootCanvas.height - 2 - 4,
										kTXTCLR_BLACK, kTXTCLR_WHITE,
										kTXTCLR_WHITE, kTXTCLR_BLACK);					
	
	window_draw(&rootCanvas, win_editor);
	window_draw(&rootCanvas, win_command);

	txtui_printAtXY(&rootCanvas, 1, rootCanvas.height - 2, "Press TAB to exit");

	FILE * file = fopen(prcArguments, "r");
	if (file){
		int column = 0;
		int line = 0;
		
		txtui_setColour(&rootCanvas, kTXTCLR_BLACK, kTXTCLR_WHITE);
		
		int lenght = 0;
		char * text = NULL;
		
		char buf[205];
		memset(buf, 0, 205);
		
		while (fgets(buf, 200, file)){	
			int new_lenght = lenght + strlen(buf);
			
			text = realloc(text, new_lenght + 1);
			memset(&text[lenght], 0, strlen(buf));
			
			sprintf(&text[strlen(text)], "%s", buf);
			lenght = strlen(text);

			memset(buf, 0, 205);
		}
		
		textEdit_set_string(text_edit, text);
		textEdit_draw(&rootCanvas, text_edit);
		fclose(file);
	} else {
		LOG ("FILE NOT OPEND");
	}

	ThreadMsg msg;
	while(app_getMessage(&msg)) {
		switch(msg.id) {
			case MSG_KEY_TYPED:
			
				if (msg.param1 >= ' ' && msg.param1 <= '~'){
					textEdit_put_char(&rootCanvas, text_edit, msg.param1);
					break;
				}
			
				switch(msg.param1){
					// menu
					case KEY_TAB:
						menu_focus = TRUE;
						// create menu window
						GraphWindow * win_menu = window_create("MENU", 
											rootCanvas.width/2 - 10, 
											rootCanvas.height/2 - 3, 25, 5, 
											kTXTCLR_WHITE,
											kTXTCLR_BRIGHT_BLUE, 
											kTXTCLR_BLACK); 
						
						GraphSelectionList * menu_list = selectionList_create(win_menu->x + 1, 
													win_menu->y + 1, 
													win_menu->width - 2, 
													win_menu->height - 3,
													kTXTCLR_BRIGHT_BLUE, 
													kTXTCLR_WHITE, 
													kTXTCLR_WHITE,
													kTXTCLR_BLACK );	
						
						selectionList_add(menu_list, "Save And Exit");
						selectionList_add(menu_list, "Discard And Exit");
						selectionList_add(menu_list, "Cancel");
						
						window_draw(&rootCanvas, win_menu);
						selectionList_draw(&rootCanvas, menu_list, menu_selected);
						break;
						//return EXIT_SUCCESS;
						
					
					// Navigation
					case KEY_RIGHT:
						textEdit_move_cursor(&rootCanvas, text_edit, text_edit->cp_x + 1, text_edit->cp_y);
						break;
										
					case KEY_LEFT:
						textEdit_move_cursor(&rootCanvas, text_edit, text_edit->cp_x - 1, text_edit->cp_y);
						break;
					
					case KEY_UP:
						if (menu_focus){
							(menu_selected > 0)?menu_selected--:0;
							selectionList_draw(&rootCanvas, menu_list, menu_selected);
							break;
						}
						textEdit_move_cursor(&rootCanvas, text_edit, text_edit->cp_x, text_edit->cp_y - 1);
						break;
										
					case KEY_DOWN:
						if (menu_focus){
							(menu_selected < 2)?menu_selected++:2;
							selectionList_draw(&rootCanvas, menu_list, menu_selected);
							break;
						}
						textEdit_move_cursor(&rootCanvas, text_edit, text_edit->cp_x, text_edit->cp_y + 1);
						break;
					
					case KEY_DELETE:
						textEdit_del_char(&rootCanvas, text_edit);
						break;
					
					case KEY_BACKSPACE:
						textEdit_backspace_char(&rootCanvas, text_edit);
						break;
						
					case KEY_RETURN:
						if (menu_focus){
							switch (menu_selected){
								case 0:{	// save changes
									FILE * write_file = fopen(prcArguments, "w");
									
									// get text
									int linenum = textEdit_getLineNum(text_edit);
									for (int i = 0; i < linenum; i++){
										char * to_file = textEdit_getLine(text_edit, i);
										LOG ("TO_FILE: %s", to_file);
										
										fputs(to_file, write_file);
										
										free(to_file);
									}
									
									fclose(write_file);
									return EXIT_SUCCESS;
								}
								case 1:	// discard changes
									return EXIT_SUCCESS;
								case 2: // cancel
									menu_focus = FALSE;
									textEdit_draw(&rootCanvas, text_edit);
									break;
							}
							break;
						}
					
						textEdit_new_line(&rootCanvas, text_edit);
						break;
				}
				
				break;
			case MSG_QUIT:
				window_release(win_editor);
				window_release(win_command);
				textEdit_release(text_edit);
				break;
		}
	}

	return EXIT_SUCCESS;
}