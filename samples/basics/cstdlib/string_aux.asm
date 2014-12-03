.text

public _memset
_memset:
	memset [r0], r1, r2
	mov pc, lr

public _memcpy
_memcpy:
	memcpy [r0], [r1], r2
	mov pc, lr

; Same as memcpy for the APCPU
public _memmove
_memmove:
	memcpy [r0], [r1], r2
	mov pc, lr