
;*******************************************************************************
;								Code
;*******************************************************************************

.text


;*******************************************************************************
;		System functions
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
	str [_screenXRes], r2
	str [_screenYRes], r3
	mov pc, lr

public _loopForever
_loopForever:
	hlt
	b _loopForever

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

;*******************************************************************************
;    Keyboard functions
;*******************************************************************************

public _kybClearBuffer
_kybClearBuffer:
	mov r0, 3 ; Keyboard is on bus 3
	mov r1, 0 ; Function 0 : Clear bnuffer
	hwi
	mov pc, lr
	
;
; Waits for a key to be typed without using IRQs
; In:
;	None
; Out:
;	r0 - key typed, or 0 if 
;
public _kybGetKey
_kybGetKey:
	mov r0, 3 ; Keyboard is on bus 3
	mov r1, 1 ; Function 1 : Get next character in queue
	hwi
	; hwi result:
	;	r0 - error code if any
	;	r1 - Event type (0:Empty, 1:Pressed, 2:Released, 3:Typed)
	;	r2 - key
	cmp r1, 3
	bne _kybGetKey ; Keep looping until we get a Typed event
	mov r0, r2
	mov pc, lr
	
;*******************************************************************************
;		Standard C library functions
;*******************************************************************************
public _memset
_memset:
	memset [r0], r1, r2
	mov pc, lr

public _memcpy
_memcpy:
	memcpy [r0], [r1], r2
	mov pc, lr

; Same as memcpy for the APCPU
public _memmove
_memmove:
	memcpy [r0], [r1], r2
	mov pc, lr

;*******************************************************************************
;								Data
;*******************************************************************************
.data
	public _screenBuffer
	_screenBuffer:
	.word 0
	
	public _screenXRes
	_screenXRes:
	.word 0
	
	public _screenYRes
	_screenYRes:
	.word 0
	
