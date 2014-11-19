
1. Coding style

Coding style for the OS code is as bellow, except where the code came from some
public library.

1.1 Naming conventions

Data types use camel case. E.g:
	
	typedef struct MyStruct {
	
	} MyStruct;
	
Variables and functions use camel case but with the first letter in lower case.
This makes it easier to differentiate from types. Ee.g:
	
	MyStruct* myStruct;
	void someFunction(void);
	
The exception is when those functions or data types are part of the standard C
library. E.g: ( uint8_t, printf, etc)

Due to the way the C compiler and assembler work together, symbols get an added
'_' prefix. E.g:
	
	// If this is defined in C...
	int someVar;
	
	# In assembler it will exist as...
	_someVar:


Underscores are used to simulate the equivalent of C++ namespaces, for both
functions and data types, except for private code.
This helps splitting the code into modules. E.g:

	typedef bool (*hw_clk_TimerFunc)(void* userdata);
	double hw_clk_getRunningTimeMs32();
	

1.2 Braces and spaces

Indentation is 4 spaces.

Opening braces go on the same line. E.g:
	
	// Correct
	if (x) {
		// do something
	}
	
	// Wrong
	if (x)
	{
		// do something
	}
	
This saves precious vertical screen space. It applies for all non-function 
statment blocks (if, switch, for, while, do, etc)
The closing brace goes on a line of its own, except when followed by a
continuation of the same statement. E.g:

	if (x) {
		// do something
	} else {
		// do something
	}
	
For functions both opening and closing braces go on its own line. This helps
differentiate functions from other statements. E.g:

	void someFunction(void)
	{
		// Do something
	}


