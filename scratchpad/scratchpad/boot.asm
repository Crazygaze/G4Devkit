;*******************************************************************************
; Bare minimum to print a null terminated string to the screen
; By default, the computer boots with easy to use defaults, so there is no setup
; to perform for simple programs such as this one.
; The only setup related thing to implement in this example is just getting
; the memory address used as the screen buffer.
;*******************************************************************************

;
; Code section
;
.text

; Get the address of the screen device buffer.
; The screen device is always on bus 2.
; Device function 1 gets the screen buffer address
; The screen buffer address is returned in r0
mov ip, (1 << 8) | 2;
hwf

;
; Prints a "Hello World!" at the top left of the screen
; Register use:
; r0 : Screen address to write to
; r1 : Address of the next character to print
;
lea r1, [_hello] ; Load the address of _hello
ldrub r2, [r1] ; Load the first character
printCharacter:
	or r3, r2, 0x0F00 ; Add colour information (white on black)
	strh [r0], r3 ; Copy the character+colour (16 bits) to the screen buffer
	add r0, r0, 2 ; move the screen pointer to the next position
	add r1, r1, 1 ; advance to the next character
	ldrub r2, [r1] ; read character
	cmp r2, 0 ; check for end of string (null character)
	bne printCharacter

hlt ; Pause the cpu

;
; Read-only data
; Note that by using .rodata doesn't in itself enable memory protection, but
; even if you are not using memory protection, it's still good practice to use
; .rodata for read-only data.
.rodata

; string to print
_hello: .string "Hello World!"

