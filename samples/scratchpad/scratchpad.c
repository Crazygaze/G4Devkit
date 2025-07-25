typedef struct ROMInfo
{
	// .text start address and size in bytes
	int textAddr;
	int textSize;
	
	// .rodata start address and size in bytes
	int rodataAddr;
	int rodataSize;
	
	// (.data+.bss) start address and size in bytes
	int dataAddr;
	int dataSize;
	
	// (.data_shared+.bss_shared) start address and size in bytes
	int dataSharedAddr;
	int dataSharedSize;
} ROMInfo;

extern ROMInfo gROMInfo;

// This is so we define inline VBCC style assembly functions,
// without the IDE's syntax parser throwing errors.
// Otherwise we would need separate blocks with #ifdef __syntax_parser__
#ifdef __syntax_parser__
	#define __reg(x)
	#define INLINEASM(str) { return 0; }
#else
	#define INLINEASM(str) =str
#endif


ROMInfo* foo(void)
{
	return &gROMInfo;
}

void main(void)
{
	ROMInfo* rom = foo();
	gROMInfo.dataAddr = gROMInfo.dataAddr;
}



