#ifndef _hwcrt0_stdc_h_
#define _hwcrt0_stdc_h_

////////////////////////////////////////////////////////////////////////////////
// contents typically in stddef.h
////////////////////////////////////////////////////////////////////////////////

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

#define FALSE 0
#define TRUE 1

#define STRINGIFY(a) STRINGIFY_(a)
#define STRINGIFY_(a) #a

// This is so we define inline VBCC style assembly functions,
// without the IDE's syntax parser throwing errors.
// Otherwise we would need separate blocks with #ifdef __syntax_parser__
#ifdef __syntax_parser__
	#define __reg(x)
	#define INLINEASM(str) { return 0; }
#else
	#define INLINEASM(str) =str
#endif

////////////////////////////////////////////////////////////////////////////////
// contents typically in stdint.h
////////////////////////////////////////////////////////////////////////////////

typedef unsigned char uint8_t;
typedef unsigned char u8;
typedef signed char int8_t;
typedef signed char s8;

typedef unsigned short uint16_t;
typedef unsigned short u16;
typedef signed short int16_t;
typedef signed short s16;

typedef unsigned int uint32_t;
typedef unsigned int u32;
typedef signed int int32_t;
typedef signed int s32;

typedef long intmax_t;
typedef unsigned long uintmax_t;

typedef unsigned long uintptr_t;

//
// Fastest signed/unsigned integer types with width of at least N bytes.
//
typedef uint32_t uint_fast8_t;
typedef uint32_t uint_fast16_t;
typedef uint32_t uint_fast32_t;

typedef int32_t int_fast8_t;
typedef int32_t int_fast16_t;
typedef int32_t int_fast32_t;

//
// Smallest signed/unsigned integer types with width of at least N bytes
//
typedef uint8_t  uint_least8_t;
typedef uint16_t uint_least16_t;
typedef uint32_t uint_least32_t;

typedef int8_t  int_least8_t;
typedef int16_t int_least16_t;
typedef int32_t int_least32_t;

////////////////////////////////////////////////////////////////////////////////
//                   Contents typically in stdbool.h
////////////////////////////////////////////////////////////////////////////////

typedef _Bool bool;
#define false 0
#define true 1


////////////////////////////////////////////////////////////////////////////////
//                   Contents typically in assert.h
////////////////////////////////////////////////////////////////////////////////

void _assert_impl(void)
INLINEASM("\t\
dbgbrk 0");

#ifdef NDEBUG
	// For Release build, asserts don't do anything
	#define assert(condition)
#else
	#define assert(condition) if (!(condition)) { _assert_impl(); }
#endif

////////////////////////////////////////////////////////////////////////////////
//                   Bare minimum C runtime functions
////////////////////////////////////////////////////////////////////////////////
int strlen(const char* str);

void* memset(void* dest, int c, int count)
INLINEASM("\t\
memset [r0], r1, r2");

// memcpy and memmove are the same for the APCPU
void* memcpy(void* dest, const void* src, int count)
INLINEASM("\t\
memcpy [r0], [r1], r2");

void* memmove(void* dest, const void* src, int count)
INLINEASM("\t\
memcpy [r0], [r1], r2");

#endif
