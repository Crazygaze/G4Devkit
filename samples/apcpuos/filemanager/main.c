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
	int res = make_file_system(0);
	
	

	/*txtui_clear(&rootCanvas);

	printline("File Manager Initialization...");
			
	FRESULT res_drive = app_dd_mount_drive("1");
	if (res_drive == FR_INVALID_DRIVE){
		printline ("    Drive not found.");
	} else {
		printline ("    Drive found.");
	}
	
	app_sleep(500);

	FIL * file_test = malloc(sizeof(FIL));
	FRESULT res_fsyst = app_dd_open_file(file_test, "NONEX.FILE", FA_READ);
	app_sleep(500);
	free(file_test);
	if (res_fsyst == FR_NO_FILESYSTEM){
		printline ("        File system not found.");
		printline ("            Mark up the file system. This may take several minutes... ");
		app_sleep(10000);
		FRESULT res_mkfs = app_dd_make_fat("1");
		app_sleep(500);
		if (res_mkfs != FR_OK){
			printline ("    FAILED.");
		} else {
			printline ("    Done.");
		}
	} else {
		printline ("		File system found!");
	}
	
	printline ("		Trying to find a file...");
	
	FIL * file = malloc(sizeof(FIL));
	FRESULT res_ftest = app_dd_open_file(file, "TEST.TXT", FA_READ);
	app_sleep(500);
	if (res_ftest == FR_OK){
		printline ("		    Found! Trying to read...");
		
		char buffer[64];
		int by_wr;
		
		app_dd_read_from_file(file, buffer, 64, &by_wr);
		app_sleep(500);
		printfline ("		        Done! message: %s", buffer);
		
		if (app_dd_close_file(file) != FR_OK){
			app_sleep(500);	
			printline ("		            FAILED.");
		} else {
			app_sleep(500);
			printline ("		            Done.");
		}
	} else {
		printline ("		    Not found. Trying to create...");
		FRESULT res_nf = app_dd_open_file(file, "TEST.TXT", FA_WRITE | FA_CREATE_ALWAYS);
		app_sleep(500);
		if (res_nf != FR_OK){
			printline ("		        FAILED.");
		} else {
			printline ("		        Done. Trying to write to a file...");
			
			int by_wr;
			FRESULT res_wr = app_dd_write_to_file(file, "Hello from DiskDrive!", strlen("Hello from DiskDrive!"), &by_wr);
			app_sleep(500);
			
			if (res_wr != FR_OK){
				printline ("		            FAILED.");
			} else {
				printline ("		            Done. Close the file");
				
				if (app_dd_close_file(file) != FR_OK){
					app_sleep(500);
					printline ("		                FAILED.");
				} else {
					app_sleep(500);
					printline ("		                Done.");
				}
			}
		}
	}
	
	
	free(file);
	app_sleep(500);
	*/

	// Infinite loop to lock the application
	ThreadMsg msg;
	while(app_getMessage(&msg)) {
		app_sleep(250);
	}

	return EXIT_SUCCESS;
}