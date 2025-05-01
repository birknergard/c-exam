#include "util.h"

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "./include/debug.h"


int ParsePositiveInt(char *psz){
	int iNum, iLen, i;
	char *pszBuffer = NULL;

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
;

	iNum = atoi(psz);

	free(pszBuffer);
	pszBuffer = NULL;
	
	return iNum;
}
