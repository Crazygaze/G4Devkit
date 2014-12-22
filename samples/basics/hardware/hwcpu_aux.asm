.text

public _cpu_halt
_cpu_halt:
	hlt
	mov pc, lr

public _cpu_dbgbrk
_cpu_dbgbrk:
	dbgbrk
	mov pc, lr


public _cpu_enableIRQ
_cpu_enableIRQ:
	mrs r0 ; load flags register
	and r0, r0, ~(1<<27) ; clear bit 27
	msr r0 ; and set the flags register to the new value
	mov pc, lr

public _cpu_disableIRQ
_cpu_disableIRQ:
	mrs r0 ; load flags register
	or r0, r0, 1<<27 ; Set bit 27
	msr r0 ; and set the flags register to the new value
	mov pc, lr
	
; bool cpu_getNextIRQ(IRQData* dst, u8 busFilter);
public _cpu_getNextIRQ
_cpu_getNextIRQ:
	push {r4, r5, lr}
	
	mov r5, r0
	nextirq r1
	cmp ip, 0 ; nextirq sets ip to 0 if the queue was empty
	beq _cpu_getNextIRQ_empty
	
	; Save the bus
	srl r4, ip, 24
	str [r5 + 4*0], r4
	; Save the reason
	and r4, ip, 0x80FFFFFF
	str [r5 + 4*1], r4
	; Save r0..r3
	str [r5 + 4*2], r0
	str [r5 + 4*3], r1
	str [r5 + 4*4], r2
	str [r5 + 4*5], r3
	mov r0, 1
	b _cpu_getNextIRQ_exit
	
	_cpu_getNextIRQ_empty:
	mov r0, 0
	
	_cpu_getNextIRQ_exit:
	pop {r4, r5, pc}
	