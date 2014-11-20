#ifndef _stdcdef_shared_h_
#define _stdcdef_shared_h_

typedef unsigned int size_t;
#ifndef __syntax_parser__
	#define offsetof(type,member) __offsetof(type,member)
#else
	// This is so the IDE syntax parser parses this correctly,
	// since clang's built in offsetof is __builtin_offsetof
	#define offsetof(type,member) __builtin_offsetof(type,member)
#endif

typedef int ptrdiff_t;
#define NULL 0L

typedef unsigned char bool;
#define false 0
#define true 1
#define FALSE 0
#define TRUE 1

// This is so we define inline VBCC style assembly functions,
// without the IDE's syntax parser throwing errors.
// Otherwise we would need seperate blocks with #ifdef __syntax_parser__
#ifdef __syntax_parser__
	#define __reg(x)
	#define INLINEASM(str)
#else
	#define INLINEASM(str) =str
#endif


#include "stdint_shared.h"

#include "stdcshared_defs.h"

#endif
