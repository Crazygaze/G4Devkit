#include "texteditor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app_txtui.h"
#include "app_process.h"

#include "windows.h"
#include "text_view/text_view.h"
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

	GraphTextView * text_view = create_textView(1, 2, rootCanvas.width - 2, rootCanvas.height - 2 - 2);
	
	draw_window(&rootCanvas, win_editor);
	draw_window(&rootCanvas, win_command);

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
		
		setString_textView(text_view, text);
		draw_textView(&rootCanvas, text_view);
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