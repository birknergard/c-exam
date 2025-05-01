#include "util.h"

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "./include/debug.h"

int StrncpyLowercase(char *szLowerString[], char szString[], int iMaxBuffer){
	char *pszStringBuffer = NULL;  
	int iLength, i;

	iLength = strlen(szString);
	if(iLength > iMaxBuffer){
		berror("STRING EXCEEDS MAX BUFFER");
		return 1;
	} 

	pszStringBuffer = (char *) malloc(iLength);
	if(pszStringBuffer == NULL){
		berror("StrncpyLowercase(): String buffer allocation failed");
		return 1;
	}

	for(i = 0; i < iLength; i++){
		pszStringBuffer[i] = tolower(szString[i]);	
	}

	strncpy(*szLowerString, pszStringBuffer, iMaxBuffer);
	(*szLowerString)[iLength] = '\0'; 

	free(pszStringBuffer);

	return 0;
}


int ParsePositiveInt(char *psz){
	int iNum, iLen, i;

	if(strlen(psz) >= MAX_STRING_SIZE){
		berror("Input is too large. (MAX: 128 bytes)");
		return -1;	
	}

	/* Check each char if digit */
	i = 0;
	iLen = strlen(psz);
	while(i < iLen){
		if(isdigit(psz[i]) == 0){
			return -1;	
		}
		i++;
	}

	iNum = atoi(psz);

	return iNum;
}

