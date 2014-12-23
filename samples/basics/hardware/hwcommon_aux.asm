.text


;
; Full hwi call.
; It's a bit of overkill for device functions that don't use all the registers,
; but it's flexible.
; Whenever speed is required, a specialized function should be created that
; only uses the required registers
;
; In
; r0 - bus
; r1 - function number
; r2 - pointer to array of 4 words and 4 doubles
;	This is used both as input and output
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
	; Some hardware functions might need floating point registers, so set them
	fldrd f0, [r4 + 4*4 + 8*0]
	fldrd f1, [r4 + 4*4 + 8*1]
	fldrd f2, [r4 + 4*4 + 8*2]
	fldrd f3, [r4 + 4*4 + 8*3]
	
	hwi
	; Save outputs back to the struct
	str [r4 + 4*0], r0
	str [r4 + 4*1], r1
	str [r4 + 4*2], r2
	str [r4 + 4*3], r3
	; Save floating registers some hardware functions might need
	fstrd [r4 + 4*4 + 8*0], f0
	fstrd [r4 + 4*4 + 8*1], f1
	fstrd [r4 + 4*4 + 8*2], f2
	fstrd [r4 + 4*4 + 8*3], f3

	; hwi puts any error codes in ip, move setup the return value
	mov r0, ip
	
	pop {r4,pc}

