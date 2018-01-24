;*******************************************************************************
;
; This sample shows how to install an handler for every type of interrupt.
; What you do when an interrupt occurs is up to you.
;
; NOTE: Because when some types of interrupts cause the debugger to go into
; break mode, you should detach the debugger after launching the sample
;
; The code flow for this sample goes something like this:
;	- The Interrupt vector table specifies the handler for the RESET and
;	  hardware interrupt type.
;	- When an interrupt occurs, the handler passes control to the respective C
;	  function. It's easier to only have the bare minimum in assembly, and do
;	  most of the work in C
;	- At boot, a RESET interrupt occurs
;		- The handler passes control to the C function, which setups the
;		  application context.
;		- Execution is passed to the application context.
;	- The application waits for a key to be pressed, and they causes the 
;	  desired interrupt.
;	- If the interrupt that occurs is to be considered an error, like for
;	  example the application tried to read/write/execute an invalid address,
;	  some variables are updated, and the application is restarted, so we can
;	  keep running the sample.
;	- If the interrupt is an IRQ or a SWI (system call), the interrupt is
;	  handled, some variables udpated and then the application is resumed.
;*******************************************************************************

.text

; Interrupt vector table
.word _resetHandler
.word _interruptHandler

;
; The default Execution context is fixed at address 8
; It's the context the cpu switches to at boot or for interrupts
;
; Note that that there is an extra word at the end which is a pointer to the
; context name. This is so that our interrupt context matches the Ctx struct
; defined in the C file
_interruptCtx:
.zero 204 ; registers (r0...pc), flags register, rim0/rim1, and floating point registers

; Functions in the C file, which have the real interrupt handlers
extern _handleReset
extern _handleInterrupt

; Variables in the C file
extern _interruptedCtx
extern _interruptBus
extern _interruptReason
.text

;*******************************************************************************
; 		INTERRUPT HANDLERS
; These just pass control to a C function which does the real work
; It's easier to work with C than Assembly
;*******************************************************************************

_resetHandler:
	bl _handleReset
	ctxswitch [r0]

;
; All interrupts handlers call this, to avoid code duplication
_interruptHandler:
	str [_interruptedCtx], lr ; save interrupted context

	; Save the Bus and reason that caused the interrupt
	srl r4, ip, 24
	str [_interruptBus], r4;
	and r4, ip, 0x80FFFFFF
	str [_interruptReason], r4;
	
	bl _handleInterrupt
	ctxswitch [r0]
	; We should never get here...


;*******************************************************************************
; Utility functions called from the C file, to cause some interrupts for testing
;*******************************************************************************	

; Utility function to try to execute at an invalid address,
; therefore cusing an Abort interrupt
public _causeAbortExecute
_causeAbortExecute:
	mov pc, 0x0FFFFFFA
	
; Utility function to cause a "Undefined Instruction" interrupt
public _causeUndefinedInstruction
_causeUndefinedInstruction:
	.word 0xFFFFFFFF

; Utility function to cause a "Illegal Instruction" interrupt
; This is done by setting the application context to run in User Mode, then
; call a privileged instruction
public _causeIllegalInstruction
_causeIllegalInstruction:
	mov r0,0
	msr r0 ; Set the flags register 0, which disable the Supervisor Mode bit
	; This should now cause a "Illegal instruction" interrupt, as 'hwi' is a
	; privileged instruction
	hwf

public _causeSystemCall
_causeSystemCall:
	; For a system call, you would here set the registers with the parameters
	; you want to pass to the kernel
	mov r0, 0xF00D
	mov r1, 0xBEEF
	mov r2, 0
	mov r3, 0
	swi
	; NOTE: The SWI interrupt handler will set our r0 to the result
	mov pc, lr

public _causeIRQ
_causeIRQ:
	push {lr}
	; Clock is fixed at bus 1
	; Clock function 2 sets a timer
	; Read the Clock documentation to understand all the parameters	
	mov ip, (0x1 << 24) | 2;	
	mov r0, 0x40000007 ; Timer 7, IRQ mode, No auto reset
	mov r1, 100 ; Trigger the timer in 1 ms.
	hwf
	pop {pc}

;*******************************************************************************
;										DATA
;*******************************************************************************
.data
	_interruptCtxName:


