.text

public _kyb_clearBuffer
_kyb_clearBuffer:
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
public _kyb_getKey
_kyb_getKey:
	; Keyboard fixed on bus 3
	; Keyboard function 0: Get next character in queue
	mov ip, (0x3<<24) | 1
	hwi

	; hwi result:
	;	r0 - Event type (0:Empty, 1:Pressed, 2:Released, 3:Typed)
	;	r1 - key
	cmp r0, 3
	bne _kyb_getKey ; Keep looping until we get a Typed event
	mov r0, r1
	mov pc, lr
	