#include "detail/stdc_internal.h"
#include "math.h"

int _isinfImpl(double arg)
{
	return arg == INFINITY;
}

int _isnanImpl(double arg)
{
	// Note: the `fcmp` instruction sets the Z flag to 0 if any of the operands
	// is a nan. This makes for an easy way to detect nan values
	return arg != arg;
}

//
// log2
//

#define M_LOG2E 1.442695040888963407359 // log2(e)
float log2f(float arg)
{
	return log(arg) * M_LOG2E;
}

double log2(double arg)
{
	return log(arg) * M_LOG2E;
}

