;*******************************************************************************
; Bare minimum boot file that calls C main
;*******************************************************************************

.text

; Execute application
extern _boot_startup

mov r0, sp ; Pass the stack pointer as the first parameter to boot_startup
call _boot_startup

; Disable IRQ raising, and halt
mov r0, 0
setcr crirqmsk, r0
hlt


.rodata
;
; Process runtime information
; This is patched by the linker when building a ROM file, 
; and information about the size of the program:
; Contents are:
; 	4 bytes - textAddr (where .text starts)
;	4 bytes - textSize (size of the .text)
;	4 bytes - rodataAddr ( where .rodata starts)
;	4 bytes - rodataSize (size of .rodata)
;	4 bytes - dataAddr (where .data starts)
;	4 bytes - dataSize (size of .data + .bss
;	4 bytes - dataSharedAddr (where .data_shared starts)
;	4 bytes - dataSharedSize (size of .data_shared + .bss_shared)
public _gROMInfo
_gROMInfo:
.zero 32
