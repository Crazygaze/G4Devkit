/**
	Disk Drive functions test
**/


#include <stdlib.h>

#include "app_process.h" // Process API shared by both OS and application
#include "app_txtui.h" // Display shared by both OS and application
#include "app_diskdrive.h"

#include "file_system_provider.h"

#define printline(text) txtui_printAtXY(&rootCanvas, 0, text_offset_y++, text)
#define printfline(text, ...) txtui_printfAtXY(&rootCanvas, 0, text_offset_y++, text, __VA_ARGS__)

static int text_offset_y = 0;

int file_manager (int proc_num)
{
	//int res = make_file_system(0);
	printline("File Manager Initialization");
	
	for (int i = 0; i < 4; i++){
		if (is_disk_exist(i)){
			printfline ("Drive \'%d\' found. ", i);
		} else {
			printfline ("Drive \'%d\' not found. ", i);
		}
	}

	// Infinite loop to lock the application
	ThreadMsg msg;
	while(app_getMessage(&msg)) {
		app_sleep(250);
	}

	return EXIT_SUCCESS;
}