;*******************************************************************************
;
;*******************************************************************************

.word _startup ; RESET handler

;
; Things we need from the C file
extern _appMain
extern _mmuTable
extern _calcMMUTableSize

_startup:
	call _calcMMUTableSize
	; Make room for the mmu table, in the stack
	sub sp, sp, r0
	str [_mmuTable], sp
	call _appMain

_bootLoop:
	hlt
	b _bootLoop
