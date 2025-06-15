.text


; Generic syscall function,
;
; r0 - system call number
; r1 - Input : Pointer to whatever struct we want to pass to the kernel
; r2 - Output : pointer to array of 4 words 
public _app_syscallGeneric:
	enter 0
	pushm {r4}
	
	mov r4, r0 ; save the syscall number
	mov r0, r1 ; set the input
	mov r1, r2 ; set the output
	swi r4
	
	leave
	ret 0
