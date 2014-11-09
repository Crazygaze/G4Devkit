#ifndef _utilshared_staticassert_h_
#define _utilshared_staticassert_h_

//
// Static assert macros
//
#define CONCATENATE_IMPL(s1,s2) s1##s2
#define CONCATENATE(s1,s2) CONCATENATE_IMPL(s1,s2)

// Note: __COUNTER__ Expands to an integer starting with 0 and incrementing by 1 every time it is used in a source file or included headers of the source file.
#ifdef __COUNTER__
	#define ANONYMOUS_VARIABLE(str) \
		CONCATENATE(str,__COUNTER__)
#else
	#define ANONYMOUS_VARIABLE(str) \
		CONCATENATE(str,__LINE__)
#endif

/*
 * Example of usage:
 * STATIC_ASSERT( sizeof(Foo)==16 )
 *
 * Note: Due the the fact that in Release build the macro evaluates to nothing
 */
#ifdef NDEBUG
	#define STATIC_ASSERT(expr)
#else
	#define STATIC_ASSERT(expr) \
		static const char ANONYMOUS_VARIABLE(scheck) [ expr==1 ];
#endif

#endif
