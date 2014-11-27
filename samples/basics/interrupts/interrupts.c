
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

// The assembly interrupt handler sets this whenever an interrupt happens
Ctx* interruptedCtx;
// The assembly interrupt handler sets this whenever an IRQ interrupt happens
u32 interruptReason;

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
	u32 data0, u32 data1, u32 data2, u32 data3)
{
	redrawScreen(TRUE);
	
	int x = 4;
	int y = 1;
	printStringData(x,++y, "Interrupt type: ", title);
	printNumberData(x+50,y, "Bus|Reason: ", interruptReason,16);
	printStringData(x,++y, "Interrupted context name: ", interruptedCtx->name);
	printNumberData(x,++y, "Num interrupts: ", interruptsCount, 10);
	printNumberData(x,++y, "Num of application restarts: ", restartsCount, 10);
	printNumberData(x,++y, "Interrupt Data0: ", data0, 16);
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


void cpuHandleIRQ(u32 data0, u32 data1, u32 data2, u32 data3)
{
	interruptsCount++;
	printInterruptDetails(interruptedCtx, "IRQ",data0,data1,data2,data3);
	// Note that for an IRQ handler, an operating system would just handle it
	// then resume the application.
	// That's why I'm not calling "restart" here
}

#define CPU_INTERRUPT_RESET 0
#define CPU_INTERRUPT_ABORT 1
#define CPU_INTERRUPT_DIVIDEBYZERO 2
#define CPU_INTERRUPT_UNDEFINEINSTRUCTION 3
#define CPU_INTERRUPT_ILLEGALINSTRUCTION 4
#define CPU_INTERRUPT_SWI 5
#define CPU_INTERRUPT_IRQ 6

void handleCpuInterrupt(u32 reason,u32 data0, u32 data1, u32 data2, u32 data3)
{
	static const char* reasons[7] =
	{
		"RESET",
		"ABORT",
		"DIVIDEBYZERO",
		"UNDEFINEDINSTRUCTION",
		"ILLEGALINSTRUCTION",
		"SWI",
		"IRQ",
	};
	
	static const char* abortTypes[3] = {
		"Abort (Execute)",
		"Abort (Write)",
		"Abort (Read)"};		
	
	interruptsCount++;	
	switch(reason)
	{
		case CPU_INTERRUPT_RESET:
			initCommon();
			setupAppCtx();
			interruptedCtx = &appCtx;
		break;
		
		case CPU_INTERRUPT_ABORT:
		{
			printInterruptDetails(interruptedCtx, abortTypes[data1], data0, data1,
				0, 0);
			setupAppCtx();
		}
		break;
		
		case CPU_INTERRUPT_DIVIDEBYZERO:
		case CPU_INTERRUPT_UNDEFINEINSTRUCTION:
		case CPU_INTERRUPT_ILLEGALINSTRUCTION:
			printInterruptDetails(interruptedCtx, reasons[reason],0,0,0,0);
			setupAppCtx();
		break;
				
		case CPU_INTERRUPT_SWI:
			// NOTE
			// The system call parameters are in the interrupted context's
			// registers, so I'm printing r0 and r1 to show this came frome the
			// assembly function "_causeSystemCall"
			printInterruptDetails(interruptedCtx, reasons[reason],
				interruptedCtx->gregs[0], interruptedCtx->gregs[1],0,0);

			// A SWI interrupt handler should set the interrupted context's registers
			// to any values it wishes to return back to the application.
			// In this case, we are just setting r0 to an incrementing value
			interruptedCtx->gregs[0] = ++lastSystemCallResult;		
			break;

		default:
			// unsupported interrupt type ?
	}
		
}

void handleClockInterrupt(u32 reason, u32 data0, u32 data1, u32 data2, u32 data3)
{
	interruptsCount++;
	printInterruptDetails(interruptedCtx, "IRQ",data0,data1,data2,data3);
}

void clockHandleTimer(u32 data0, u32 data1, u32 data2, u32 data3)
{
}

#define HWDEVICE_CPU 0
#define HWDEVICE_CLK 1

Ctx* handleInterrupt(u32 data0, u32 data1, u32 data2, u32 data3)
{
	u8 bus = interruptReason >> 24;
	u32 reason = interruptReason & 0x80FFFFFF;
	
	if (bus==HWDEVICE_CPU) // CPU
		handleCpuInterrupt(reason, data0, data1, data2, data3);
	else if (bus==HWDEVICE_CLK) //
		handleClockInterrupt(reason, data0, data1, data2, data3);
	else {
		// Interrupt for a device we don't support in this sample
	}
	
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
	int y = 12;
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
