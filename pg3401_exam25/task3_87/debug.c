#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define MAX_BUFFER 4096

/*
 * Debug function used in various places for easier debugging. Just a printf with extra info.
 * Made with heavy input from the Debugger task earlier in the year. (Exercises -> Leksjon 7) 
 * */
void Debug(unsigned short usErrorType, int iLine, const char szFileName[], const char *szFormat, ...){
	va_list vaArgs;
	char szOutput[MAX_BUFFER + 1] = {0};
	const char *szType = (usErrorType == 0) ? "ERROR" : "DEBUG";

	va_start(vaArgs, szFormat);
	vsnprintf(szOutput, MAX_BUFFER, szFormat, vaArgs);
	va_end(vaArgs);

	printf("%s in %s:%d ->> %s \n", szType, szFileName, iLine, szOutput);
}

