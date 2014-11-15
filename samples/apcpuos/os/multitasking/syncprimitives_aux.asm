.text

;
; Lock and unlock mutex
; Based on http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dht0008a/CJHBGBBJ.html
;
; In:
;	r0 - Pointer to mutex
; Out:
;	none
; Deprecated. A better solution is provided with cmpxchg
;public _mtx_spinlock
;_mtx_spinlock:
;	mov r2, 1
;	swp r1, r2, [r0] ; Swap r2 with location [r0], [r0] value placed on r1
;	cmp r1, r2 ; Check if memory value was locked (1)
;	beq _mtx_spinlock	; if was locked means it didn't acquired, so repeat
;	mov pc, lr ; We acquired the mutex, so we're done



; In:
;	r0 - Pointer to mutex
; Out:
;	none
;public _mtx_unlock
;_mtx_unlock:
;	str [r0], 0
;	mov pc, lr
	