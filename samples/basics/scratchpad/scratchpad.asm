.text
.word _startup ; RESET handler
.word 0  ; Interrupt handler (not used in this sample)
.zero 204 ; registers (r0..pc), rim0,rim1, flags register, and floating point registers

.text
public _startup
_startup:

	lea sp, [_regs + 204]
	push {r0,lr}

	pop {r0,lr}
	
	loop:
	b loop

.data
	_hello:
	.string "Hello World!"
	_regs:
	.zero 204

