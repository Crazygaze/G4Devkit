/**
	Disk Drive functions test
**/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

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
#define MAX_PATH_LENGHT 128

static int mounted_drive = -1;
static bool cursor_blink = false;
static int cursor_pos = 1;
static char command[MAX_COMMAND_LENGHT];
static char path[MAX_PATH_LENGHT];

bool initialization()
{
	printline("File Manager Initialization");
		
	int first_drive = checkDrivesAndGetFirst();
	
	if (first_drive == -1){
		printline("Connected drives not found! Exit");	
		return FALSE;
	}
	
	printfline("Mounting \'%d\' drive...", first_drive);	
	if (change_drive(first_drive)){
		printline("    Done");	
		mounted_drive = first_drive;
	} else { 
		printline("    Failed");	
		return FALSE;
	}
	
	if (! is_file_system_exist()){
		printline("File System Not Found. Marking... This may take serveral minutes. ");
		if (! make_file_system(mounted_drive)){
			printline("    FAILED. Exit");
			return FALSE;
		}
		printline("    Done.");
	} else {
		printline("File System Found!");
	}
		
	printline("Start the File Manager...");

	app_sleep(1000);
	
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

void print_dir_entries(const char * path)
{
	txtui_fillArea(&rootCanvas, 1, 2, rootCanvas.width/2-2, rootCanvas.height-3-2, ' ');
	txtui_printAtXY(&rootCanvas, 1, 2, path);
	
	DIRECTORY * dir = opendir(path);
	
	int i = 0;
	FS_ITEM item;
	while (readdir(dir, &item)){
		txtui_printfAtXY(&rootCanvas, 2, 3+i, "%c %s", (item.type==T_FILE)?'f':'d', item.path);
		
		memset(&item, 0, sizeof(FS_ITEM));
		i++;
	}
	
	closedir(dir);
}

void printHelp()
{
	txtui_printAtXY(&rootCanvas, rootCanvas.width/2 + 18, 5, "HELP:");
	txtui_printAtXY(&rootCanvas, rootCanvas.width/2 + 3, 6, "MKDIR name - create new directory");
	txtui_printAtXY(&rootCanvas, rootCanvas.width/2 + 3, 7, "                  (8 symbols max)");
	txtui_printAtXY(&rootCanvas, rootCanvas.width/2 + 3, 8, "UNLINK name - remove dir or file ");
	txtui_printAtXY(&rootCanvas, rootCanvas.width/2 + 3, 9, "CD name - change dir             ");
	txtui_printAtXY(&rootCanvas, rootCanvas.width/2 + 3, 10, "UP - return to the previous dire");
	txtui_printAtXY(&rootCanvas, rootCanvas.width/2 + 3, 11, "                 ctory (like ..)");
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
		
		printHelp();
		
		print_dir_entries("/");
	
	
		app_setTimer(1, 500, true);
		
		// Infinite loop to lock the application
		ThreadMsg msg;
		while(app_getMessage(&msg)) {

			switch(msg.id) {
				case MSG_KEY_PRESSED:
					
					if (msg.param1 == KEY_RETURN){
						// command always have format "cmd agr" or "cmd"
						// command window not empty
						if (cursor_pos > 1){
							int space_pos = -1;
							char str_command[MAX_COMMAND_LENGHT];
							memset(str_command, 0, sizeof(char)*MAX_COMMAND_LENGHT);
							char str_argument[MAX_COMMAND_LENGHT];
							memset(str_argument, 0, sizeof(char)*MAX_COMMAND_LENGHT);
							for (int i = 0; i < strlen(command); i++){
								
								if (command[i] == ' '){
									space_pos = i;
									continue;
								}
								
								if (space_pos == -1){
									str_command[i] = command[i];
									str_command[i+1] = 0;
								} else {
									str_argument[i-space_pos-1] = command[i];
									str_argument[i-space_pos] = 0;
								}
							}
										
							// found valid command format "cmd arg"
							if (space_pos != -1){
								if (strcmp(str_command, "MKDIR") == 0){
									char tmp_buf[128];
									sprintf(tmp_buf, "%s/%s", path, str_argument);
									
									make_dir(tmp_buf);
									print_dir_entries(path);
								}
								
								if (strcmp(str_command, "UNLINK") == 0){
									char tmp_buf[128];
									sprintf(tmp_buf, "%s/%s", path, str_argument);
									
									unlink(tmp_buf);
									print_dir_entries(path);
								}
								
								if (strcmp(str_command, "CD") == 0){
									char tmp_buf[128];
									sprintf(tmp_buf, "%s/%s", path, str_argument);
									
									if (is_dir_exist(tmp_buf))
										sprintf(&path[strlen(path)], "/%s", str_argument);
									
									
									print_dir_entries(path);
								}
							} 
							
							// found valid command format "cmd"
							if (space_pos == -1 && strlen(str_command) > 0){
								if (strcmp(str_command, "UP") == 0){
							
									for (int i = strlen(path); i > 0; i--){
										if (path[i] != '/'){
											path[i] = 0;
										} else {
											path[i] = 0;
											LOG("path: %s", path);
											break;
										}
									}
									print_dir_entries(path);
								}
							}
							
							memset(command, 0, strlen(command));
							cursor_pos = 1;
							
						}else{	// enter to subdir
							
						}
					}
					
					if (msg.param1 == KEY_BACKSPACE && cursor_pos > 1) {
						command[strlen(command)-1] = 0;
						cursor_pos--;
					}
					
					if (msg.param1 >= KEY_ASCII_FIRST && msg.param1 <= KEY_ASCII_LAST){
						command[cursor_pos-1] = msg.param1;
						cursor_pos++;
					}				
							
					txtui_setColour(&rootCanvas, kTXTCLR_BLACK, kTXTCLR_WHITE);
					txtui_fillArea(&rootCanvas, 1, rootCanvas.height-1, rootCanvas.width-2, 1, ' ');
					txtui_printAtXY(&rootCanvas, 1, rootCanvas.height-1, command);
					break;
				case MSG_QUIT:
					
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
	}
	
	return EXIT_SUCCESS;
}