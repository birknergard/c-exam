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


MENU *CreateMenu(void *vpStruct);
int AddOption(MENU *pMenu, char *szTitle, void (*funcAction)());
int StartMenu(MENU *pMenu, char szProgramName[]);
int DestroyMenu(MENU *pMenu);


#endif /*ndef ___MENU_H___ */
