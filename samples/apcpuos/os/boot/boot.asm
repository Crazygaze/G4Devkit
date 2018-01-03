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
; - Execution context set to fixed location 0x8
;   This is also the execution context used for any other interrupts
; - PC is set to the "Reset" interrupt handler.
; - Some registers are set:
;   r0,r1,r2,r3 - 0
;	flags
;		- IRQs are disabled
;		- CPU is set to Supervisor mode
;
; When booting, the machine behaves as if an interrupt occurred (The "Reset"
; interrupt)
;
; These is a brief explanation about each interrupt type. Please refer to the
; architecture documentation for detailed information
;
; When an interrupt occurs:
; - Execution context changes to fixed address 0x8
; - PC is set to the Interrupt Handler
; - Cpu Mode set to Supervisor
; - IRQs are disabled
; - Some registers are set:
;		lr - The interrupted context
;		ip - Bus and Reason for the interrupt: (bus << 24) | reason
;       r0,r1,r2,r3 - Values dependent on the interrupt type.
;
;*******************************************************************************

; First 4 bytes should contain the interrupt handler address, but on boot, we
; point it to our boot function, which will patch this to point to the interrupt
; handler
_intrHandlerAddr:
.word _intrHandler_Reset;
.word _intrHandler_All

; This is the default execution context
public _intrCtxStart
_intrCtxStart:
	.zero 64 ; r0-r15
	.zero 8 ; rim0, rim1
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
	str [_krn_currIntrBusAndReason], 0
	str [_krn_prevIntrBusAndReason], 0
	
	;
	; Setup a temporary stack frame so we can call C code to do most of the work
	; We do this by pointing the stack to the top address (which is the ammount
	; of ram we have).
	; Although the top portion of the ram will be mapped for the screen buffer,
	; C code will take care of setting a proper stack frame, so that portion of
	; ram is free to be used for the screen buffer
	
	; Check how much RAM we have
	mov ip, (0<<24) | 0; Bus 0, function 0
	hwf
	str [_ramAmount] , r0
	mov sp, r0 ; set stack to top address

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
	str [_krn_currIntrBusAndReason], -1
	
	; Switch to the contex to run.
	ctxswitch [r0]

	; We will only get here if some other context changes to our context, which
	; per design, we don't allow
	extern _krn_panicUnexpectedCtxSwitch
	b _krn_panicUnexpectedCtxSwitch	

;*******************************************************************************
;              INTERRUPT HANDLERS
;*******************************************************************************
extern _krn_handleInterrupt

_intrHandler_All:
	; Save the interrupted context
	str [_krn_interruptedCtx], lr
	
	; Set the previous bus and reason variable
	; This allows us to detect kernel double faults
	ldr r4, [_krn_currIntrBusAndReason]
	str [_krn_prevIntrBusAndReason], r4
	str [_krn_currIntrBusAndReason], ip
	
	bl _krn_handleInterrupt
	
	; We are done with the interrupt servicing, so martk it as so.
	str [_krn_currIntrBusAndReason], -1
	
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
.word 0

; These two variables allows detection of double faults.
; When we are serving an interrupt, if another one happens, it's a double fault.
public _krn_currIntrBusAndReason
_krn_currIntrBusAndReason:
.word 15
public _krn_prevIntrBusAndReason
_krn_prevIntrBusAndReason:
.word 15

; Set when an interrupt occurs
public _krn_interruptedCtx
_krn_interruptedCtx:
.word 0

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


