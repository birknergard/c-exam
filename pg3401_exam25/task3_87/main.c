#include "main.h"

#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "menu.h"
#include "flight_list.h"

int main(void){
    bdebug("Testing program!");
    
    MENU *pMenu = NULL;
    FLIGHT_LIST *pflFlights = NULL;

    pMenu = CreateMenu();
    //pflFlights = CreateFlightList();

    AddOption(pMenu, "Add flight", OptOne);

    AddOption(pMenu, "Add passenger to flight", OptTwo);

    AddOption(pMenu, "Display flights ", OptThree);

    AddOption(pMenu, "Find flight number by destination", OptFour);

    AddOption(pMenu, "Delete flight", OptFive);

    AddOption(pMenu, "Change passengers seat", OptSix);

    AddOption(pMenu, "Find which flights passenger is on", OptSeven);
    
    AddOption(pMenu, "View passengers multiple booked flights", OptEight);

    StartMenu(pMenu, "Task 3");
	
    DestroyMenu(pMenu);
    //DestroyFlightList(pflFlights);

    return 0;
}

/*
 * Option 1
 * */
void OptOne(){
    printf("Option 1!!");


}

/*
 * Option 2
 * */
void OptTwo(){

}

/*
 * Option 3
 * */
void OptThree(){

}

/*
 * Option 4
 * */
void OptFour(){

}

/*
 * Option 5
 * */
void OptFive(){

}


/*
 * Option 6
 * */
void OptSix(){

}

/*
 * Option 7
 * */
void OptSeven(){

}

/*
 * Option 8
 * */
void OptEight(){

}


