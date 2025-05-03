#include "menu.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "util.h"

/*
 * Private function for creating options. Takes a string and a function pointer.
 * */
static OPTION *_CreateOption(char szTitle[], void (*vfAction)(void *vpStruct)){
	OPTION *pNewOption = NULL;
	int iTitleLength = strlen(szTitle);

	if(iTitleLength > TITLE_BUFFER){
		berror("Title exceeds buffer.");
		return NULL;
	}

	pNewOption = (OPTION *) malloc(sizeof(OPTION));
	if(pNewOption == NULL){
		berror("Malloc failed for newOption *.");
		return NULL;	
	}

	pNewOption->pszTitle = (char *) malloc(iTitleLength + 1);
	if(pNewOption->pszTitle == NULL){
		berror("Malloc failed for newOptions->pszTitle.");
		free(pNewOption);
		pNewOption = NULL;
		return NULL;
	}

	strncpy(pNewOption->pszTitle, szTitle, iTitleLength);
	pNewOption->pszTitle[iTitleLength] = '\0';

	pNewOption->vfAction = vfAction; 

	return pNewOption;
}

/*
 * Creates an empty menu struct.
 * */
MENU *CreateMenu(void *vpStruct){
	MENU *pNewMenu = NULL;	

	pNewMenu = (MENU *) malloc(sizeof(MENU));
	if(pNewMenu == NULL){
		berror("Malloc to pNewMenu failed.");
		return NULL;
	}
	pNewMenu->vpStruct = vpStruct;
	pNewMenu->pOptions = NULL;
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

	/* Holds the optioncount in menu directly through int pointer */ 
	piOptionCount = &(pMenu->iOptionCount);

	/* If MENU is empty */
	if(*piOptionCount == 0){

		/* Allocate pointer */
		pMenu->pOptions = (OPTION **) malloc(sizeof(OPTION *));
		if(pMenu->pOptions == NULL){
			return ERROR;
		}
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

		ppExtendedOptions = NULL;
	}

	/* Dereferences the pointer and increments the option count */
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

	/* Frees every title in the option list */
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
	free(pMenu->pOptions);
	free(pMenu);

	return OK;
}

/*
 *	Executes an options given action by indexing
 * */
int ExecuteAction(MENU pMenu, int iSelection){
	pMenu.pOptions[iSelection - 1]->vfAction(pMenu.vpStruct);
	return OK;
}

/*
 *  Generates menu UI
 * */
int DisplayOptions(MENU pMenu){
	int i;
	printf("%s\n\n", "________________________________________________________");
	for(i = 0; i < pMenu.iOptionCount; i++){
		printf("%2d) %s\n\n", i + 1, pMenu.pOptions[i]->pszTitle);
	}
		printf("%2d) %s\n", 0, "EXIT");
		printf("\n\n");
	printf("%s\n\n", "________________________________________________________");

	return OK;
}

int StartMenu(MENU *pMenu, char szProgramName[]){
	int iSelection;
	char szDesc[] = "Below is a list of options.\nYou select an option by entering its number in the terminal.";

	char *pszUserInput = NULL; 

	system("clear");
	printf("\n\nWelcome to %s!\n\n", szProgramName);
	printf("%s\n", szDesc);

	DisplayOptions(*pMenu);

	pszUserInput = (char*) malloc(MAX_BUFFER);
	if(pszUserInput == NULL){
		berror("Failed malloc to pszUserInput");
		return ERROR;
	}

	for(;;){
		fgets(pszUserInput, MAX_BUFFER, stdin);
		/* Removes newline from input */
		pszUserInput[strcspn(pszUserInput, "\r\n")] = 0;

		iSelection = ParsePositiveInteger(pszUserInput);

		if(iSelection < 0 || iSelection > pMenu->iOptionCount){
			printf("Invalid selection. Please try again ...\n");
		
		} else {
			break;
		}
	}

	free(pszUserInput);
	pszUserInput = NULL;

	if(iSelection == 0){
		printf("\nExiting ...\n");
		return OK;

	} else {
		system("clear");
		ExecuteAction(*pMenu, iSelection);	
		printf("\nAction completed. Press enter to return to menu.\n");

		char cInput = getchar();
		while(cInput != '\n'){
			cInput = getchar();
		}
		
		StartMenu(pMenu, szProgramName);
	}

	return ERROR;
}

