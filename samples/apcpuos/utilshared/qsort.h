#ifndef _utilshared_qsort_h_
#define _utilshared_qsort_h_

#include "utilsharedconfig.h"

void qsort(
	void *array, size_t count, size_t elementSize,
	int (*cmp)(const void *, const void *));


#endif



