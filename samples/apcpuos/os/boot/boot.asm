;*******************************************************************************
; 				Boot code
;*******************************************************************************


;
; declare functions from other files
extern _krn_preboot
extern _krn_init

.text

;*******************************************************************************
;
; When booting, the following happens:
;
; - Execution context set to fixed location 0x20 (32)
;   This is also the execution context used for any other interrupts
; - PC is set to the "Reset" interrupt handler.
; - Some registers are set:
;   r0,r1,r2 - 0
;	flags
;		- IRQs are disabled
;		- CPU is set to Supervisor mode
;
; When booting, the machine behaves as if an interrupt occured (The "Reset"
; interrupt)
;
; These is a brief explanation about each interrupt type. Please refer to the
; architecture documention for detailed information
; 
; - Execution context changes to fixed address 0x20 (32)
; - PC is set to the respective interrupt handler
; - Some registers are set, depending on the interrupt type:
;       r0 - set to the interrupted context
;       r1,r2,r3 - Values dependent on the the interrupt type. See below
;       flags register :
;		    Cpu Mode set to Supervisor
;		    IRQs are disabled
;
; The following is a list of the meaning of r1,r2,r3 for each interrupt type
;
; Abort:
; 	r1 = Address access that cause the interrupt
; 	r2 = Type of violation:
;		0 : Execute
;		1 : Write
;		2 : Read
;	r3 = 0
;
; DivideByZero:
;	r1 = 0
;	r2 = 0
;	r3 = 0
;
; UndefinedInstruction
;	r1 = 0
;	r2 = 0
;	r3 = 0
;
; IllegalInstruction
;	r1 = 0
;	r2 = 0
;	r3 = 0
;
; SWI (System call)
;	r1 = 0
;	r2 = 0
;	r3 = 0
;
; IRQ (Hardware Interrupt)
;	r1 = (BusId<<24) | reason
;			BusId - Identifies the device that caused the IRQ
;			reason - Specific to the device that caused the IRQ.
;	r2 = Dependent on the device that caused the interrupt
;	r3 = Dependent on the device that caused the interrupt
;
;*******************************************************************************

; First 4 bytes should contain the interrupt handler address, but on boot, we
; point it to our boot function, which will patch this to point to the interrupt
; handler
_intrHandlerAddr:
.word _intrHandler_Reset;
.word _intrHandler_Abort
.word _intrHandler_DivideByZero
.word _intrHandler_UndefinedInstruction
.word _intrHandler_IllegalInstruction
.word _intrHandler_SWI
.word _intrHandler_IRQ
.word _intrHandler_RESERVED ; Reserved for future use

; This is the default execution context
public _intrCtxStart
_intrCtxStart:
	.zero 64 ; r0-r15
	.word 0 ; flags register
	.zero 128 ; floating point registers (16*8)
	; Marker so we can check when booting if the interrupt context matches the
	; size of what we have in C source code
	public _intrCtxEnd
	_intrCtxEnd:


;*******************************************************************************
;							BOOT
; On boot we setup just the minimum we need with assembly so we can call C code.
; The minimum required for this is setting up a stack frame
;*******************************************************************************
;
_intrHandler_Reset:
	str [_krn_previousIntr], 15
	str [_krn_currentIntr], 0
	
	;
	; Setup a temporary stack frame so we can call C code to do most of the work
	; We do this by pointing the stack to the top address (which is the ammount
	; of ram we have).
	; Although the top portion of the ram will be mapped for the screen buffer,
	; C code will take care of setting a proper stack frame, so that portion of
	; ram is free to be used for the screen buffer
	
	; Check how much RAM we have
	mov r0, 0
	mov r1, 0
	hwi
	str [_ramAmount] , r1
	mov sp, r1 ; set stack to top address

	; Boot first pass to initialize basics
	; This is required, so we setup a stack where we want, exit the preboot
	; so we can abandon the temporary stack, then call the fullboot function
	; with a proper stack
	bl _krn_preboot
	mov sp, r0 ; _kernel_preboot returns the stacktop to use, so set the stack
		
	;
	; Fully initialize the rest of the system.
	; krnInit returns the context we should switch to
	;
	bl _krn_init
	
	; Switch to the contex to run.
	str [_krn_currentIntr], 15 ; Mark as not inside any interrupts
	ctxswitch [r0]

	; We will only get here if some other context changes to our context, which
	; per design, we don't allow
	extern _krn_panicUnexpectedCtxSwitch
	b _krn_panicUnexpectedCtxSwitch	

;*******************************************************************************
;              INTERRUPT HANDLERS
;*******************************************************************************
extern _krn_panicDoubleFault
extern _krn_handleInterrupt

_intrHandler_Abort:
	mov r4, 1
	b _dispatchIntr

_intrHandler_DivideByZero:
	mov r4, 2
	b _dispatchIntr

_intrHandler_UndefinedInstruction:
	mov r4, 3
	b _dispatchIntr
	
_intrHandler_IllegalInstruction:
	mov r4, 4
	b _dispatchIntr

_intrHandler_SWI:
	mov r4, 5
	b _dispatchIntr

_intrHandler_IRQ:
	mov r4, 6
	b _dispatchIntr

_intrHandler_RESERVED:
	mov r4, 7
	b _dispatchIntr
	

;
; It expects the current interrupt type in r4
_dispatchIntr:
	ldr ip, [_krn_currentIntr]
	str [_krn_previousIntr], ip
	str [_krn_currentIntr], r4
	bl _krn_handleInterrupt
	str [_krn_currentIntr], 15
	ctxswitch [r0]
	; We will only get here if some other context changes to our context, which
	; per design, we don't allow
	b _krn_panicUnexpectedCtxSwitch
	

;*******************************************************************************
;*******************************************************************************
;*******************************************************************************

;
; Global variables with misc system information
;
.bss

public _ramAmount
_ramAmount:
.zero 4

;
; This tells what interrupt type we were are serving, and the one at the moment
; 0..7 - Serving an interrupt of that type
; 15 - No interrupt being served
; Keeping track of the previous, so the kernel panic function can tell what
; interrupt type we were executing
;
public _krn_currentIntr
_krn_currentIntr:
.word 15
public _krn_previousIntr
_krn_previousIntr:
.word 15


;
; Read only data (after we set MMU)
;
.rodata

;
; Process runtime information
; This is patched by the linker when building a ROM file, 
; and information about the size of the program:
; Contents are:
; 	4 bytes - readOnlyAddress (where code and .rodata starts)
;	4 bytes - readOnlySize (size of the read only portion)
;	4 bytes - readWriteAddress ( where read/write data starts)
;	4 bytes - readWriteSize (size of the read/write data)
;	4 bytes - sharedReadWriteAddress ( where shared read/write data starts)
;	4 bytes - sharedReadWriteSize (size of the shared read/write data)
public _processInfo
_processInfo:
.zero 24

.section ".apcpudebug"


