#ifndef SEVENZIPPP_CONSOLECALLBACK
#define SEVENZIPPP_CONSOLECALLBACK
#include <iostream>
#include <stdarg.h>  // For va_start, etc.
#include <stdio.h>
/*
 * Interface for Debugmessages
 */
struct ConsoleCallback {
public:
	virtual void PrintMessage(const char *string) = 0;
};

/*
 * string_format formats a string just like printf or strfmt does and returns it as a char buffer
 */
static const char* string_format(const std::string fmt, ...) {
	char* buffer = new char[256];
	va_list args;
	va_start (args, fmt);
	int n = vsnprintf (buffer,256,fmt.c_str(), args);
	buffer[n] = '\0';
	va_end (args);
	return buffer;
}
#endif