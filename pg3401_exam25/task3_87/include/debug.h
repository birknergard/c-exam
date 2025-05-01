#ifndef DEBUG_H

#define OK 0
#define ERROR 1

#define DEBUG_H

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

void InitializeDebugger();
void Debug(unsigned short usErrorType, int iLine, const char szFileName[], const char *szFormat, ...);

#define bdebug(...) Debug(1, __LINE__, __FILE__, __VA_ARGS__)
#define berror(...) Debug(0, __LINE__, __FILE__, __VA_ARGS__)

#endif /* ndef DEBUG_H */
