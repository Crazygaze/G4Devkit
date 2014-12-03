.text


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
