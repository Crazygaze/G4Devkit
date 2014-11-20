
;*******************************************************************************
;								Code
;*******************************************************************************

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
; Small function to write a character to the screen
; In:
;	r0 - x
;	r1 - y
; 	r2 - character to show
public _printCharacter
_printCharacter:
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
	mov ip, lr
	
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
		
	mov pc, ip

;*******************************************************************************
;								Data
;*******************************************************************************
.data
	_screenBuffer:
	.word 0
