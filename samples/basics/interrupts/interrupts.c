
#include "common/common.h"

/*!
This struct matches a register set.
Having a struct like this makes it easier to use from C, instead of just a
simple pointer/array
*/
typedef struct Ctx
{
	// Fields that must match the architecture register set
	int gregs[16];
	int flags;
	double fregs[16];
} Ctx;

// This will be used as the application execution context
Ctx appCtx;

// The assembly interrupt handler sets this whenever an interrupt happens
Ctx* interruptedCtx;

// The assembly interrupt handler sets this whenever an IRQ interrupt happens
u32 interruptBus;
u32 interruptReason;

// Counts the total number of interrupts
int interruptsCount;

//
// Whenever there is a system call, we set this variable with what the system
// call returned. It shows how a system call can pass a result back to the
// application
int lastSystemCallResult;

void launchApplication(void);
void redrawScreen(int doClear);
void showMenu(void);


/*!
Setups the application execution context.
This is used at startup, and whenever an unrecoverable interrupt occurs (in this
sample) to reset the application.
*/
void setupAppCtx(void)
{
	// We use this small memory block as a stack for the application context
	#define APPSTACKSIZE 4096
	static char appStack[APPSTACKSIZE];

	memset(&appCtx, 0, sizeof(appCtx));
	// Setup the stack (register SP)
	// Note that the stack grows downwards, so we point SP to the top address of
	// the memory block we are using for the stack	
	appCtx.gregs[13] = (int) &appStack[APPSTACKSIZE];
	// Setup the PC register
	appCtx.gregs[15] = (int) &launchApplication;
	// Setup the flags register
	// The value specified (0x04000000), sets Supervisor mode, and enables
	// IRQs	
	appCtx.flags = 0x04000000;	
}

/*******************************************************************************
*		Utility functions to print things to the screen
*******************************************************************************/

void printStringData(int x, int y, const char* name, const char* data)
{
	printString(x,y,name);
	printString(x+strlen(name), y, data);
}

void printNumberData(int x, int y, const char* name, int number, int base)
{
	printString(x,y,name);
	x += strlen(name);
	if (base==16) {
		printString(x,y, "0x");
		x +=2;
	}
	printNumber(x, y, number, base);
}

void printInterruptDetails(
	Ctx* interruptedCtx, const char* title,
	u32 data0, u32 data1, u32 data2, u32 data3)
{
	redrawScreen(TRUE);
	
	int x = 4;
	int y = 1;
	printStringData(x,++y, "Interrupt type: ", title);
	printNumberData(x+40,y, "Bus ", interruptBus, 10);
	printNumberData(x+50,y, "Reason ", interruptReason, 10);
	printNumberData(x,++y, "Num interrupts: ", interruptsCount, 10);
	
	int yy = y;
	printNumberData(x,++y, "Interrupt ctx r0: ", data0, 16);
	printNumberData(x,++y, "Interrupt ctx r1: ", data1, 16);
	printNumberData(x,++y, "Interrupt ctx r2: ", data2, 16);
	printNumberData(x,++y, "Interrupt ctx r3: ", data3, 16);
	
	y = yy;
	x = 40;
	printNumberData(x,++y, "App ctx r0: ", interruptedCtx->gregs[0], 16);
	printNumberData(x,++y, "App ctx r1: ", interruptedCtx->gregs[1], 16);
	printNumberData(x,++y, "App ctx r2: ", interruptedCtx->gregs[2], 16);
	printNumberData(x,++y, "App ctx r3: ", interruptedCtx->gregs[3], 16);
	printNumberData(x,++y, "Last SWI call result: ", lastSystemCallResult, 10);
}

/*******************************************************************************
*		Interrupt handlers
*******************************************************************************/

// Interrupts for the CPU
#define CPU_INTERRUPT_ABORT 0
#define CPU_INTERRUPT_DIVIDEBYZERO 1
#define CPU_INTERRUPT_UNDEFINEINSTRUCTION 2
#define CPU_INTERRUPT_ILLEGALINSTRUCTION 3
#define CPU_INTERRUPT_SWI 4
#define CPU_INTERRUPT_MAX 5

// Interrupts for the Clock
#define CLOCK_INTERRUPT_MAX 1

// How many drivers we support in the sample
#define NUM_DRIVERS 2

typedef void (*InterruptHandler)(u32 data0, u32 data1, u32 data2, u32 data3);
typedef struct Driver
{
	InterruptHandler* handlers;
	int numHanders;
} Driver;


Ctx* handleReset(void)
{
	initCommon();
	setupAppCtx();
	return &appCtx;
}

void cpu_handleGeneric(u32 data0, u32 data1, u32 data2, u32 data3)
{
	static const char* reasons[CPU_INTERRUPT_MAX] =
	{
		"ABORT",
		"DIVIDE BY ZERO",
		"UNDEFINED INSTRUCTION",
		"ILLEGAL INSTRUCTION",
		"SWI"
	};

	printInterruptDetails(interruptedCtx, reasons[interruptReason], data0,
			data1, data2, data3);
	
	if (interruptReason!=CPU_INTERRUPT_SWI) {
		// Anything other than a SWI interrupt is an unrecoverable interrupt
		// in this sample, so lets reset the application.
		setupAppCtx();
	} else {
		// If it's a system call, pass a return value back to the application
		interruptedCtx->gregs[0] = ++lastSystemCallResult;	
	}
}

void clock_handleTimer(u32 data0, u32 data1, u32 data2, u32 data3)
{
	interruptsCount++;
	printInterruptDetails(interruptedCtx, "IRQ",data0,data1,data2,data3);
}

InterruptHandler cpu_handlers[] =
{
	&cpu_handleGeneric,
	&cpu_handleGeneric,
	&cpu_handleGeneric,
	&cpu_handleGeneric,
	&cpu_handleGeneric
};
InterruptHandler clock_handlers[] =
{
	&clock_handleTimer
};

//
// Put all the interrupt handlers together
Driver drivers[NUM_DRIVERS] =
{
	{ cpu_handlers, CPU_INTERRUPT_MAX },
	{ clock_handlers, CLOCK_INTERRUPT_MAX }
};

Ctx* handleInterrupt(u32 data0, u32 data1, u32 data2, u32 data3)
{
	interruptsCount++;	
	drivers[interruptBus].handlers[interruptReason](data0, data1, data2, data3);	
	return &appCtx;
}

void showMenu(void)
{
	int x = 4;
	int y = 12;
	printString(x, y++, "1. Test ABORT (Execute)");
	printString(x, y++, "2. Test ABORT (Write)");
	printString(x, y++, "3. Test ABORT (Read)");
	printString(x, y++, "4. Test DIVIDE BY ZERO");
	printString(x, y++, "5. Test UNDEFINED INSTRUCTION");
	printString(x, y++, "6. Test ILLEGAL INSTRUCTION");
	printString(x, y++, "7. Test SWI (System Call) (will pass 0xf00d,0xbeef,0x0,0x0 to the handler)");
	printString(x, y++, "8. Test IRQ (triggers a one-off timer)");
}

void redrawScreen(int doClear)
{
	if (doClear) {
		clearScreen();
	}
	printString(0,0,
		"Interrupts example: Make sure you disconnect the debugger");
	showMenu();
}


/*
Causes an Abort due to an invalid execute permissions
This one is implemented in assembly, so I can easily change the PC register
to point to an invalid address
*/
void causeAbortExecute(void);
// Defined in the assembly file.
void causeUndefinedInstruction(void);
// Defined in the assembly file.
void causeIllegalInstruction(void);
// Defined in the assembly file.
int causeSystemCall(void);
// Defined in the assembly file.
void causeIRQ(void);


// Used just for forcing a divide by zero without the compiler detect it
static int zero = 0;
static int dummy;

/*******************************************************************************
*		Interrupt handlers
*******************************************************************************/
void appMain(void)
{
	redrawScreen(FALSE);

	while(1) {
		int key = kybGetKey();
		switch(key) {
			case '1':
				causeAbortExecute();
			break;
			case '2':
			{
				// Cause an abort by trying to write to an invalid address
				int* ptr = (int*)0x0FFFFFFB;
				*ptr = dummy;
			}
			break;
			case '3':
			{
				// Cause an abort by trying to read from an invalid address
				int* ptr = (int*)0x0FFFFFFC;
				dummy = *ptr;
			}
			break;
			case '4':
				dummy = 10/zero;
				break;
			case '5':
				causeUndefinedInstruction();
				break;
			case '6':
				causeIllegalInstruction();
				break;
			case '7':
				lastSystemCallResult = causeSystemCall();
				break;
			case '8':
				causeIRQ();
				break;
		}
	}
}

void launchApplication(void)
{
	appMain();
	
	// Normally, once the main function of an application finished, the OS
	// would do some cleanup. In this case, if the application ever finishes,
	// we are just blocking here
	loopForever();
}
