
;*******************************************************************************
; This is required for every application, as some portion at the beginning of
; the RAM is reserved
;*******************************************************************************

; Interrupt Vector Table
; We only need to set the one that jumps to our main function.
; In this sample, we don't make use of other Interrupts.
.text
.word _startup ; RESET handler
.word 0  ; Interrupt handler (not used in this sample)

;
; The default CPU context is fixed at address 8, and we need to reserve that
; space
.zero 196 ; registers (r0..pc), flags register, and floating point registers

;*******************************************************************************
; Our main function
; 
;*******************************************************************************
.text
; When the computer boots, it switches to the context found at address 8
; and starts executing the RESET handler
public _startup
_startup:

	; Get the address of the screen device buffer
	; Screen device is always on device bus 2
	; Device function 0 gets the screen buffer address
	mov ip, (0x2<<24) | 0 ;
	hwi
	; The screen buffer address is returned in r0
	mov r9, r0 ; keep the screen address in r9

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

