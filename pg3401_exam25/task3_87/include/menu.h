#ifndef ___MENU_H___
#define ___MENU_H___

#define TITLE_BUFFER 256
#define MAX_BUFFER 1024

#pragma pack(1)
typedef struct _OPTION {
	char *pszTitle;
	void (*funcAction)();
} OPTION;
#pragma pack(1)

/*
 * Keeping a list of pointers to pointer to options.
 * Using double pointers for more predictable memory structure, given that
 * the options hold function pointers which can vary in size.
 * Im hoping this helps with implementation.
 * */
typedef struct _MENU {
	OPTION **pOptions;
	int iOptionCount;

} MENU;


OPTION *CreateOption(char *szTitle, void (*funcAction)());
MENU *CreateMenu();
int AddOption(MENU *pMenu, char *szTitle, void (*funcAction)());

int StartMenu(MENU *pMenu, char szProgramName[]);
int DisplayOptions(MENU pMenu);
int SelectAction(MENU pMenu, char cSelection);
int DestroyMenu(MENU **pMenu);


#endif /*ndef ___MENU_H___ */
