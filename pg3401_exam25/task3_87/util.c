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
int GetInput(int iArgC, char *szArgMessages[], char szTypeFlags[], ...){
	/* Declaring variables */
	va_list vaPointers; 
	int iStatus, iBuffer, i;
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

         /* Loads address to store data */
			pszArg = va_arg(vaPointers, char**);

         /* Prompts for input */
			fgets(*pszArg, MAX_INPUT, stdin);

         /* Removes \n from string (with \r just in case) */
			(*pszArg)[strcspn(*pszArg, "\r\n")] = 0;
		
		/* If type is int */
		} else if(szTypeFlags[i] == 'I'){
			piArg = va_arg(vaPointers, int *);

			/* Loops until we get correct input */
			while(1){
				/* Allocate to buffer */
				pszBuffer = (char *) malloc(MAX_INPUT);
				if(pszBuffer == NULL){
					iStatus = ERROR;
					free(pszBuffer);
					break;
				}
				/* Print message to terminal */
				printf("%s\n", szArgMessages[i]);

				/* Get user input, load into buffer first */
				fgets(pszBuffer, MAX_INPUT, stdin);
				/* Remove newline from pressing enter */
				pszBuffer[strcspn(pszBuffer, "\r\n")] = 0;

				/* Attempt to convert buffer into integer */
				if((iBuffer = ParsePositiveInteger(pszBuffer)) > -1){
					*piArg = iBuffer;
					free(pszBuffer);
					break;
				}
				free(pszBuffer);

			}
		} else {
			iStatus = ERROR;
			break;
		}
	}

	iStatus = OK;
   /* Ending va_list */
	va_end(vaPointers);		

	/* Cleanup */
	pszArg = NULL;
	pszBuffer = NULL;
	piArg = NULL;
	return iStatus;
} 

/*
 * Takes a string and attempts to convert it to a positive integer
 * Returns a positive integer on success, negative on fail
 * */
int ParsePositiveInteger(char *psz){
	int iNum, iLen, i;

	iLen = strlen(psz);
   /* Check ceiling */
	if(iLen >= MAX_STRING_SIZE){
		berror("Input is too large. (MAX: 128 bytes)");
		return -1;	
	}

	/* Check each char if digit */
	i = 0;
	while(i < iLen){
		if(isdigit(psz[i]) == 0){
			return -1;	
		}
		i++;
	}

   /* Uses atoi to convert it */
	iNum = atoi(psz);

	return iNum;
}

