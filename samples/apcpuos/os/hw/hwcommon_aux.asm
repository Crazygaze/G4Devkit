.text

;
; Full hwi call
public _hw_hwiFull
_hw_hwiFull:
	stmfd sp!, {r4-r10,lr} ; save registers we will need to restore
	mov ip, r0; save the parameter
	
	; setup call to hwi.
	; This loads r0-r10 with the contents of the memory starting at ip
	ldmia ip, {r0-r10}
	hwi
	
	; save the state of the registers after hwi into the output structure
	stmia ip, {r0-r10}
	ldmfd sp!, {r4-r10,pc}
