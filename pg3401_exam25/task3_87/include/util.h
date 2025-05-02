#ifndef ___UTIL_H___
#define ___UTIL_H___

#define MAX_STRING_SIZE 10
#define MAX_INPUT 1024

int StrncpyLowercase(char szLowerString[], char szString[], int iMaxBuffer);
int ParsePositiveInteger(char *psz);
int GetInput(char *szArgMessages[], char *szTypeFlags, ... );

#endif /*ndef ___UTIL_H___ */
