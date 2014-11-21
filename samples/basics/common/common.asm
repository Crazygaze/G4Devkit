
;*******************************************************************************
;								Code
;*******************************************************************************

.text

; 
; Initializes some things needed by the other functions
;
public _initCommon
_initCommon:
	; Get the address of the screen device buffer
	mov r0, 2 ; Screen device is always in device bus 2
	mov r1, 0 ; Device function 0
	hwi
	str [_screenBuffer], r1 ; Save the screen buffer address
	mov pc, lr

;
; Returns the pointer to a given screen position
; In:
;	r0 - x
;	r1 - y
; Out:
;	r0 - Pointer to the screen position
_calculateScreenPosition:
	; pos = screenBuffer + (y*160 + x*2)
	smul r0, r0, 2
	smul r1, r1, 160
	ldr ip, [_screenBuffer]
	add ip, ip, r0
	add r0, ip, r1 ; final address
	mov pc, lr
	
;
; Small function to write a character to the screen
; In:
;	r0 - x
;	r1 - y
; 	r2 - character to show
public _printCharacter
_printCharacter:
	push {lr}
	bl _calculateScreenPosition
	; print the character
	or r2, r2, 0x0F00 ; Add colour information to the character
	; print character as half word (1 byte for colour, 1 for the character)
	struh [r0], r2
	pop {pc}

;
; Prints a string at the specified screen position
; No checks are made to make sure if the screen falls outside the screen
; In
;	r0 - x
;	r1 - y
;	r2 - pointer to a null terminated string
public _printString
_printString:
	push {r4,r5,lr}
	bl _calculateScreenPosition
	ldrub r4, [r2] ; Get character
	_printString_Loop:
		or r5, r4, 0x0F00; Add colour information
		struh [r0], r5 ; Write to screen
		add r0, r0, 2 ; Move screen position
		; Increment string pointer, and check for null character
		add r2, r2, 1
		ldrub r4, [r2] ; Get character
		cmp r4,0
		bne _printString_Loop
	pop {r4,r5,pc}

;
; Returns the system running time in seconds (How long it's been running since
; boot )
; In:
;	None
; Out:
;	f0 - Running time in seconds
public _getRunningTimeSeconds
_getRunningTimeSeconds:
	mov r0, 1 ; Clock is at busid 1
	mov r1, 0 ; Clock function 0 gives us the running time
	hwi
	; Running time in seconds is already in f0, so return
	mov pc, lr


; Pause for the specified milliseconds
; In:
;	r0 - Pause duration in ms
;
public _pause
_pause:
	push {lr}
	
	;
	; Convert the parameter to seconds
	;
	fflt f1, r0 ; Convert r0 to float
	; Set f2 to 1000.0
	; Ideally, this could be done by having floating point constants in the data
	; section and doing: fldrs f2, [_someConstant]
	mov r1, 1000
	fflt f2, r1
	fdiv f1, f1, f2 ; Convert to seconds: f1 = f1 / 1000

	; Get the current time, so we can then compare and loop and wait the
	; required time
	bl _getRunningTimeSeconds
	fadd f1, f1, f0 ; finalTime = f1 + getRunningTimeSeconds()
	
	_pause_loop:
		bl _getRunningTimeSeconds
		fcmp f0, f1
		blt _pause_loop ; If current time is lower than our required time, loop
		
	pop {pc}

;*******************************************************************************
;								Data
;*******************************************************************************
.data
	_screenBuffer:
	.word 0
