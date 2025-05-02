#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "menu.h"
#include "flight_list.h"

int main(void){
    bdebug("Testing program!");
    
    MENU *pMenu = NULL;

    pMenu = CreateMenu();
	
    DestroyMenu(&pMenu);

    InternalFlightListTest();

    return 0;
}
