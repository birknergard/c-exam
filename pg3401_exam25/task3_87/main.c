#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "menu.h"
#include "ll_double.h"
#include "ll_single.h"

int main(void){

	bdebug("Testing program!");

	MENU *pMenu = NULL;

	pMenu = CreateMenu();
	
	DestroyMenu(&pMenu);

	return 0;
}	
