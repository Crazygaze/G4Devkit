.text

;
; Interrupt handlers
; 
.word _startup ; RESET interrupt handler
.zero 28 ; Other interrupt handlers we don't need for this samples

; The default context is fixed at address 32
_startupCtx:
	.zero 196 ; registers (r0..pc), flags register, and floating point registers

; When the computer boots, it switches to the context found at address 32
; and starts executing the interrupt specified at the address 0,
; which is the RESET interrupt
public _startup
_startup:

	; Get the address of the screen device buffer
	mov r0, 2 ; Screen device is always in device bus 2
	mov r1, 0 ; Device function 0
	hwi

	mov r9, r1 ; keep the screen address in r9

	;
	; Prints a "Hello World" null terminated string 
	; at the top left of the screen
	;

	; Pointer to the screen position to write to
	lea r1, [_hello] ; Pointer to string to print
	ldrub r2, [r1] ; Get the first character

	printCharacter:
		or r3, r2, 0x0F00 ; Add colour information
		struh [r9], r3 ; print character as half word (1 byte for colour, 1 for the character)
		add r9, r9, 2 ; move screen pointer to the next position
		add r1, r1, 1 ; advance to the net character
		ldrub r2, [r1] ; read character
		cmp r2, 0 ; check for end of string
		bne printCharacter
	
	infiniteLoop:
		b infiniteLoop

	
.data
	_hello:
	.string "Hello World!"

