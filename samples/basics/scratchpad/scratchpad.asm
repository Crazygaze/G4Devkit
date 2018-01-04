.text
.word _startup ; RESET handler
.word _interruptH
ctx:
.zero 204 ; registers (r0..pc), rim0,rim1, flags register, and floating point registers

ctx2:
.zero 204 ; registers (r0..pc), rim0,rim1, flags register, and floating point registers
.text

public _startup
_startup:

	;dbgbrk
	;nextirq r0
	;rdtsc r9:r10
	;hwf tested

	;swi
	;hlt tested


	cmpxchg [r10],r4,r5

	
	loop:
	b loop

public _interruptH
_interruptH:
	mov r4,r4
	loop2:
	b loop2

public _other
_other:
	lea r10, [ctx]
	ctxswitch [r10]

.data
	_hello:
	.string "Hello World!"
	_hello2:
	.string "Hello World!"
	_regs:
	.zero 204

