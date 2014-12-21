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