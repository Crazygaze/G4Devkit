;*******************************************************************************
; Bare minimum the test framework needs do to be somehow useful
;
;*******************************************************************************
;
; Code section
;
.text

; In :
;	None
; Out:
;	r0 - Screen address
; 	r1 - Number of cols
; 	r2 - Number of rows
;	r3 - Bytes per character
_test_getScreenInfo:
	mov ip, (1 << 8) | 2;
	hwf ip
	ret 0

; In
;	r0 - int : Screen column where to print
;	r1 - int : Screen line where to print
;	r2 - const char* : String to print
; Out
;	None
;
;public _test_printXY
public _test_printXY:
	enter 0
	pushm {r4, r5, r6}
	
	; save the parameters
	mov r4, r0
	mov r5, r1
	mov r6, r2
	
	call _test_getScreenInfo
	
	; Calculate the screen address for the line we want
	; offfset = (line * numCols * bytes per character) + column
	smul r5, r5, r1	; line * numColumns
	smul r5, r5, r3 ; * bytes per character
	smul r4, r4, 2  ; + (column*2)
	add r5, r5, r4
	
	; addr = screen addr + offset
	add r0, r0, r5
	
	; r1 will hold he character to print
	ldrub r1, [r6] ; Load the first character
	
.L1:
	or r4, r1, 0x0F00 ; Add colour information (white on black)
	strh [r0], r4 ; Copy the character+colour (16 bits) to the screen buffer
	add r0, r0, r3 ; move the screen pointer to the next position
	add r6, r6, 1 ; advance to the next character
	ldrub r1, [r6] ; read character
	cmp r1, 0 ; check for end of string (null character)
	bne .L1
	
	popm {r4, r5, r6}
	leave
	ret 0

;
; Small hwf call, where only r0...r3 are needed.
;
; In
; r0 - bus
; r1 - function number
; r2 - pointer to array of 4 words and 4 doubles
;	This is used both as input and output
public _test_hwfsmall:
	enter 0
	pushm {r4}
	
	; build ip register
	sll ip, r1, 8
	or ip, ip, r0
	; Keep the destination pointer
	mov r4, r2
	; setup hwf call
	ldr r0, [r4 + 4*0]
	ldr r1, [r4 + 4*1]
	ldr r2, [r4 + 4*2]
	ldr r3, [r4 + 4*3]	
	hwf ip
	; Save output back to the struct
	str [r4 + 4*0], r0
	str [r4 + 4*1], r1
	str [r4 + 4*2], r2
	str [r4 + 4*3], r3
	; hwf puts any error codes in ip
	mov r0, ip
	
	popm {r4}
	leave
	ret 0


; Check if f0 is nearly equal to f1
;
; In
; f0 and f1, values to compare
; f2 margin
; Out
; r0 true or false 
; 
public _test_nearlyEqual:
	fsub f0, f0, f1
	fabs f0, f0
	mov r0, 1    ; default to true
	fcmp f0, f2
	ble .L_test_nearlyEqual_end
	mov r0, 0 ; if >f2, then set to 0 (false)
	.L_test_nearlyEqual_end:
	ret 0
