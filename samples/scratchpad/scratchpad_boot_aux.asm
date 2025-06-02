.text

; Execute application
extern _main
call _main
mov r0, -1
;sdiv r0, r0, 0
; Disable IRQ raising, and halt
mov r0, 0
setcr crirqmsk, r0
hlt

.rodata
public _gROMInfo;
_gROMInfo:
.zero 32

