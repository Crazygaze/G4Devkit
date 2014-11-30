
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
	; Screen device always on bus 2
	; Function 0 retrieves information
	mov ip, (0x2<<24) | 0 ;
	hwi
	str [_screenBuffer], r0 ; Save the screen buffer address
	str [_screenXRes], r1
	str [_screenYRes], r2
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
	; Clock is at bus 1
	; Clock function 0 gives us the running time
	mov ip, (1<<24) | 0
	hwi
	; Running time in seconds is already in f0, so return
	mov pc, lr

;
; Full hwi call
; In
; r0 - bus
; r1 - function number
; r2 - pointer to array of 4 words
public _hwiCall
_hwiCall:
	push {r4,lr} ; save registers we will need to restore
	
	sll ip, r0, 24
	or ip, ip, r1	
	mov r4, r2
	
	; setup call to hwi.
	ldr r0, [r4 + 4*0]
	ldr r1, [r4 + 4*1]
	ldr r2, [r4 + 4*2]
	ldr r3, [r4 + 4*3]
	hwi
	; Save outputs back to the struct
	str [r4 + 4*0], r0
	str [r4 + 4*1], r1
	str [r4 + 4*2], r2
	str [r4 + 4*3], r3

	; hwi puts any error codes in ip, move setup the return value
	mov r0, ip
	
	pop {r4,pc}

;*******************************************************************************
;    Keyboard functions
;*******************************************************************************

public _kybClearBuffer
_kybClearBuffer:
	; Keyboard fixed on bus 3
	; Keyboard function 0: Clear buffer
	mov ip, (0x3<<24) | 0
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
	; Keyboard fixed on bus 3
	; Keyboard function 0: Get next character in queue
	mov ip, (0x3<<24) | 1
	hwi

	; hwi result:
	;	r0 - Event type (0:Empty, 1:Pressed, 2:Released, 3:Typed)
	;	r1 - key
	cmp r0, 3
	bne _kybGetKey ; Keep looping until we get a Typed event
	mov r0, r1
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
	
