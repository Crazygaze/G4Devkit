;*******************************************************************************
; This sample shows how to do context switching
; It seems long, but most of the code is setting up things, which if done in
; C would be mostly assignment statements.
;
;              - Overview how the sample works -
;
; - On boot the cpu changes to context at fixed location 32 (_mainCtx here),
;   and sets the PC register of that context to the RESET interrupt handler,
;	named here as label _startup
; - What _startup does
;		- Get and save screen buffer address so we can use it later
;		- Setup the contexts _ctx1 and _ctx2
;		- Change executiong to _ctx1
;		- Execution keeps switching between _ctx1 and _ctx2
;
; - _ctx1/_ctx2 execution goes like this:
;		- Clear the last character
;		- Increment the screen column
;		- Print character at the new screen position
;		- Change execution to the other context
;		- Once the other context passes execution back, loop
;
; To avoid requiring a stack, everything _ctx1 and _ctx2 need to know is kept
; in registers, which the main context setups. _ctx1/_ctx2 state is setup as
;	r4 - Screen column where to display the character
;	r5 - Screen row where to display the character
;	r6 - Character to show
;	r7 - context to switch to after 1 iteration
;
;*******************************************************************************

.text

; Interrupt vector
.word _startup ; RESET interrupt handler
.zero 28 ; Space for the other interrupts

;
; The default CPU context is fixed at address 32
_mainCtx:
.zero 196 ; registers (r0..pc), flags register, and floating point registers

; The two contexts we will be running
_ctx1:
.zero 196
_ctx2:
.zero 196

;*******************************************************************************
; Booting execution, as specified by the RESET interrupt handler
;*******************************************************************************
.text
public _startup
_startup:

	; Get the address of the screen device buffer
	mov r0, 2 ; Screen device is always in device bus 2
	mov r1, 0 ; Device function 0
	hwi
	str [_screenBuffer], r1 ; Save the screen buffer address

	;
	; Setup the contexts
	;
	; Set the screen columns the contexts will write to (kept in register r4)
	str [_ctx1 + 4*4], 0
	str [_ctx2 + 4*4], 0
	
	; Set screen rows the contexts will write to (kept in the r5 register)
	str [_ctx1 + 5*4], 0
	str [_ctx2 + 5*4], 1

	; Set character to show (kept in register r6)
	str [_ctx1 + 6*4], 49
	str [_ctx2 + 6*4], 50
	
	; Set what context each context switches to after 1 iteration (register r7)
	lea r1, [_ctx1]
	lea r2, [_ctx2]
	str[_ctx1 + 7*4], r2 ; ctx1 switches to ctx2
	str[_ctx2 + 7*4], r1 ; ctx2 swtiches to ctx1

	; Set where the contexts start execution
	lea r0, [_contextLoop]
	str [_ctx1 + 15*4], r0 ; Set context's PC register
	str [_ctx2 + 15*4], r0 ; Set context's PC register
	
	; Set the contexts flags registers
	; This is necessary, so the contexts have proper permissions to execute
	; ctxswitch, since it's a previleged instruction.
	; In this case I'm just copying the flags from the main context
	mrs r0 ; Get our flags register
	str [_ctx1 + 16*4], r0
	str [_ctx2 + 16*4], r0

	; Change execution to ctx1
	ctxswitch [r1]
	
	; NOTE: We never get here, unless some of the other contexts switch to us
	; again, which are not doing in this sample

;
; _ctx1/_ctx2 looping
;
_contextLoop:

	; Clear the previous screen character
	mov r0, r4 ; x
	mov r1, r5 ; y
	mov r2, 32; a space, to clear
	bl _printChar
	
	; Increment screen column
	; r4 = (r4+1) % 80
	add r4, r4, 1
	sdiv r4:ip, r4, 80
	
	; Print the character at the desired screen coordinates
	mov r0, r4 ; x
	mov r1, r5 ; y
	mov r2, r6 ; character
	bl _printChar

	; Change execution to the other context
	ctxswitch [r7]
	
	; loop to beginning when this context gets resumed
	b _contextLoop 

;
; Small function to write a character to the screen
; In:
;	r0 - x
;	r1 - y
; 	r2 - character to show
_printChar:
	; calculate screen position
	; pos = screenBuffer + (y*160 + x*2)
	smul r0, r0, 2
	smul r1, r1, 160
	ldr ip, [_screenBuffer]
	add ip, ip, r0
	add ip, ip, r1 ; final address
	
	; print the character
	or r2, r2, 0x0F00 ; Add colour information to the character
	struh [ip], r2 ; print character as half word (1 byte for colour, 1 for the character)
	
	; Return from the function
	mov pc, lr


;*******************************************************************************
;								Data
;*******************************************************************************
.data
	; Variable to save the screen buffer address
	_screenBuffer:
	.word 0
	_currentCtx:
	.word 0
	
