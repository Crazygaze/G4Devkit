#include "texteditor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app_txtui.h"
#include "app_process.h"

#include "windows.h"
#include "file_system_provider.h"


int text_editor (int cookie)
{	
	if (!change_drive(0)){
		return EXIT_FAILURE;
	}

	print_header(&rootCanvas, "APCPU Text Viewer", kTXTCLR_BLUE, kTXTCLR_WHITE);
	
	char title_tmp[128];
	sprintf (title_tmp, "File: %s", prcArguments);
	
	GraphWindow * win_editor = create_window(title_tmp, 0, 1, rootCanvas.width, rootCanvas.height - 3,
						kTXTCLR_BRIGHT_GREEN, kTXTCLR_BLACK, kTXTCLR_BLACK);
	
	GraphWindow * win_command = create_window("Commands:", 0, rootCanvas.height - 3, rootCanvas.width, 3,	
						kTXTCLR_BLUE, kTXTCLR_BLACK, kTXTCLR_WHITE);
	
	draw_window(&rootCanvas, win_editor);
	
	
	draw_window(&rootCanvas, win_command);

	txtui_printAtXY(&rootCanvas, 1, rootCanvas.height - 2, "Press BACKSPASE to exit");

	LOG ("FILENAME: %s", prcArguments);

	FILE * file = fopen(prcArguments, "r");
	if (file){
		int column = 0;
		int line = 0;
		
		txtui_setColour(&rootCanvas, kTXTCLR_BLACK, kTXTCLR_WHITE);
		
		char ch = 0;
		while ((ch = fgetc(file)) != EOF){
			if (ch == '\n'){
				line++;
				column = 0;
				continue;
			} else {
				column++;
			}
			txtui_printCharAtXY(&rootCanvas, column+1, line+2, ch);
		}
		
		fclose(file);
	} else {
		LOG ("FILE NOT OPEND");
	}

	ThreadMsg msg;
	while(app_getMessage(&msg)) {
		switch(msg.id) {
			case MSG_KEY_PRESSED:
				if (msg.param1 == KEY_BACKSPACE)
					return EXIT_SUCCESS;
				break;
			case MSG_QUIT:
				release_window(win_editor);
				release_window(win_command);
				break;
		}
	}

	return EXIT_SUCCESS;
}