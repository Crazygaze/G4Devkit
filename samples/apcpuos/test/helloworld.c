#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "hw/hwscreen.h"
#include "hw/hwcpu.h"

#include "syscalls.h"
#include "kernel/kerneldebug.h"
#include "osmem.h"

typedef struct {
	float a;
	double b;
	int ia;
	int ib;
} st;

st testFloats(float a, double b)
{
	st s;
	s.a = a+b;
	s.b = a*b;
	s.ia = (int)a+(int)b;
	s.ib = (int)a*(int)b;
	return s;
}

float fadd(float a, double b)
{
	return a+b;
}

int test2(int p1, int p2, int p3, int p4, int p5, int p6)
{
	p3 = 0;
	p4 = 0;
	p2 = 0;
	return p1+p5+p6;
}

/*
float getFloat(void)
{
	return 10.5f;
}
*/

void crash(void)
{
	unsigned char *ptr = 0;
	*ptr = 0x38;
}
void crashAt(int addr)
{
	unsigned char* ptr =0;
	ptr += addr;
	*ptr = 0x38;
}

void func2(int p1, int p2, int p3, int p4, int p5, int p6, int p7)
{
	p1 = 0;
	p2 = 0;
	p3 = 0;
	p4 = 0;
	p5 = 0;
	p6 = 0;
	p7 = 0;
}

void func1(int p1, int p2, int p3, int p4, int p5, int p6, int p7)
{
	int v1,v2;
	v1 = p1+p2;
	v2 = p1+p2;
	func2(v1,v2,p3,p4,p5,p6,p7);
}

void testvar(int p1, ...)
{
   int *varg = (&p1);
   hw_scr_printNumberAtXY(10,10, *varg++);
   hw_scr_printNumberAtXY(10,11, *varg++);
   hw_scr_printNumberAtXY(10,12, *varg++);
   hw_scr_printNumberAtXY(10,13, *varg++);
}


void testfloats(int p1, int p2, int p3, double p4)
{
   hw_scr_printNumberAtXY(10,10, p1);
   hw_scr_printNumberAtXY(10,11, p2);
   hw_scr_printNumberAtXY(10,12, p3);
   hw_scr_printFloatNumberAtXY(10,13, p4);
}

const int bufsize = 512;
int main(void)
{
	char buf[512];
	char buf2[512];
	unsigned int counter=0;
	char* testbuf = "*Hi there. This is from a SWI call*";
	hw_scr_printAtXY(0,0, "Hi there! Hello World!");
	KERNEL_DEBUG("Hello World!");
	//KERNEL_DEBUG("Hello World2 %s %d %f", "!!", 10, 10.5f);
	//KERNEL_DEBUG("Test %s %d %f", "!!", 10, 12345.70f);
	KERNEL_DEBUG("Test %s %d %f", "!!!",10,12345.70f);
	KERNEL_DEBUG("Before...");
	switest(0,24, (unsigned int)(testbuf));
	//crash();
	KERNEL_DEBUG("After...");
	
/*
	s = testFloats(2.0f,3.5f);
	printNumberAtXY(0,4, s.ia);
	printNumberAtXY(20,4, s.ib);
	printFloatNumberAtXY(0 ,5, s.a);
	printFloatNumberAtXY(20,5, s.b);
	printFloatNumberAtXY(20,6, fadd(1.5,2.0));

	printAtXY(25,10, "Hi there! Hello World!");
	printAtXY(25, 12, "Hi there Dopefish!!!");
	printAtXY(25, 13, "Just pulled a Dopenade!!!");
	
	printNumberAtXY(0,0, test2(1,2,3,4,5,6));
*/
	/*
	stringf(buf, "Hello %s. %f, %f", "Rui", 1.5f, 1.5f);
	*/
	sprintf(buf, "Hello %d %f %f", 1, 1.5f, 2.5f);
	hw_scr_printAtXY(0,14, buf);	
	sleep(500);
	sprintf(buf2, "*Hello %d %f %f*", 1, 1.5f, 2.5f);
	memcpy(buf, 0, bufsize);
	memcpy(buf, buf2, bufsize);
	memcpy(buf, buf2, sizeof(buf)-1);
	hw_scr_printAtXY(0,14, buf);
	sleep(500);

	//
	// Memory allocations
	//
	/*
	hw_cpu_disableIRQ();
	{
		KERNEL_DEBUG("Original");
		os_debugmem();
		
		int numallocs=5;
		int i;
		void** allocs = os_malloc_array(void*, numallocs);
		for(i=0; i<numallocs; i++) {
			allocs[i] = os_malloc(12);
			KERNEL_DEBUG("Alloc - 0x%X", allocs[i]);
		}

		os_debugmem();

		KERNEL_DEBUG("After..");
		os_free(allocs[1]);
		KERNEL_DEBUG("Cleaned 1..");
		os_debugmem();
		
		os_free(allocs[0]);
		KERNEL_DEBUG("Cleaned 0..");
		os_debugmem();
		
		os_free(allocs[2]);
		KERNEL_DEBUG("Cleaned 2..");
		os_debugmem();
		
		os_free(allocs[3]);
		os_free(allocs[4]);
		os_free(allocs);
		KERNEL_DEBUG("All clear..");
		os_debugmem();
	}
	hw_cpu_enableIRQ();
	*/


	while(1) {
		hw_scr_printNumberAtXY(0,15, counter);
		counter++;

		if (counter>10000000) {
			crash();
		}
			
		sleep(100);
	}
	
	//testvar(1,2,3,4);
	//testfloats(1,2,3,4.5f);
	sleep(2000);
	crash();	
	
	//sprintf(buf, "%s %d", "Hi there", 5, 6, 7, 8.0);
	sleep(100000);
	crash();
	sleep(1000);
	hw_scr_clear();
	sleep(1000);

	return 14;
}


