
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
	
	// Any extra fields must go here at the end
	const char* name;
} Ctx;

// This will be used as the application execution context
Ctx appCtx;

// We use this small memory block as a stack for the application context
#define APPSTACKSIZE 4096
char appStack[APPSTACKSIZE];

// Counts the total number of interrupts
int interruptsCount;

// Counts how many times the application was restarted.
// In most cases, an application would be closed by the OS whenever it causes
// for example an Abort,Divide by zero, etc. This counts how many times that
// happened, and we restarted the application, so we can keep running this
// sample.
// Some other interrupts, like IRQ or SWI(System call), occur normally, and
// the application is just resumed after the interrupt is handled.
int restartsCount;

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
sample).
*/
void setupAppCtx(void)
{
	restartsCount++;
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
	appCtx.name = "Application Context";
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
	u32 data1, u32 data2, u32 data3)
{
	redrawScreen(TRUE);
	
	int x = 4;
	int y = 1;
	printStringData(x,++y, "Interrupt type: ", title);
	printStringData(x,++y, "Interrupted context name: ", interruptedCtx->name);
	printNumberData(x,++y, "Num interrupts: ", interruptsCount, 10);
	printNumberData(x,++y, "Num of application restarts: ", restartsCount, 10);
	printNumberData(x,++y, "Interrupt Data1: ", data1, 16);
	printNumberData(x,++y, "Interrupt Data2: ", data2, 16);
	printNumberData(x,++y, "Interrupt Data3: ", data3, 16);
	if (lastSystemCallResult) {
		printNumberData(x,++y, "Last SWI call result: ",
			lastSystemCallResult,10);
	}
	
}


/*******************************************************************************
*		Interrupt handlers
*******************************************************************************/

Ctx* handleReset(void)
{
	initCommon();
	setupAppCtx();
	return &appCtx;
}

Ctx* handleAbort(Ctx* interruptedCtx, unsigned address, unsigned type)
{
	static const char* names[3] = {
		"Abort (Execute)",
		"Abort (Write)",
		"Abort (Read)"};
		
	interruptsCount++;
	printInterruptDetails(interruptedCtx, names[type], address, type, 0);
	setupAppCtx();
	return interruptedCtx;
}

Ctx* handleDivideByZero(Ctx* interruptedCtx)
{
	interruptsCount++;
	printInterruptDetails(interruptedCtx, "DivideByZero",0,0,0);
	setupAppCtx();
	return interruptedCtx;
}

Ctx* handleUndefinedInstruction(Ctx* interruptedCtx)
{
	interruptsCount++;
	printInterruptDetails(interruptedCtx, "UndefinedInstruction",0,0,0);
	setupAppCtx();
	return interruptedCtx;
}

Ctx* handleIllegalIntruction(Ctx* interruptedCtx)
{
	interruptsCount++;
	printInterruptDetails(interruptedCtx, "IllegalInstruction",0,0,0);
	setupAppCtx();	
	return interruptedCtx;
}

Ctx* handleSystemCall(Ctx* interruptedCtx)
{
	interruptsCount++;
	// NOTE
	// The system call parameters are in the interrupted context's registers,
	// so I'm printing r0 and r1 to show this came frome the assembly function
	// '_causeSystemCall'
	printInterruptDetails(interruptedCtx, "SystemCall",
		interruptedCtx->gregs[0], interruptedCtx->gregs[1],0);

	
	// A SWI interrupt handler should set the interrupted context's registers
	// to any values it wishes to return back to the application.
	// In this case, we are just setting r0 to an incrementing value
	interruptedCtx->gregs[0] = ++lastSystemCallResult;

	// Note that for a system call, we want to pass control back to the
	// application, so I'm not reseting the application	
	return interruptedCtx;
}

Ctx* handleIRQ(Ctx* interruptedCtx, u32 data1, u32 data2, u32 data3)
{
	interruptsCount++;
	printInterruptDetails(interruptedCtx, "IRQ",data1,data2,data3);
	// Note that for an IRQ handler, an operating system would just handle it
	// then resume the application.
	// That's why I'm not calling "restart" here

	return interruptedCtx;
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


void showMenu(void)
{
	int x = 10;
	int y = 11;
	printString(x, y++, "1. Test Abort (Execute)");
	printString(x, y++, "2. Test Abort (Write)");
	printString(x, y++, "3. Test Abort (Read)");
	printString(x, y++, "4. Test DivideByZero");
	printString(x, y++, "5. Test UndefinedInstruction");
	printString(x, y++, "6. Test IllegalInstruction");
	printString(x, y++, "7. Test SystemCall (will pass 0xf00d,0xbeef to the handler)");
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
