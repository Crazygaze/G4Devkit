.text
.word _startup ; RESET handler
.word _interruptH
ctx:
.zero 204 ; registers (r0..pc), rim0,rim1, flags register, and floating point registers

ctx2:
.zero 204 ; registers (r0..pc), rim0,rim1, flags register, and floating point registers
.text

extern _fooFunc;

public _startup
_startup:

	mov r0, 1111111

	lea sp, [_stackH]
	fpush {f0-f15}
	fpop {f0- f15}

	bl _fooFunc
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
	_stackL:
	.zero 1024
	_stackH:

	_hello:
	.string "Hello World!"
	_hello2:
	.string "Hello World!"
	_regs:
	.zero 204


