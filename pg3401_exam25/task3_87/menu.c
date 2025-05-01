#include "./include/menu.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "util.h"

/*
 * Private function for creating options. Takes a string and a function pointer.
 * */
OPTION *CreateOption(char szTitle[], void (*funcAction)()){
	OPTION *pNewOption = NULL;
	int iTitleLength = strlen(szTitle);

	pNewOption = (OPTION *) malloc(sizeof(OPTION));
	if(pNewOption == NULL){
		berror("Malloc failed for newOption *.");
		return NULL;	
	}

	pNewOption->pszTitle = (char *) malloc(TITLE_BUFFER);
	if(pNewOption->pszTitle == NULL){
		berror("Malloc failed for newOptions->pszTitle.");
	}

	if(iTitleLength >= TITLE_BUFFER){
		strncpy(pNewOption->pszTitle, szTitle, TITLE_BUFFER);
		pNewOption->pszTitle[TITLE_BUFFER] = '\0';

	} else {
		strncpy(pNewOption->pszTitle, szTitle, iTitleLength);
		pNewOption->pszTitle[iTitleLength] = '\0';
	}
	

	pNewOption->funcAction = funcAction;

	return pNewOption;
}

/*
 * Creates an empty menu struct.
 * */
MENU *CreateMenu(){
	MENU *pNewMenu = NULL;	

	pNewMenu = (MENU *) malloc(sizeof(MENU));
	if(pNewMenu == NULL){
		berror("Malloc to pNewMenu failed.");
		return NULL;
	}
	pNewMenu->pOptions = NULL;
	pNewMenu->iOptionCount = 0;

	return pNewMenu;
}

/*
 *  Adds an option to the menu. 
 *  It requires the menu struct as a pointer, 
 *  a title (string) and a function(ptr) which runs the option.
 * */
int AddOption(MENU *pMenu, char szTitle[], void (*funcAction)()){
	OPTION *pNewOption = NULL;
	OPTION **ppExtendedOptions = NULL;
	int *piOptionCount = NULL; 

	pNewOption = CreateOption(szTitle, funcAction);
	piOptionCount = &(pMenu->iOptionCount);

	/* If MENU is empty */
	if(piOptionCount == 0){
		pMenu->pOptions = (OPTION **)malloc(sizeof(OPTION *));
		memcpy(pMenu->pOptions, pNewOption, sizeof(OPTION*));

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

		/* Reassigning main ptr to new data */
		pMenu->pOptions = ppExtendedOptions;

		ppExtendedOptions = NULL;
	}

	/* Incrementing optioncount value */
	++(*piOptionCount);

	/* Handling dangling ptrs */
	piOptionCount = NULL;
	pNewOption = NULL;

	return OK;
}

/*
 *  Frees all memory associated with menu and removes dangling pointers.
 * */
int DestroyMenu(MENU **ppMenu){
	int i;
	OPTION *pCurrOption = NULL;

	if((*ppMenu)->iOptionCount > 0){
		for(i = 0; i < (*ppMenu)->iOptionCount; i++){
			pCurrOption = (*ppMenu)->pOptions[i]; 

			pCurrOption->funcAction = NULL;

			free(pCurrOption->pszTitle);
			pCurrOption->pszTitle = NULL;

			free(pCurrOption);

			/* Removing dangling pointers */
			pCurrOption = NULL;
		}
	}

	free((*ppMenu)->pOptions);
	free(*ppMenu);
	return OK;
}

/*
 *	Executes an options given action by indexing
 * */
int ExecuteAction(MENU pMenu, int iSelection){
	pMenu.pOptions[iSelection - 1]->funcAction();
	return OK;
}

/*
 *  Generates menu UI
 * */
int DisplayOptions(MENU pMenu){
	int i;
	printf("%s\n\n", "________________________________________________________");
	for(i = 0; i < pMenu.iOptionCount; i++){
		printf("%2d) %+30s\n\n", i + 1, pMenu.pOptions[i]->pszTitle);
	}
		printf("%2d) %+30s\n", 0, "EXIT");
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
		DestroyMenu(&pMenu);
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
}

