int vara = 1;
int varb = 2;
int varc;

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

void hwcpu_set_ds(int val)
INLINEASM("\t\
mov r11, r0");
	

void derp(void);

int main(void)
{
	ROMInfo* rom = &gROMInfo;
	int* pa = &vara;
	int* pb = &varb;
	int* pc = &varc;
	
	hwcpu_set_ds(rom->dataSharedAddr);
	int* pa2 = &vara;
	int* pb2 = &varb;
	int* pc2 = &varc;
	
	while (1) {
	}
	return 0;
}

