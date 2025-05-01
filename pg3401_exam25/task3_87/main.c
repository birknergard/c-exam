#include <stdio.h>
#include <stdlib.h>

#include "menu.h"
#include "ll_double.h"

int main(void){

	MENU *pMenu = NULL;

	pMenu = CreateMenu();
	
	DestroyMenu(&pMenu);


	return 0;
}	
