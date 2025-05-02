#include "util.h"

#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "./include/debug.h"

/*
 * This function gets input from user and assigns it to given variable pointers
 *
 * iArgs is number of variables to set 
 * szArgMsg is an array of strings, where each string is the message to print when asking for input 
 * szTypeFlags is a string with each arguments type. compatible with *string and *int. example input = "SSII"
 * ... is the variables adresses, so that it assigns at the correct place. */
int GetInput(char *szArgMessages[], char *szTypeFlags, ...){
	/* Declaring variables */
	va_list vaPointers; 
	int iArgC = strlen(szTypeFlags), iStatus, iBuffer, i;
  	int *piArg = NULL;
	char **pszArg = NULL, *pszBuffer = NULL;

	/* Start variadic args */
	va_start(vaPointers, szTypeFlags);

	/* Run for each argument */
	for(i = 0; i < iArgC; i++){


		/* If type is string */
		if(szTypeFlags[i] == 'S'){
			/* Prints message */
			printf("%s\n", szArgMessages[i]);

			pszArg = va_arg(vaPointers, char**);
			fgets(*pszArg, MAX_INPUT, stdin);
		
		/* If type is int */
		} else if(szTypeFlags[i] == 'I'){
			piArg = va_arg(vaPointers, int*);

			/* Loops until we get correct input */
			while(piArg == NULL){
				/* Print message to terminal */
				printf("%s:\n", szArgMessages[i]);

				/* Get user input, load into buffer first */
				fgets(pszBuffer, MAX_INPUT, stdin);

				/* Convert buffer into integer */
				if((iBuffer = ParsePositiveInteger(pszBuffer)) > -1){
					*piArg = iBuffer;
				}
			}
		} else {
			va_end(vaPointers);		
			iStatus = ERROR;
			break;
		}
	}

	iStatus = OK;

	/* Removing danglers */
	pszArg = NULL;
	pszBuffer = NULL;
	piArg = NULL;
	return iStatus;
} 

int StrncpyLowercase(char szLowerString[], char szString[], int iMaxBuffer){

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

	strncpy(szLowerString, pszStringBuffer, iMaxBuffer);
	szLowerString[iLength] = '\0'; 

	free(pszStringBuffer);

	return 0;
}


int ParsePositiveInteger(char *psz){
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

