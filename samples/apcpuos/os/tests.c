//#include "appslist.h"
//#include "kernel/kerneldebug.h"


float testArray[16];
int testArray2[6];

int params[4];

float callFloats(float a, float b, float c, float d)
{
	return a+b+c+d;
}

int someInt;
float stressFloats(int a, int b, int c, int d)
{
	float f0 = testArray[0];
	float f1 = testArray[1];
	float f2 = testArray[2];
	float f3 = testArray[3];
	float f4 = testArray[4];
	float f5 = testArray[5];
	float f6 = testArray[6];
	
	params[0] = a;
	params[1] = b;
	params[2] = c;
	params[3] = d;
	callFloats(1,2,3,4);
	
	int i0 = testArray2[0];
	int i1 = testArray2[1];
	int i2 = testArray2[2];
	int i3 = testArray2[3];
	int i4 = testArray2[4];

	someInt = i0+i1+i2+i3+i4;
	return f0+f1+f2+f3+f4+f5+f6;
}
