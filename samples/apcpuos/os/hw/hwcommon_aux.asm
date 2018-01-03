.text

;
; Full hwi call
public _hw_hwiFull
_hw_hwiFull:
	push {r4,lr} ; save registers we will need to restore
	
	; Setup the bus|function register
	sll ip, r0, 24
	or ip, ip, r1
	
	; setup call to hwi.
	mov r4, r2
	ldr r0, [r4 + 4*0]
	ldr r1, [r4 + 4*1]
	ldr r2, [r4 + 4*2]
	ldr r3, [r4 + 4*3]
	hwf
	; Save the results
	str [r4 + 4*0], r0
	str [r4 + 4*1], r1
	str [r4 + 4*2], r2
	str [r4 + 4*3], r3
	; ip has the error code, if any
	mov r0, ip
	
	pop {r4,pc}
