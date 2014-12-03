.text

;
; Returns the system running time in seconds (How long it's been running since
; boot )
; In:
;	None
; Out:
;	f0 - Running time in seconds
public _clk_getRunningTimeSeconds
_clk_getRunningTimeSeconds:
	; Clock is at bus 1
	; Clock function 0 gives us the running time
	mov ip, (1<<24) | 0
	hwi
	; Running time in seconds is already in f0, so return
	mov pc, lr
