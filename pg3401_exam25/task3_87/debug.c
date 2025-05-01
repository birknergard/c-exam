#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define MAX_BUFFER 4096

void Debug(unsigned short usErrorType, int iLine, const char szFileName[], const char *szFormat, ...){
	va_list vaArgs;
	char szOutput[MAX_BUFFER + 1] = {0};
	const char *szType = (usErrorType == 0) ? "ERROR" : "DEBUG";

	va_start(vaArgs, szFormat);
	vsnprintf(szOutput, MAX_BUFFER, szFormat, vaArgs);
	va_end(vaArgs);

	printf("%10s in %s:%d ->> %s \n", szType, szFileName, iLine, szOutput);
}

