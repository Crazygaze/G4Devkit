.text

public _cpu_halt
_cpu_halt:
	hlt
	mov pc, lr

public _cpu_dbgbrk
_cpu_dbgbrk:
	dbgbrk
	mov pc, lr
