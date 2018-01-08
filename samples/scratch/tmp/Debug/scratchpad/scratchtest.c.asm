.sourcefile "C:\Work\crazygaze\G4All\G4Devkit\samples\scratch\scratchpad\scratchtest.c.asm"
; That first line above is used by the assembler to know from where the assembly came from,
; and it's used for debugging

.text

	; FUNCTION BEGIN - fooFunc 
	;(maxpushed=0, rtempsavesize=0, localsize=8, rsavesize=0 
	public	_fooFunc
_fooFunc:
	sub sp, sp, 8 ; grow stack to hold our local storage (variables and such)

.L3: ; IC start
	ldr r0,[sp + 4] ; Line 539

	add sp, sp, 8 ; remove stack space we used for local storage
	mov pc, lr
	.L4: ; end of function fooFunc
	; Debug entries 
.section ".apcpudebug"
	.string "fooFunc"
	.string "C:\Work\crazygaze\G4All\G4Devkit\samples\scratch\scratchpad\scratchtest.c"
	.word 1 ; type
	.word 8 ; flags
	.word 10 ; line
	.word _fooFunc ; address 
	.word .L4 - _fooFunc ; size
	.word 1; Number of ic mappings
		.word 12 ; line 
		.word .L3 ; position 
