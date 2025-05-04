#include "menu.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "util.h"

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
 *  a title (string) and a function(ptr) which runs the option.
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
		return ERROR;
	}

	/* Holds the optioncount in menu directly through int pointer 
		i realize this is very extra, I only did it to be fancy :P */ 
	piOptionCount = &(pMenu->iOptionCount);

	/* If MENU is empty */
	if(*piOptionCount == 0){

		/* Allocate pointer pointer */
		pMenu->pOptions = (OPTION **) malloc(sizeof(OPTION *));
		if(pMenu->pOptions == NULL){
			return ERROR;
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

	return OK;
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

	return OK;
}

/*
 *	Executes an options given action by indexing
 *	out of bounds selections is handled by StartMenu
 * */
static int _ExecuteAction(MENU pMenu, int iSelection){
	pMenu.pOptions[iSelection - 1]->vfAction(pMenu.vpStruct);
	return OK;
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
	
	printf("\n\n");
	printf("%s\n\n", "________________________________________________________");

	return OK;
}

/*
 * Starts the menu and handles the runtime of the program.
 *	Takes a string as program name to display the title of the program.
 * */
int StartMenu(MENU *pMenu, char szProgramName[]){
	/* Declaring variables, as well as a guiding description */
	int iSelection;
	char szDesc[] = "Below is a list of options.\nYou select an option by entering its number in the terminal.";
	char *pszUserInput = NULL; 

	/* Starts by clearing the terminal */
	system("clear");

	/* Printing title and description */
	printf("\n\nWelcome to %s!\n\n", szProgramName);
	printf("%s\n", szDesc);

	/* Displays options */
	_DisplayOptions(*pMenu);

	/* Allocates to input buffer */
	pszUserInput = (char*) malloc(MAX_BUFFER);
	if(pszUserInput == NULL){
		berror("Failed malloc to pszUserInput");
		return ERROR;
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
		return OK;

	/* Otherwise it executes the selection's action */
	} else {
		system("clear");
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

	return ERROR;
}

