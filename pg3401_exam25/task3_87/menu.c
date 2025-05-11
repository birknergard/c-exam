#include "menu.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "debug.h"

/*
 * Internal function for creating menu options. Takes a string and a function pointer.
 * */
static OPTION *_CreateOption(char szTitle[], void (*vfAction)(void *vpStruct)){

	/* Declare variables */
	OPTION *pNewOption = NULL;
	int iTitleLength = strlen(szTitle);

	/* Check that title is within buffer */
	if(iTitleLength > TITLE_BUFFER){
		berror("Title exceeds buffer.");
		return NULL;
	}

	/* Allocates option pointer */
	pNewOption = (OPTION *) malloc(sizeof(OPTION));
	if(pNewOption == NULL){
		berror("Malloc failed for newOption *.");
		return NULL;	
	}

	/* Allocates options title pointer. Since we checked length earlier
 		we dynamically allocate with iTitleLength */
	pNewOption->pszTitle = (char *) malloc(iTitleLength + 1);
	if(pNewOption->pszTitle == NULL){
		berror("Malloc failed for newOptions->pszTitle.");
		free(pNewOption);
		pNewOption = NULL;
		return NULL;
	}

	/* Copies the given title into the allocated pointer and null terminates it */
	strncpy(pNewOption->pszTitle, szTitle, iTitleLength);
	pNewOption->pszTitle[iTitleLength] = '\0';

	pNewOption->vfAction = vfAction; 

	return pNewOption;
}

/*
 * Creates an empty menu struct. 
 * is destroyed completely by DestroyMenu
 * */
MENU *CreateMenu(void *vpStruct){
	MENU *pNewMenu = NULL;	

	/* Allocates pointer */
	pNewMenu = (MENU *) malloc(sizeof(MENU));
	if(pNewMenu == NULL){
		berror("Malloc to pNewMenu failed.");
		return NULL;
	}

	/* Assigns an anonymous state struct (void *) */
	pNewMenu->vpStruct = vpStruct;

	/* Sets default options to NULL (we allocate when we add it later) */
	pNewMenu->pOptions = NULL;

	/* Sets option count tracker to 0 */
	pNewMenu->iOptionCount = 0;

	return pNewMenu;
}

/*
 *  Adds an option to the menu. 
 *  It requires the menu struct as a pointer, 
 *  a title (string) and a function(ptr) which runs the options
 * */
int AddOption(MENU *pMenu, char szTitle[], void (*vfAction)(void *vfStruct)){
	/* Declaring variables */
	OPTION *pNewOption = NULL;
	OPTION **ppExtendedOptions = NULL;

	/* Integer pointer to hold menu option count later */
	int *piOptionCount = NULL; 

	/* Creates a new option, checks if valid (function takes care of logging) */ 
	pNewOption = _CreateOption(szTitle, vfAction);
	if(pNewOption == NULL){
		return 1;
	}

	/* Holds the optioncount in menu directly through int pointer 
		i realize this is very extra, I only did it to be fancy :P */ 
	piOptionCount = &(pMenu->iOptionCount);

	/* If MENU is empty */
	if(*piOptionCount == 0){

		/* Allocate pointer pointer */
		pMenu->pOptions = (OPTION **) malloc(sizeof(OPTION *));
		if(pMenu->pOptions == NULL){
			return 1;
		}
		/* Bitwise copy initial option address to new option */
		pMenu->pOptions[0] = pNewOption;

	/* If menu contains previous elements */
	} else {

		/* Creating extended pointer */
		ppExtendedOptions = (OPTION **) malloc(sizeof(OPTION *) * (*piOptionCount + 1));

		/* Copying old data to new ptr */
		memcpy(ppExtendedOptions, pMenu->pOptions, sizeof(OPTION *) * (*piOptionCount));

		/* Inserting new data to new ptr */
		ppExtendedOptions[*piOptionCount] = pNewOption;

		/* Deleting old data */
		free(pMenu->pOptions);

		/* Reassigning main ptr to new one with added data */
		pMenu->pOptions = ppExtendedOptions;

		/* Remove dangling pointer */
		ppExtendedOptions = NULL;
	}

	/* Dereferences the int pointer and increments the option count */
	(*piOptionCount)++;

	/* Cleanup */
	piOptionCount = NULL;
	pNewOption = NULL;

	return 0;
}

/*
 *  Frees all memory associated with menu and removes dangling pointers.
 * */
int DestroyMenu(MENU *pMenu){
	int i;
	OPTION *pCurrOption = NULL;

	/* Checks every option* in the option* array and frees the title and pointer */
	if(pMenu->iOptionCount > 0){
		for(i = 0; i < pMenu->iOptionCount; i++){
			pCurrOption = pMenu->pOptions[i]; 
			if(pCurrOption != NULL){
				free(pCurrOption->pszTitle);
				free(pCurrOption);
			}
		}
	}

	/* Removing dangling pointers */
	pCurrOption = NULL;

	/* Freeing the rest of the menu */
	free(pMenu->pOptions);
	free(pMenu);

	return 0;
}

/*
 *	Executes an options given action by indexing
 *	out of bounds selections is handled by StartMenu
 * */
static int _ExecuteAction(MENU pMenu, int iSelection){

	pMenu.pOptions[iSelection - 1]->vfAction(pMenu.vpStruct);
	return 0;
}

/*
 *  Generates menu UI, unfortunately I did not have time to make it prettier.
 * */
static int _DisplayOptions(MENU pMenu){
	int i;
	printf("%s\n\n", "________________________________________________________");
	for(i = 0; i < pMenu.iOptionCount; i++){
		printf("%2d) %s\n\n", i + 1, pMenu.pOptions[i]->pszTitle);
	}
	/* EXIT is a default option, and so it is displayed regardless */
	printf("%2d) %s\n", 0, "EXIT");
	
	printf("\n");
	printf("%s\n", "________________________________________________________");

	return 0;
}

/*
 * Starts the menu and handles the runtime of the program.
 *	Takes a string as program name to display the title of the program.
 * */
int StartMenu(MENU *pMenu, char szProgramName[]){
	/* Declaring variables, as well as a guiding description */
	int iSelection;
	char *pszUserInput = NULL; 

	/* Starts by clearing the terminal */
	system("clear");


	/* Displays options */
	printf("%s\n", szProgramName);
	_DisplayOptions(*pMenu);
	printf("ENTER YOUR SELECTION (1 -> %d) or 0 to quit.\n", pMenu->iOptionCount);

	/* Allocates to input buffer */
	pszUserInput = (char*) malloc(MAX_BUFFER);
	if(pszUserInput == NULL){
		berror("Failed malloc to pszUserInput");
		return 1;
	}

	/* Enters infinite loop */
	for(;;){

		/* Prompts the user for input */
		fgets(pszUserInput, MAX_BUFFER, stdin);

		/* Removes newline from input */
		pszUserInput[strcspn(pszUserInput, "\r\n")] = 0;

		/* Utilizes a function from "util.h" to convert input from string to positive integer */
		iSelection = ParsePositiveInteger(pszUserInput);

		/* Verifies that the input is within selection range, 
 			continues prompting until it is */
		if(iSelection < 0 || iSelection > pMenu->iOptionCount){
			printf("Invalid selection. Please try again ...\n");
		} else {
			/* If input is accepted the loop exits */
			break;
		}
	}

	/* Cleans up user input pointer */
	free(pszUserInput);
	pszUserInput = NULL;

	/* If the selected number is 0 the program always exits */
	if(iSelection == 0){
		printf("\nExiting ...\n");
		return 0;

	/* Otherwise it executes the selection's action */
	} else {
		system("clear");

		printf("%s\n", szProgramName);
		printf("%s\n\n", "________________________________________________________");
		_ExecuteAction(*pMenu, iSelection);	

		/* Once the function is completed, prompt the user for input to return to menu ... */
		printf("\nAction completed. Press enter to return to menu.\n");

		/* ... using getchar. NOTE: I realize this could have been used above too */
		char cInput = getchar();
		while(cInput != '\n'){
			cInput = getchar();
		}
		
		/* Since selection is not 0, we restart the program, 
 			effectively looping until the user chooses to exit */
		StartMenu(pMenu, szProgramName);
	}

	return 1;
}

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
	int iStatus = 0;
	int iBuffer, i;
	int *piArg = NULL;
	char **pszArg = NULL, *pszBuffer = NULL;

	/* Start variadic args */
	va_start(vaPointers, szTypeFlags);

	/* Run for each argument */
	for(i = 0; i < iArgC; i++){

		/* Allocates buffer */
		pszBuffer = (char *) malloc(MAX_INPUT);
		if(pszBuffer == NULL){
			iStatus = 1;
			free(pszBuffer);
			break;
		}
		pszBuffer[MAX_INPUT - 1] = '\0';

		/* If type is string */
		if(szTypeFlags[i] == 'S'){

			/* Prints message */
			printf("%s\n", szArgMessages[i]);

			/* Loads address to store data */
			pszArg = va_arg(vaPointers, char**);

			/* Prompts for input */
			fgets(pszBuffer, MAX_INPUT, stdin);
			char c;

			int i;
			for(i = 0; i < strlen(pszBuffer); i++){
				c = pszBuffer[i];

				/* NOTE: Supposed to restrict from using any characters but regular letters and number.
				 * However this is buggy and inconsistent. Not entirely sure why. */
				if(!((58 > c && c > 47) ||
					(91 > c && c > 64) ||
					(123 > c && c > 96))){

					
					return 1;
				} 
			}

			/* Removes \n from string (with \r just in case) */
			pszBuffer[strcspn(pszBuffer, "\r\n")] = 0;

			*pszArg = pszBuffer;

			/* If type is int */
		} else if(szTypeFlags[i] == 'I'){
			piArg = va_arg(vaPointers, int *);

			/* Loops until we get correct input */
			while(1){
				/* Allocate to buffer */
				pszBuffer = (char *) malloc(MAX_INPUT);
				if(pszBuffer == NULL){
					iStatus = 1;
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
					break;
				}
			}
		} else {
			iStatus = 1;
			break;
		}
	}

	/* Ending va_list */
	va_end(vaPointers);		

	/* Cleanup */
	pszArg = NULL;

	free(pszBuffer);
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

