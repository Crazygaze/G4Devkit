.text

;
; Small hwf call, where only r0...r3 are needed.
;
; In
; r0 - bus
; r1 - function number
; r2 - pointer to array of 4 words
;	This is used both as input and output
public _hw_hwfsmall:
	enter 0
	pushm {r4}
	
	; build ip register
	sll ip, r1, 8
	or ip, ip, r0
	; Keep the destination pointer
	mov r4, r2
	; set inputs
	ldr r0, [r4 + 4*0]
	ldr r1, [r4 + 4*1]
	ldr r2, [r4 + 4*2]
	ldr r3, [r4 + 4*3]	
	;
	hwf ip
	; copy back outputs
	str [r4 + 4*0], r0
	str [r4 + 4*1], r1
	str [r4 + 4*2], r2
	str [r4 + 4*3], r3
	; hwf puts any error codes in ip
	mov r0, ip
	
	popm {r4}
	leave
	ret 0
	
;
; In
; r0 - bus
; r1 - function number
public _hw_hwf_0_0:
	enter 0
	
	; build ip register
	sll ip, r1, 8
	or ip, ip, r0
	;
	hwf ip
	
	; hwf puts any error codes in ip
	mov r0, ip
	
	leave
	ret 0
	
;
; In
; r0 - bus
; r1 - function number
; r2 - pointer to array of 4 words
;	This is used both as input and output
public _hw_hwf_0_1:
	enter 0
	
	; build ip register
	sll ip, r1, 8
	or ip, ip, r0
	;
	hwf ip
	; copy back outputs
	str [r2 + 4*0], r0
	
	; hwf puts any error codes in ip
	mov r0, ip
	
	leave
	ret 0
	
;
; In
; r0 - bus
; r1 - function number
; r2 - pointer to array of 4 words
;	This is used both as input and output
public _hw_hwf_0_2:
	enter 0
	
	; build ip register
	sll ip, r1, 8
	or ip, ip, r0
	;
	hwf ip
	; copy back outputs
	str [r2 + 4*0], r0
	str [r2 + 4*1], r1
	
	; hwf puts any error codes in ip
	mov r0, ip
	
	leave
	ret 0
	
;
; In
; r0 - bus
; r1 - function number
; r2 - pointer to array of 4 words
;	This is used both as input and output
public _hw_hwf_0_4:
	enter 0
	pushm {r4}
	
	; build ip register
	sll ip, r1, 8
	or ip, ip, r0
	; Keep the destination pointer
	mov r4, r2
	;
	hwf ip
	; copy back outputs
	str [r4 + 4*0], r0
	str [r4 + 4*1], r1
	str [r4 + 4*2], r2
	str [r4 + 4*3], r3
	; hwf puts any error codes in ip
	mov r0, ip
	
	popm {r4}
	leave
	ret 0
	
;
; In
; r0 - bus
; r1 - function number
; r2 - pointer to array of 4 words
;	This is used both as input and output
public _hw_hwf_0_3:
	enter 0
	pushm {r4}
	
	; build ip register
	sll ip, r1, 8
	or ip, ip, r0
	; Keep the destination pointer
	mov r4, r2
	;
	hwf ip
	; copy back outputs
	str [r4 + 4*0], r0
	str [r4 + 4*1], r1
	str [r4 + 4*2], r2
	; hwf puts any error codes in ip
	mov r0, ip
	
	popm {r4}
	leave
	ret 0
	
;
; In
; r0 - bus
; r1 - function number
; r2 - pointer to array of 4 words
;	This is used both as input and output
public _hw_hwf_1_0:
	enter 0
	pushm {r4}
	
	; build ip register
	sll ip, r1, 8
	or ip, ip, r0
	; Keep the destination pointer
	mov r4, r2
	; set inputs
	ldr r0, [r4 + 4*0]
	;
	hwf ip
	; hwf puts any error codes in ip
	mov r0, ip
	
	popm {r4}
	leave
	ret 0
	
	
;
; In
; r0 - bus
; r1 - function number
; r2 - pointer to array of 4 words
;	This is used both as input and output
public _hw_hwf_2_0:
	enter 0
	pushm {r4}
	
	; build ip register
	sll ip, r1, 8
	or ip, ip, r0
	; Keep the destination pointer
	mov r4, r2
	; set inputs
	ldr r0, [r4 + 4*0]
	ldr r1, [r4 + 4*1]
	;
	hwf ip
	; hwf puts any error codes in ip
	mov r0, ip
	
	popm {r4}
	leave
	ret 0
	
	
;
; In
; r0 - bus
; r1 - function number
; r2 - pointer to array of 4 words
;	This is used both as input and output
public _hw_hwf_3_0:
	enter 0
	pushm {r4}
	
	; build ip register
	sll ip, r1, 8
	or ip, ip, r0
	; Keep the destination pointer
	mov r4, r2
	; set inputs
	ldr r0, [r4 + 4*0]
	ldr r1, [r4 + 4*1]
	ldr r2, [r4 + 4*2]
	;
	hwf ip
	; hwf puts any error codes in ip
	mov r0, ip
	
	popm {r4}
	leave
	ret 0
	
;
; In
; r0 - bus
; r1 - function number
; r2 - pointer to array of 4 words
;	This is used both as input and output
public _hw_hwf_1_1:
	enter 0
	
	; build ip register
	sll ip, r1, 8
	or ip, ip, r0
	; set inputs
	ldr r0, [r2 + 4*0]
	;
	hwf ip
	; copy back outputs
	str [r2 + 4*0], r0
	; hwf puts any error codes in ip
	mov r0, ip
	
	leave
	ret 0
	
;
; Full hwf call.
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
public _hw_hwffull:
	enter 0
	pushm {r4} ; save registers we will need to restore
	
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
	; Some hardware functions might need floating point registers, so set them
	fldrd f0, [r4 + 4*4 + 8*0]
	fldrd f1, [r4 + 4*4 + 8*1]
	fldrd f2, [r4 + 4*4 + 8*2]
	fldrd f3, [r4 + 4*4 + 8*3]
	
	hwf ip
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

	; hwf puts any error codes in ip
	mov r0, ip
	
	; pop saved registers and return
	popm {r4}
	leave
	ret 0

