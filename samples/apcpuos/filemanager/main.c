/*
	Simple File Manager
*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

#include "texteditor.h"

#include "app_process.h" // Process API shared by both OS and application
#include "app_txtui.h" // Display shared by both OS and application
#include "app_diskdrive.h"

#include "windows.h"
#include "selection_list.h"
#include "text_view.h"

static int text_offset_y = 0;

#define printline(text) txtui_printAtXY(&rootCanvas, 0, text_offset_y++, text)
#define printfline(text, ...) txtui_printfAtXY(&rootCanvas, 0, text_offset_y++, text, __VA_ARGS__)

#define MAX_COMMAND_LENGHT 60
#define MAX_PATH_LENGHT 128

static int mounted_drive = -1;
static bool cursor_blink = false;
static int cursor_pos = 1;
static GraphWindow * win_drive;

static bool focus;

void printHelp()
{
	char help_text[400] = "           HELP:\n" \
		 "MKDIR name - create new directory\n" \
		 "                  (8 symbols max)\n" \
		 "UNLINK name - remove dir or file \n" \
		 "CD name - change dir             \n" \
		 "UP - return to the previous dire \n" \
		 "                 ctory (like ..) \n" \
		 "use ARROW KEYS to navigate in dir\n" \
		 "use ENTER and BACKSPACE to enter \n" \
		 "             and exit directories\n";

	
	GraphTextView * view = textView_create(rootCanvas.width/2 + 3, 5, 30, 25);
	textView_set_string(view, help_text);
	textView_draw(&rootCanvas, view);	
}

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
		
		
	FILE * file = fopen("EXAM.PLE", "w");
	if (file){
		fwrite("=================\n", 1, strlen("=================="), file);
		fwrite("Simple text file \n", 1, strlen("=================="), file);
		fwrite(" Just MORE text  \n", 1, strlen("=================="), file);
		fwrite("_=_=_=_=_=_=_=_=_\n", 1, strlen("=================="), file);
		fwrite("                 \n", 1, strlen("=================="), file);
		fwrite("      MEOOOOW    \n", 1, strlen("=================="), file);
		fwrite("                 \n", 1, strlen("=================="), file);
		fwrite("         /\\_/\\   \n", 1, strlen("=================="), file);
		fwrite("        ( o_o )  \n", 1, strlen("=================="), file);
		fwrite("         )   (   \n", 1, strlen("=================="), file);
		fwrite("        /    |   \n", 1, strlen("=================="), file);
		fwrite("       / || ||   \n", 1, strlen("=================="), file);
		fwrite("   ====__||_||   \n", 1, strlen("=================="), file);
		fwrite("=================\n", 1, strlen("=================="), file);
		fwrite("tested\n\nmore\nnew\nlines\n!", 1, strlen("testednmorennnewnlinesn!"), file);
		
		fclose(file);
	}
	
		
	printline("Start the File Manager...");

	app_sleep(1000);
	
	txtui_clear(&rootCanvas);
	
	text_offset_y = 0;
	
	return TRUE;
} 

static int items_on_screen = 0;
static int selected_index = 0;
static char selected_item[14];
static FS_ITEM items[32];
static GraphSelectionList * list_dir_entries;

void print_dir_entries_header(const char * path)
{
	txtui_fillArea(&rootCanvas, 1, 2, rootCanvas.width/2-2, rootCanvas.height-3-2, ' ');
	
	txtui_setColour(&rootCanvas, kTXTCLR_BRIGHT_GREEN, kTXTCLR_WHITE);
	txtui_fillArea(&rootCanvas, 1, 2, rootCanvas.width/2-2, 1, ' ');
	
	txtui_setColour(&rootCanvas, kTXTCLR_BRIGHT_GREEN, kTXTCLR_BLACK);
	txtui_printfAtXY(&rootCanvas, 1, 2, "Current: %s", path);
	
	txtui_setColour(&rootCanvas, kTXTCLR_BRIGHT_YELLOW, kTXTCLR_WHITE);
	txtui_fillArea(&rootCanvas, 1, 3, rootCanvas.width/2-2, 1, ' ');
	
	txtui_printfAtXY(&rootCanvas, 1, 3, "TYPE NAME     SIZE", path);
}

void update_filelist(const char * path)
{
	clear_selectionList(list_dir_entries);

	DIRECTORY * dir = opendir(path);
	
	if (dir){
		int i = 0;
		FS_ITEM item;
		while (readdir(dir, &item)){
			char spaces[10];
			memset(spaces, 0, 10);
			strncpy(spaces, "          ", 9-strlen(item.path));
			
			
			char line[40];
			memset(line, 0, 40);
			sprintf(line, "%c %s%s %d", ((item.type == T_DIR)?'d':'f'), item.path, spaces, item.size);
						
			add_to_selectionList(list_dir_entries, line);
			
			items[i++] = item;
		}
		items_on_screen = i;
		
		closedir(dir);
	}
}

/*
 * Parse command with format "com" or "cmd arg"
 *
 * returns 
 */
int parse_command(const char * command, char * fist, char * second)
{	
	int space_pos = -1;
	for (int i = 0; i < strlen(command); i++){
		
		// "cmd arg"
		if (command[i] == ' '){
			space_pos = i;
			
			memcpy(fist, command, (space_pos)*sizeof(char));
			memcpy(second, &command[space_pos + 1], strlen(command) - space_pos - 1);
					
			return 1;
		}	
	}
	
	// "cmd"
	memcpy(fist, command, strlen(command));
	return 0;
}

void upToParrentDir(char * path)
{
	selected_index = 0;

	if (strlen(path) > 1){
		for (int i = strlen(path); i >= 0; i--){
			if (path[i] == '/'){
				memset(&path[i], 0, strlen(path)-i);
				break;
			}
		}
		
		update_filelist(path);
		window_clean(&rootCanvas, win_drive);
		print_dir_entries_header(path);
		draw_selectionList(&rootCanvas, list_dir_entries, selected_index);
		
		sprintf(selected_item, "%s",items[selected_index].path);
	}
}

void changeDir(char * path, char * next_dir)
{
	char tmp_buf[128];
	sprintf(tmp_buf, "%s/%s", path, next_dir);
	
	if (is_dir_exist(tmp_buf)){
		sprintf(&path[strlen(path)], "/%s", next_dir);
	} else {
		return;
	}
		
	selected_index = 0;	
	
	update_filelist(path);
	window_clean(&rootCanvas, win_drive);
	print_dir_entries_header(path);
	draw_selectionList(&rootCanvas, list_dir_entries, selected_index);
	
	sprintf(selected_item, "%s",items[selected_index].path);
}

void callEditor(const char * path)
{
	ProcessCreateInfo app_info;
	strcpy(app_info.name, "TxtEditr");
	app_info.startFunc = text_editor;
	app_info.stacksize = 1024*4;
	app_info.memsize = 1024*64+4000;
	app_info.flags = APPFLAG_WANTSCANVAS | APPFLAG_WANTSKEYS | APPFLAG_WANTSSTATUSBAR;	
	
	int PID = app_createProcess(&app_info, path);
		
	if (PID){
		while (!app_setFocusTo( PID )){
			app_sleep(100);
		}
	}
}

int file_manager (int proc_num)
{
	static char path[MAX_PATH_LENGHT];
	char command[MAX_COMMAND_LENGHT];
	
	memset(path, 0, MAX_PATH_LENGHT*sizeof(char));
	memset(command, 0, MAX_PATH_LENGHT*sizeof(char));

	if ( initialization() == FALSE){
		printline("Drive mounting FAILED.");	
	} else {
		print_header(&rootCanvas, "APCPU File Manager", kTXTCLR_BRIGHT_BLUE, kTXTCLR_WHITE);
		
		char drive_name[12];
		sprintf(drive_name, "Drive: \'%d\'", mounted_drive);
		
		win_drive = window_create(drive_name, 0, 1, rootCanvas.width/2, rootCanvas.height-2, 
						kTXTCLR_BRIGHT_YELLOW, kTXTCLR_BLACK, kTXTCLR_BRIGHT_WHITE);
		
		GraphWindow * win_command = window_create("Commands:", 0, rootCanvas.height-2, rootCanvas.width, 3, 
						kTXTCLR_BRIGHT_YELLOW, kTXTCLR_BLACK, kTXTCLR_BRIGHT_WHITE);
		
		window_draw(&rootCanvas, win_drive);			
		
		window_draw(&rootCanvas, win_command);	
		
		printHelp();
		
		list_dir_entries = create_selectionList(win_drive->x + 3, win_drive->y + 3, 
							win_drive->width - 2, win_drive->height - 5,
							kTXTCLR_BLACK, kTXTCLR_BRIGHT_WHITE,
							kTXTCLR_WHITE, kTXTCLR_BLACK);
				
		update_filelist("/");
		
		print_dir_entries_header(path);
		draw_selectionList(&rootCanvas, list_dir_entries, selected_index);
		sprintf(selected_item, "%s",items[selected_index].path);
		
		app_setTimer(1, 500, true);
				
		// Infinite loop to lock the application
		ThreadMsg msg;
		while(app_getMessage(&msg)) {

			switch(msg.id) {
				case MSG_KEY_PRESSED:
					if (focus == FALSE)
						break;
					
					if (msg.param1 == KEY_RETURN){
						// command always have format "cmd agr" or "cmd"
						// command window not empty
						if (cursor_pos > 1){
							char str_command[MAX_COMMAND_LENGHT];
							char str_argument[MAX_COMMAND_LENGHT];
							
							memset(str_command,  0, sizeof(char)*MAX_COMMAND_LENGHT);
							memset(str_argument, 0, sizeof(char)*MAX_COMMAND_LENGHT);
							int args_num = parse_command(command, str_command, str_argument);
										
							// found valid command format "cmd arg"
							if (args_num == 1){
								if (strcmp(str_command, "MKDIR") == 0){
									char tmp_buf[128];
									sprintf(tmp_buf, "%s/%s", path, str_argument);
									
									make_dir(tmp_buf);
									update_filelist(path);
									draw_selectionList(&rootCanvas, list_dir_entries, selected_index);
								}
								
								if (strcmp(str_command, "UNLINK") == 0){
									char tmp_buf[128];
									sprintf(tmp_buf, "%s/%s", path, str_argument);
									
									unlink(tmp_buf);
									update_filelist(path);
									draw_selectionList(&rootCanvas, list_dir_entries, selected_index);
								}
								
								if (strcmp(str_command, "CD") == 0){
									changeDir(path, str_argument);
								}
							} 
							
							// found valid command format "cmd"
							if (args_num == 0){
								if (strcmp(str_command, "UP") == 0){
									upToParrentDir(path);
								}
							}
							
							memset(command, 0, strlen(command));
							cursor_pos = 1;
							
						}else{	// enter to subdir
							if (items[selected_index].type == T_DIR){
								changeDir(path, selected_item);
							} else {
														
								// try to call another app
								char * file_name_arg = malloc (128 * sizeof(char));
								sprintf(file_name_arg, "%s/%s", path, selected_item);							
								
								callEditor(file_name_arg);
							}
						}
					}
					
					if (msg.param1 == KEY_BACKSPACE){
						if (cursor_pos > 1) {		
							command[strlen(command)-1] = 0;
							cursor_pos--;
						} else {	// Up to parent dir
							upToParrentDir(path);
						}
					}
					
					if (msg.param1 == KEY_UP){
						if (selected_index > 0)
							selected_index--;
						
						sprintf(selected_item, "%s",items[selected_index].path);
						
						draw_selectionList(&rootCanvas, list_dir_entries, selected_index);
					}
					
					if (msg.param1 == KEY_DOWN){
						if (selected_index < items_on_screen - 1)
							selected_index++;
						
						sprintf(selected_item, "%s",items[selected_index].path);
						
						draw_selectionList(&rootCanvas, list_dir_entries, selected_index);
					}					
				
					
					if ((msg.param1 >= '0' && msg.param1 <= '9') 
						|| (msg.param1 >= 'a' && msg.param1 <= 'z')
						|| (msg.param1 >= 'A' && msg.param1 <= 'Z')
						|| (msg.param1 == ' ')){		
						command[cursor_pos-1] = msg.param1;
						cursor_pos++;
					}				
							
					txtui_setColour(&rootCanvas, kTXTCLR_BLACK, kTXTCLR_WHITE);
					txtui_fillArea(&rootCanvas, 1, rootCanvas.height-1, rootCanvas.width-2, 1, ' ');
					txtui_printAtXY(&rootCanvas, 1, rootCanvas.height-1, command);
					break;
				case MSG_QUIT:
					release_selectionList(list_dir_entries);
					window_release(win_drive);
					window_release(win_command);
					break;
				
				case MSG_TIMER:
					cursor_blink = !cursor_blink;
					txtui_setColour(&rootCanvas, (cursor_blink == TRUE)?kTXTCLR_WHITE:kTXTCLR_BLACK, kTXTCLR_WHITE);
					txtui_printCharAtXY(&rootCanvas, cursor_pos, rootCanvas.height-1, ' ');
					// if backspaced
					txtui_setColour(&rootCanvas, kTXTCLR_BLACK, kTXTCLR_WHITE);
					txtui_printCharAtXY(&rootCanvas, cursor_pos+1, rootCanvas.height-1, ' ');
					break;
					
				case MSG_FOCUSGAINED:
					focus = TRUE;
					break;
					
				case MSG_FOCUSLOST:
					focus = FALSE;
					break;
					
				case MSG_KEY_RELEASED:
					//sysstats_procesKeyRelease(&state, &msg);
					break;
			}
		}
	}
	
	return EXIT_SUCCESS;
}