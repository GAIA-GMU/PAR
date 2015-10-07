/*
 * vperror.c++
 *
 * See: Stroustrup: C++ (2nd ed.) p. 134.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "lwnet.h"

char* itoa(int n)
{
	char buf[BUFSIZ];

	sprintf_s(buf,BUFSIZ,"%d", n);
	char* s = new char[strlen(buf)+1];
	strcpy_s(s,BUFSIZ,buf);
	return s;
}

void errwarn(int severity ...)
{
	va_list ap;

	if(severity)
		std::cerr << "Error: ";
	else
		std::cerr << "Warning: ";

	va_start(ap, severity);
	for(char* p = va_arg(ap, char*); p != 0; p = va_arg(ap, char*))
		std::cerr << p << ' ';
	va_end(ap);

	std::cerr << '\n';

	if(severity)
		exit(severity);
}
