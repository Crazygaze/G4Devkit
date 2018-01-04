.text
.word _hardwareTestsBoot
.word _interruptHandlerASM

_interruptCtx:
.zero 204 ; Space for all the registers

; Variables in the C file
extern _interruptedCtx
extern _interruptBus
extern _interruptReason


_hardwareTestsBoot:	
	extern _setupAppCtx
	bl _setupAppCtx;
	ctxswitch [r0] ; switch to the context the handler tells us to

_interruptHandlerASM:
	str [_interruptedCtx], lr ; save interrupted context
	
	; Save the Bus and reason that caused the interrupt
	srl r4, ip, 24
	str [_interruptBus], r4;
	and r4, ip, 0x80FFFFFF
	str [_interruptReason], r4;
		
	extern _interruptHandler
	bl _interruptHandler
	ctxswitch [r0] ; switch to the context the handler tells us to
