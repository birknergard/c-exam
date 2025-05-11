#ifndef ___UTIL_H___
#define ___UTIL_H___


int StrncpyLowercase(char szLowerString[], char szString[], int iMaxBuffer);
int ParsePositiveInteger(char *psz);
int GetInput(int iArgC, char *szArgMessages[], char *szTypeFlags, ... );

#endif /*ndef ___UTIL_H___ */
