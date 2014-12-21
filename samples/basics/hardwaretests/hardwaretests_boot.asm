.text
.word _hardwareTestsBoot
.word _interruptHandlerASM

_interruptCtx:
.zero 196

_hardwareTestsBoot:	
	extern _setupAppCtx
	bl _setupAppCtx;
	ctxswitch [r0];

_interruptHandlerASM:
	extern _interruptHandler
	bl _interruptHandler;
