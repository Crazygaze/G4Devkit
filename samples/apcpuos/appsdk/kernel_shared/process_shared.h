#ifndef _process_shared_h_
#define _process_shared_h_

#include <stddef_shared.h>

#define PRC_NAME_SIZE 9

#define TIMER_MAX_INTERVAL_MASK ((1<<30)-1)
#define TIMER_MAX_INTERVAL TIMER_MAX_INTERVAL_MASK

/*! Miscellaneous process stats
*/
typedef struct ProcessInfo
{
	// process id
	uint8_t pid;

	// Process name
	char name[PRC_NAME_SIZE];
		
	// Percentage of the cpu used by the process
	// This include the time spent in SWI calls
	uint8_t cpu;
	// Percentage of cpu used serving swi calls
	uint8_t cpuswi;
	
	// Total memory reserved for this process
	// This is based on the total number of pages reserved for the process
	// Actual used memory by the process if calculated by the process itself,
	// by inspecting its pool
	int memory;

	// Copied from the applist startup list
	uint32_t flags;
	
	// copied from the applist startup list
	uint32_t cookie;
	
	void* heap_start;
	uint32_t heap_size;
	
} ProcessInfo;

/*! Miscellaneous OS and kernel stats
*/
typedef struct OSInfo
{
	//
	// Memory stats
	//
	int mem_available;
	
	//
	// Cpu stats
	//
	int cpu_usage;
	
	//
	// Kernel stats
	//
	int krn_mem_used;
	int krn_mem_free;
	int krn_mem_maxAlloc;
	
	// id of the process with focus
	uint8_t focusedPid;
	
} OSInfo;

typedef struct ThreadMsg
{
	uint32_t id;
	uint32_t param1;
	uint32_t param2;
} ThreadMsg;

//
// Messages
// 
#define MSG_QUIT 0
#define MSG_KEY_PRESSED 1
#define MSG_KEY_RELEASED 2
#define MSG_KEY_TYPED 3
#define MSG_TIMER 4
#define MSG_FOCUSGAINED 5
#define MSG_FOCUSLOST 6
#define MSG_FIRSTUSER 7

/******************************************************************************/
//
// Keyboard
//
/******************************************************************************/
//
// Keys
//
#define KEY_BACKSPACE 0x01
#define KEY_RETURN 0x02
#define KEY_INSERT 0x03
#define KEY_DELETE 0x04
#define KEY_UP 0x05
#define KEY_DOWN 0x06
#define KEY_LEFT 0x07
#define KEY_RIGHT 0x08
#define KEY_SHIFT 0x09
#define KEY_CONTROL 0x0A
#define KEY_TAB 0x0B
/*
 * These are the printable characters as defined in
 * http://en.wikipedia.org/wiki/ASCII ,
 * in the table "ASCII printable characters"
 */
#define KEY_ASCII_FIRST 0x20  // decimal 32 (space)
#define KEY_ASCII_LAST 0x7E

#define KEY_FLAG_CTRL (1<<0)
#define KEY_FLAG_SHIFT (1<<1)

#define APPFLAG_WANTSKEYS (1<<0)
#define APPFLAG_WANTSCANVAS (1<<1)
#define APPFLAG_WANTSSTATUSBAR (1<<2)
#define APPFLAG_NOFOCUS (1<<3)

//! Mem stats will be calculated
#define OSINFO_KERNELMEM (1<<0)
//! Cpu usage stats will be calculated
#define OSINFO_CPU (1<<1)



#endif

