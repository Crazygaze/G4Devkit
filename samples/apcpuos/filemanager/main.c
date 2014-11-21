/**
	Disk Drive functions test
**/


#include <stdlib.h>
#include <stdio.h>

#include "app_process.h" // Process API shared by both OS and application
#include "app_txtui.h" // Display shared by both OS and application
#include "app_diskdrive.h"

#include "file_system_provider.h"

#define printline(text) txtui_printAtXY(&rootCanvas, 0, text_offset_y++, text)
#define printfline(text, ...) txtui_printfAtXY(&rootCanvas, 0, text_offset_y++, text, __VA_ARGS__)

static int text_offset_y = 0;

int checkDrivesAndGetFirst(){
	int first_founded = -1;
	for (int i = 0; i < 4; i++){
		if (is_disk_exist(i)){
			printfline ("    Drive \'%d\' found. ", i);
			if (first_founded == -1)
				first_founded = i;
		} else {
			printfline ("    Drive \'%d\' not found. ", i);
		}
	}
	
	return first_founded;
}

#define MAX_COMMAND_LENGHT 60

static int mounted_drive = -1;
static bool cursor_blink = false;
static int cursor_pos = 1;
static char command[MAX_COMMAND_LENGHT];

bool initialization()
{
	printline("File Manager Initialization");
		
	int first_drive = checkDrivesAndGetFirst();
	
	printfline("Mounting \'%d\' drive...", first_drive);	
	if (change_drive(first_drive)){
		printline("    Done");	
		mounted_drive = first_drive;
	} else { 
		printline("    Failed");	
		return FALSE;
	}
	
	printline("Start the File Manager...");

	//app_sleep(1000);
	
	txtui_clear(&rootCanvas);
	
	text_offset_y = 0;
	
	return TRUE;
} 

void print_header()
{
	txtui_setColour(&rootCanvas, kTXTCLR_BRIGHT_BLUE, kTXTCLR_WHITE);
	txtui_printAtXY(&rootCanvas, 0, 0, "                              APCPU File Manager                                ");
}

void draw_window(const char * title, int x, int y, int width, int height)
{
	txtui_setColour(&rootCanvas, kTXTCLR_BRIGHT_YELLOW, kTXTCLR_WHITE);
	txtui_fillArea(&rootCanvas, x, y, width, height, ' ');
	txtui_printfAtXY(&rootCanvas, width/2 - 7, y, title);
	txtui_setColour(&rootCanvas, kTXTCLR_BLACK, kTXTCLR_WHITE);
	txtui_fillArea(&rootCanvas, x+1, y+1, width-2, height-2, ' ');
}

int file_manager (int proc_num)
{
	if ( initialization() == FALSE){
		printline("Drive mounting FAILED.");	
	} else {
		print_header();
		
		char drive_name[12];
		sprintf(drive_name, "Drive: \'%d\'", mounted_drive);
		draw_window(drive_name, 0, 1, rootCanvas.width/2, rootCanvas.height-3);
		
		draw_window("Command Window", 0, rootCanvas.height-2, rootCanvas.width, rootCanvas.height);
	}
	
	app_setTimer(1, 500, true);
	
	// Infinite loop to lock the application
	ThreadMsg msg;
	while(app_getMessage(&msg)) {
	
		if (msg.id>=MSG_KEY_PRESSED && msg.id<=MSG_KEY_TYPED) {
			bool ctrl = (msg.param2&KEY_FLAG_CTRL) ? TRUE : FALSE;
			bool shift = (msg.param2&KEY_FLAG_SHIFT) ? TRUE : FALSE;
		}

		switch(msg.id) {
			case MSG_KEY_PRESSED:
				
				if (msg.param1 == KEY_BACKSPACE && cursor_pos > 1) {
					command[cursor_pos-1] = 0;
					cursor_pos--;
				}
				
				if (msg.param1 >= KEY_ASCII_FIRST && msg.param1 <= KEY_ASCII_LAST){
					command[cursor_pos-1] = msg.param1;
					cursor_pos++;
				}				
						
				txtui_setColour(&rootCanvas, kTXTCLR_BLACK, kTXTCLR_WHITE);
				txtui_printAtXY(&rootCanvas, 1, rootCanvas.height-1, command);
				break;
			case MSG_QUIT:
				/* TODO: */
				break;
			
			case MSG_TIMER:
				cursor_blink = !cursor_blink;
				txtui_setColour(&rootCanvas, (cursor_blink == TRUE)?kTXTCLR_WHITE:kTXTCLR_BLACK, kTXTCLR_WHITE);
				txtui_printCharAtXY(&rootCanvas, cursor_pos, rootCanvas.height-1, ' ');
				// if backspaced
				txtui_setColour(&rootCanvas, kTXTCLR_BLACK, kTXTCLR_WHITE);
				txtui_printCharAtXY(&rootCanvas, cursor_pos+1, rootCanvas.height-1, ' ');
				break;
				
			case MSG_KEY_RELEASED:
				//sysstats_procesKeyRelease(&state, &msg);
				break;
			
			//app_sleep(250);
		}
	}
	return EXIT_SUCCESS;
}