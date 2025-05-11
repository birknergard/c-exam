/* TITLE: MENU
 * AUTHOR: 87
 *	DESCRIPTION 
 *
 * This API was created to display a menu with options in the terminal.
 * It designed to be flexible and could be used with any number of programs.
 *
 * Documentation in this header file is brief and only concerning high level
 * design decisions. For more fine grained commentary check the source file.
 * */
#ifndef ___MENU_H___
#define ___MENU_H___

#define TITLE_BUFFER 256
#define MAX_BUFFER 1024

#define MAX_STRING_SIZE 10
#define MAX_INPUT 1024

#pragma pack(1)
typedef struct _OPTION {
	char *pszTitle;
	void (*vfAction)(void *vpStruct);
} OPTION;
#pragma pack(1)

/*
 * Keeping a list of pointers to pointer to options.
 * Using double pointers for more predictable memory structure, given that
 * the options hold function pointers which can vary in size.
 * */
typedef struct _MENU {
	OPTION **pOptions;
	int iOptionCount;
	void *vpStruct; /* This struct can hold internal state depending on the program
							 In this submission it holds the FLIGHT_LIST structure for flight_list.h :) */
} MENU;

/*
 * Initializes and empty menu struct.
 * Takes a state object as void * as parameter 
 * */
MENU *CreateMenu(void *vpStruct);

/*
 * Adds an optionable action to the menu struct. Stored in a dynamic array.
 * */
int AddOption(MENU *pMenu, char *szTitle, void (*funcAction)());


/*
 * Starts the menu program with the menu struct.
 * */
int StartMenu(MENU *pMenu, char szProgramName[]);

/*
 * Deallocates / destroys struct
 * */
int DestroyMenu(MENU *pMenu);


/*
 * Takes input based on flags given.
 * */
int GetInput(int iArgC, char *szArgMessages[], char szTypeFlags[], ... );

/*
 * Converts a string to a POSITIVE integer. Used when selecting menu options.
 * */
int ParsePositiveInteger(char *psz);


#endif /*ndef ___MENU_H___ */
