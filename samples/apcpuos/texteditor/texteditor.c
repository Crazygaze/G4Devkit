#include "texteditor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app_txtui.h"
#include "app_process.h"

#include "windows.h"
#include "text_view/text_view.h"
#include "text_edit/text_edit.h"
#include "file_system_provider.h"


int text_editor (int cookie)
{	
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

	GraphTextEdit * text_edit = textEdit_create(1, 2, rootCanvas.width - 2, rootCanvas.height - 2 - 2,
										kTXTCLR_BLACK, kTXTCLR_WHITE,
										kTXTCLR_WHITE, kTXTCLR_BLACK);					
	
	window_draw(&rootCanvas, win_editor);
	window_draw(&rootCanvas, win_command);

	txtui_printAtXY(&rootCanvas, 1, rootCanvas.height - 2, "Press BACKSPACE to exit");

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
			
				switch(msg.param1){
					case KEY_BACKSPACE:
						return EXIT_SUCCESS;
					
					// Navigation
					case KEY_RIGHT:
						textEdit_move_cursor(&rootCanvas, text_edit, text_edit->cp_x + 1, text_edit->cp_y);
						break;
										
					case KEY_LEFT:
						textEdit_move_cursor(&rootCanvas, text_edit, text_edit->cp_x - 1, text_edit->cp_y);
						break;
					
					case KEY_UP:
						textEdit_move_cursor(&rootCanvas, text_edit, text_edit->cp_x, text_edit->cp_y - 1);
						break;
										
					case KEY_DOWN:
						textEdit_move_cursor(&rootCanvas, text_edit, text_edit->cp_x, text_edit->cp_y + 1);
						break;
					
				}
				
				if (msg.param1 >= ' ' && msg.param1 <= '~')
					textEdit_put_char(&rootCanvas, text_edit, msg.param1);
				
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