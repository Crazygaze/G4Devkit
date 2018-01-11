.text
.word _startup ; RESET handler
.word 0
ctx:
.zero 204 ; registers (r0..pc), rim0,rim1, flags register, and floating point registers

public _startup
_startup:
	loop:
	b loop
	


