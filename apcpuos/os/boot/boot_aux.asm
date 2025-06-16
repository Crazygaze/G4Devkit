;*******************************************************************************
; This file is set as the "boot" file for the linker, which places it as the 
; start of the ROM.
;
; Reponsibilities of this file include
; * Setup bare minimum that needs to be done in assembly
; * Call the entry point C function(s) to run the OS
;*******************************************************************************

.text

;
; We want deferencing of NULL pointers to get detected, so we need to make
; sure that any Read, Write or Execute of a NULL pointer causes a CPU exception
;
; The Read and Write are taken care automatically, because the first page is
; marked as Execute only by the OS.
; But that still leaves the problem of making sure that we can't execute the
; instruction at address 0.
;
; We solve this by patching address 0 with "ldrsb r0, [pc - 5]", which will
; always cause a CPU exception.
; * [pc - 5], because when executing address 0, pc has a value of 4, and so
; "pc - 5" gives us address 0xFFFFFFFF
;
; Encoding notes:
;
; \3\3\2\2\2\2\2\2\ \2\2\2\2\1\1\1\1\ \1\1\1\1\1\1\ \ \ \ \ \ \ \ \ \ \ \ 
; \1\0\9\8\7\6\5\4\ \3\2\1\0\9\8\7\6\ \5\4\3\2\1\0\9\8\ \7\6\5\4\3\2\1\0\
;                                                                       
;  0 0 1 0 1 1 [ Offset (imm18)                      ]   [Rdst ] [Rbase] - ldrsb
;
; Thefore a "ldrsb r0, [pc - 5]" instruction encoded as 0x2ffffb0f

nop ; do nothing

; Patch instruction at address 0, so any attempt to execute addr 0 now causes
; a cpu exception
mov r0, 0
str [r0], 0x2ffffb0f

;
; Start the actual OS code
; 

extern _mmu_preInit
extern _krn_init
extern _krn_irqHandler

	;
	; Setup's the kernel stack and ds register
	;
	; Returns the kernel's new sp value
	call _mmu_preInit
	mov sp, r0

;
; Context switching loop
; The loop is simply serving interrupts one after another.
; When an interrupt happens, we ask the kernel to handle it, and it returns back
; what context to resume
;
	
	; Initialize the kernel.
	; It returns the first context to execute.
	call _krn_init
	
	.L0:
	; crtsk is set by krn_init. This register contains the context
	; to switch to whenever an interrupt happens.
	getcr r1, crtsk
	fullctxswitch [r0], [r1] ; save current ctx to [r1] and load [r0]
	
	; When an interrupt happens and execution is transfered back to us,
	; r0, r1, r2, r3 are set with information, and thus we can pass those to the
	; C function as-is
	; krn_irqHandler returns the context to resume
	call _krn_irqHandler
	b .L0
	
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
;	4 bytes - dataSize (size of .data + .bss)
;	4 bytes - dataSharedAddr (where .data_shared starts)
;	4 bytes - dataSharedSize (size of .data_shared + .bss_shared)
public _gROMInfo
_gROMInfo:
.zero 32

; Intentionally putting this in the .rodata section because it's easier
; for the kernel to set when switching threads, since .rodata is shared between
; all processes and it doesn't required address translation
public _appTlsSlots
_appTlsSlots:
.word 0

